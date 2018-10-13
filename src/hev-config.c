/*
 ============================================================================
 Name        : hev-config.c
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2018 Heiher.
 Description : Config
 ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <iniparser.h>

#include "hev-config.h"

static char address[16];
static unsigned short port;

static int switch_keycode;

int
hev_config_init (const char *config_path)
{
	dictionary *ini_dict;

	ini_dict = iniparser_load (config_path);
	if (!ini_dict) {
		fprintf (stderr, "Load config from file %s failed!\n",
					config_path);
		return -1;
	}

	/* Main:Address */
	char *addr = iniparser_getstring (ini_dict, "Main:Address", NULL);
	if (!addr) {
		fprintf (stderr, "Get Main:Address from file %s failed!\n",
					config_path);
		iniparser_freedict (ini_dict);
		return -2;
	}
	strncpy (address, addr, 16 - 1);

	/* Main:Port */
	int value = iniparser_getint (ini_dict, "Main:Port", -1);
	if (-1 == value) {
		fprintf (stderr, "Get Main:Port from file %s failed!\n",
					config_path);
		iniparser_freedict (ini_dict);
		return -3;
	}
	port = value;

	/* Main:SwitchKeyCode */
	switch_keycode = iniparser_getint (ini_dict, "Main:SwitchKeyCode", 0);

	iniparser_freedict (ini_dict);

	return 0;
}

void
hev_config_fini (void)
{
}

const char *
hev_config_get_address (void)
{
	return address;
}

unsigned short
hev_config_get_port (void)
{
	return port;
}

int
hev_config_get_rinput_switch_keycode (void)
{
	return switch_keycode;
}

