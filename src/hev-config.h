/*
 ============================================================================
 Name        : hev-config.h
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2018 Heiher.
 Description : Config
 ============================================================================
 */

#ifndef __HEV_CONFIG_H__
#define __HEV_CONFIG_H__

#define MAJOR_VERSION		(0)
#define MINOR_VERSION		(0)
#define MICRO_VERSION		(1)

int hev_config_init (const char *config_path);
void hev_config_fini (void);

#endif /* __HEV_CONFIG_H__ */

