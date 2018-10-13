/*
 ============================================================================
 Name        : hev-rinput-receiver.h
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2018 everyone.
 Description : RInput receiver
 ============================================================================
 */

#ifndef __HEV_RINPUT_RECEIVER_H__
#define __HEV_RINPUT_RECEIVER_H__

typedef struct _HevRInputReceiver HevRInputReceiver;

HevRInputReceiver * hev_rinput_receiver_new (void);
void hev_rinput_receiver_destroy (HevRInputReceiver *self);

void hev_rinput_receiver_run (HevRInputReceiver *self);
void hev_rinput_receiver_quit (HevRInputReceiver *self);

#endif /* __HEV_RINPUT_RECEIVER_H__ */

