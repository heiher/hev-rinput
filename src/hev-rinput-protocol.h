/*
 ============================================================================
 Name        : hev-rinput-protocol.h
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2018 everyone.
 Description : RInput protocol
 ============================================================================
 */

#ifndef __HEV_RINPUT_PROTOCOL_H__
#define __HEV_RINPUT_PROTOCOL_H__

typedef struct _HevRInputEvent HevRInputEvent;

struct _HevRInputEvent
{
    unsigned short type;
    unsigned short code;
    unsigned int value;
} __attribute__ ((packed));

#endif /* __HEV_RINPUT_PROTOCOL_H__ */
