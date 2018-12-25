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

#define MAJOR_VERSION (1)
#define MINOR_VERSION (5)
#define MICRO_VERSION (3)

int hev_config_init (const char *config_path);
void hev_config_fini (void);

const char *hev_config_get_address (void);
unsigned short hev_config_get_port (void);

int hev_config_get_rinput_switch_keycode (void);

#endif /* __HEV_CONFIG_H__ */
