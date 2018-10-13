/*
 ============================================================================
 Name        : hev-rinput-sender.h
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2018 everyone.
 Description : RInput sender
 ============================================================================
 */

#ifndef __HEV_RINPUT_SENDER_H__
#define __HEV_RINPUT_SENDER_H__

typedef struct _HevRInputSender HevRInputSender;

HevRInputSender * hev_rinput_sender_new (void);
void hev_rinput_sender_destroy (HevRInputSender *self);

void hev_rinput_sender_run (HevRInputSender *self);
void hev_rinput_sender_quit (HevRInputSender *self);

#endif /* __HEV_RINPUT_SENDER_H__ */

