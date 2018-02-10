/*
 ============================================================================
 Name        : hev-config.c
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2018 Heiher.
 Description : Config
 ============================================================================
 */

#include <stdio.h>
#include <iniparser.h>

#include "hev-config.h"

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

	iniparser_freedict (ini_dict);

	return 0;
}

void
hev_config_fini (void)
{
}

