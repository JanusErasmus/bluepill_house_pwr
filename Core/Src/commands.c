/*
 * commands.c
 *
 *  Created on: May 1, 2020
 *      Author: jerasmus
 */
#include "Utils/cli.h"

const sTermEntry_t *cli_entries[] =
{
      &hEntry,
      &helpEntry,
      &rebootEntry,
      &bootEntry,
	  0
};
