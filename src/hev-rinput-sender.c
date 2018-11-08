/*
 ============================================================================
 Name        : hev-rinput-sender.c
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2018 everyone.
 Description : RInput sender
 ============================================================================
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/input.h>

#include <hev-task.h>
#include <hev-task-io.h>
#include <hev-task-io-socket.h>
#include <hev-memory-allocator.h>

#include "hev-rinput-sender.h"
#include "hev-rinput-protocol.h"
#include "hev-config.h"

#define MAX_INPUTS (1024)

typedef struct _HevRInputContext HevRInputContext;

struct _HevRInputContext
{
    int fd;
    int yield;
    HevTask *task;
    HevRInputSender *self;
};

struct _HevRInputSender
{
    int net_fd;
    unsigned int in_count;

    HevTask *task_sock;
    HevRInputContext contexts[MAX_INPUTS];

    struct sockaddr_in addr;
    int switch_keycode;

    int grab;
    int quit;
};

static void hev_rinput_task_entry (void *data);
static void hev_rinput_task_sock_entry (void *data);

static int hev_rinput_get_socket (void);
static int hev_rinput_get_inputs (HevRInputSender *self);

HevRInputSender *
hev_rinput_sender_new (void)
{
    HevRInputSender *self;

    self = hev_malloc0 (sizeof (HevRInputSender));
    if (!self) {
        fprintf (stderr, "Alloc HevRInputSender failed!\n");
        return NULL;
    }

    self->task_sock = hev_task_new (-1);
    if (!self->task_sock) {
        fprintf (stderr, "Create task sock failed!\n");
        hev_free (self);
        return NULL;
    }
    hev_task_ref (self->task_sock);

    self->net_fd = hev_rinput_get_socket ();
    if (self->net_fd < 0) {
        hev_task_unref (self->task_sock);
        hev_free (self);
        return NULL;
    }

    if (0 > hev_rinput_get_inputs (self)) {
        fprintf (stderr, "Get inputs failed!\n");
        close (self->net_fd);
        hev_task_unref (self->task_sock);
        hev_free (self);
        return NULL;
    }

    self->switch_keycode = hev_config_get_rinput_switch_keycode ();

    memset (&self->addr, 0, sizeof (self->addr));
    self->addr.sin_family = AF_INET;
    self->addr.sin_addr.s_addr = inet_addr (hev_config_get_address ());
    self->addr.sin_port = htons (hev_config_get_port ());

    return self;
}

static int
hev_rinput_get_socket (void)
{
    int fd;
    int ret, nonblock = 1;

    fd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd == -1) {
        fprintf (stderr, "Create socket failed!\n");
        return -1;
    }

    ret = ioctl (fd, FIONBIO, (char *)&nonblock);
    if (ret == -1) {
        fprintf (stderr, "Set non-blocking failed!\n");
        close (fd);
        return -1;
    }

    return fd;
}

static int
hev_rinput_get_inputs (HevRInputSender *self)
{
    DIR *dir;
    struct dirent *dire;

    dir = opendir ("/dev/input");
    if (!dir) {
        fprintf (stderr, "Open /dev/input failed!\n");
        return -1;
    }

    while ((dire = readdir (dir))) {
        char buf[512];
        int fd;
        HevTask *task;

        if (DT_CHR != dire->d_type)
            continue;
        if (0 != strncmp (dire->d_name, "event", 5))
            continue;

        snprintf (buf, 512, "/dev/input/%s", dire->d_name);

        fd = open (buf, O_RDONLY | O_NONBLOCK);
        if (-1 == fd)
            continue;

        task = hev_task_new (-1);
        if (!task) {
            close (fd);
            continue;
        }

        hev_task_ref (task);

        self->contexts[self->in_count].fd = fd;
        self->contexts[self->in_count].task = task;
        self->contexts[self->in_count].self = self;

        self->in_count++;
    }

    closedir (dir);

    return 0;
}

void
hev_rinput_sender_destroy (HevRInputSender *self)
{
    int i;

    for (i = 0; i < self->in_count; i++) {
        hev_task_unref (self->contexts[i].task);
        close (self->contexts[i].fd);
    }
    close (self->net_fd);
    hev_task_unref (self->task_sock);
    hev_free (self);
}

void
hev_rinput_sender_run (HevRInputSender *self)
{
    int i;

    for (i = 0; i < self->in_count; i++) {
        HevTask *task = self->contexts[i].task;
        hev_task_run (task, hev_rinput_task_entry, &self->contexts[i]);
    }
    hev_task_run (self->task_sock, hev_rinput_task_sock_entry, self);
}

void
hev_rinput_sender_quit (HevRInputSender *self)
{
    int i;

    self->quit = 1;

    for (i = 0; i < self->in_count; i++)
        hev_task_wakeup (self->contexts[i].task);
    hev_task_wakeup (self->task_sock);
}

static int
task_io_yielder (HevTaskYieldType type, void *data)
{
    HevRInputSender *self = data;

    if (self->quit)
        return -1;

    hev_task_yield (type);

    return (self->quit) ? -1 : 0;
}

static int
task_sock_io_yielder (HevTaskYieldType type, void *data)
{
    HevRInputContext *context = data;
    HevRInputSender *self = context->self;

    if (self->quit)
        return -1;

    context->yield = 1;
    hev_task_yield (type);
    context->yield = 0;

    return (self->quit) ? -1 : 0;
}

static void
hev_rinput_toggle_grab (HevRInputSender *self)
{
    int i;

    self->grab = self->grab ? 0 : 1;

    for (i = 0; i < self->in_count; i++)
        ioctl (self->contexts[i].fd, EVIOCGRAB, self->grab);
}

static void
hev_rinput_task_entry (void *data)
{
    HevRInputContext *context = data;
    HevRInputSender *self = context->self;
    HevTask *task = hev_task_self ();

    hev_task_add_fd (task, context->fd, EPOLLIN);

    for (;;) {
        ssize_t len;
        HevRInputEvent revent;
        struct input_event event;

        len = hev_task_io_read (context->fd, &event, sizeof (event),
                                task_io_yielder, self);
        if (sizeof (event) != len) {
            if (-2 != len)
                fprintf (stderr, "Got invalid input event!\n");
            break;
        }

        if ((EV_KEY == event.type) && (self->switch_keycode == event.code) &&
            (1 == event.value)) {
            hev_rinput_toggle_grab (self);
            continue;
        }

        if (!self->grab)
            continue;

        revent.type = event.type;
        revent.code = event.code;
        revent.value = event.value;
        len = hev_task_io_socket_sendto (self->net_fd, &revent, sizeof (revent),
                                         0,
                                         (const struct sockaddr *)&self->addr,
                                         sizeof (self->addr),
                                         task_sock_io_yielder, context);
        if (-2 == len)
            break;
    }
}

static void
hev_rinput_task_sock_entry (void *data)
{
    HevRInputSender *self = data;
    HevTask *task = hev_task_self ();

    hev_task_add_fd (task, self->net_fd, EPOLLOUT);

    for (;;) {
        int i;

        hev_task_yield (HEV_TASK_WAITIO);
        if (self->quit)
            break;

        for (i = 0; i < self->in_count; i++) {
            if (self->contexts[i].yield)
                hev_task_wakeup (self->contexts[i].task);
        }
    }
}
