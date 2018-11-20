/*
 ============================================================================
 Name        : hev-rinput-receiver.c
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2018 everyone.
 Description : RInput receiver
 ============================================================================
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/input.h>
#include <linux/uinput.h>

#include <hev-task.h>
#include <hev-task-io.h>
#include <hev-task-io-socket.h>
#include <hev-memory-allocator.h>

#include "hev-rinput-receiver.h"
#include "hev-rinput-protocol.h"
#include "hev-config.h"

struct _HevRInputReceiver
{
    HevTask *task;

    int net_fd;
    int uin_fd;

    int quit;
};

static void hev_rinput_task_entry (void *data);

static int hev_rinput_get_socket (void);
static int hev_rinput_get_uinput (void);

HevRInputReceiver *
hev_rinput_receiver_new (void)
{
    HevRInputReceiver *self;

    self = hev_malloc0 (sizeof (HevRInputReceiver));
    if (!self) {
        fprintf (stderr, "Alloc HevRInputReceiver failed!\n");
        return NULL;
    }

    self->net_fd = hev_rinput_get_socket ();
    if (self->net_fd < 0) {
        hev_free (self);
        return NULL;
    }

    self->uin_fd = hev_rinput_get_uinput ();
    if (self->uin_fd < 0) {
        hev_free (self);
        return NULL;
    }

    self->task = hev_task_new (-1);
    if (!self->task) {
        fprintf (stderr, "Create task failed!\n");
        close (self->uin_fd);
        close (self->net_fd);
        hev_free (self);
        return NULL;
    }

    hev_task_ref (self->task);

    return self;
}

static int
hev_rinput_get_socket (void)
{
    int fd;
    struct sockaddr_in addr;
    int ret, reuseaddr = 1;

    fd = hev_task_io_socket_socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd == -1) {
        fprintf (stderr, "Create socket failed!\n");
        return -1;
    }

    ret = setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,
                      sizeof (reuseaddr));
    if (ret == -1) {
        fprintf (stderr, "Set reuse address failed!\n");
        close (fd);
        return -1;
    }

    memset (&addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr (hev_config_get_address ());
    addr.sin_port = htons (hev_config_get_port ());
    ret = bind (fd, (struct sockaddr *)&addr, (socklen_t)sizeof (addr));
    if (ret == -1) {
        fprintf (stderr, "Bind address failed!\n");
        close (fd);
        return -1;
    }

    return fd;
}

static int
hev_rinput_get_uinput (void)
{
    int fd, i;
    struct
    {
        int cmd;
        int val;
    } caps[] = {
        { UI_SET_EVBIT, EV_KEY },     { UI_SET_EVBIT, EV_REP },
        { UI_SET_KEYBIT, BTN_LEFT },  { UI_SET_KEYBIT, BTN_MIDDLE },
        { UI_SET_KEYBIT, BTN_RIGHT }, { UI_SET_KEYBIT, BTN_FORWARD },
        { UI_SET_KEYBIT, BTN_BACK },  { UI_SET_KEYBIT, BTN_SIDE },
        { UI_SET_KEYBIT, BTN_EXTRA }, { UI_SET_EVBIT, EV_REL },
        { UI_SET_RELBIT, REL_X },     { UI_SET_RELBIT, REL_Y },
        { UI_SET_RELBIT, REL_WHEEL }, { 0, 0 },
    };
    struct uinput_user_dev uin_dev;

    fd = hev_task_io_open ("/dev/uinput", O_WRONLY);
    if (fd == -1) {
        fprintf (stderr, "Open uinput device failed!\n");
        return -1;
    }

    for (i = 0; caps[i].cmd; i++) {
        if (-1 == ioctl (fd, caps[i].cmd, caps[i].val)) {
            fprintf (stderr, "Set uinput capabilities failed!\n");
            close (fd);
            return -1;
        }
    }
    for (i = KEY_RESERVED; i <= KEY_RFKILL; i++) {
        if (-1 == ioctl (fd, UI_SET_KEYBIT, i)) {
            fprintf (stderr, "Set uinput capabilities failed!\n");
            close (fd);
            return -1;
        }
    }

    memset (&uin_dev, 0, sizeof (uin_dev));
    strncpy (uin_dev.name, "Remote Input Device", UINPUT_MAX_NAME_SIZE);
    uin_dev.id.bustype = BUS_USB;
    uin_dev.id.vendor = 0x1;
    uin_dev.id.product = 0x1;
    uin_dev.id.version = 1;
    if (sizeof (uin_dev) != write (fd, &uin_dev, sizeof (uin_dev))) {
        fprintf (stderr, "Set uinput properties failed!\n");
        close (fd);
        return -1;
    }

    if (-1 == ioctl (fd, UI_DEV_CREATE)) {
        fprintf (stderr, "Create uinput device failed!\n");
        close (fd);
        return -1;
    }

    return fd;
}

void
hev_rinput_receiver_destroy (HevRInputReceiver *self)
{
    hev_task_unref (self->task);

    ioctl (self->uin_fd, UI_DEV_DESTROY);
    close (self->uin_fd);
    close (self->net_fd);
    hev_free (self);
}

void
hev_rinput_receiver_run (HevRInputReceiver *self)
{
    hev_task_run (self->task, hev_rinput_task_entry, self);
}

void
hev_rinput_receiver_quit (HevRInputReceiver *self)
{
    self->quit = 1;
    hev_task_wakeup (self->task);
}

static int
task_io_yielder (HevTaskYieldType type, void *data)
{
    HevRInputReceiver *self = data;

    if (self->quit)
        return -1;

    hev_task_yield (type);

    return (self->quit) ? -1 : 0;
}

static void
hev_rinput_task_entry (void *data)
{
    HevRInputReceiver *self = data;
    HevTask *task = hev_task_self ();

    hev_task_add_fd (task, self->net_fd, EPOLLIN);
    hev_task_add_fd (task, self->uin_fd, EPOLLOUT);

    for (;;) {
        ssize_t len;
        HevRInputEvent revent;
        struct input_event event;

        len = hev_task_io_socket_recvfrom (self->net_fd, &revent,
                                           sizeof (revent), 0, NULL, NULL,
                                           task_io_yielder, self);
        if (sizeof (revent) != len) {
            if (-2 == len)
                break;
            fprintf (stderr, "Got invalid uinput event!\n");
            continue;
        }

        if (EV_REL == revent.type) {
            if (-1 == gettimeofday (&event.time, NULL))
                continue;
        }

        event.type = revent.type;
        event.code = revent.code;
        event.value = revent.value;
        len = hev_task_io_write (self->uin_fd, &event, sizeof (event),
                                 task_io_yielder, self);
        if (-2 == len)
            break;
    }
}
