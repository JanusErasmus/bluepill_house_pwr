/*
 * commands.c
 *
 *  Created on: May 1, 2020
 *      Author: jerasmus
 */
#include <stdio.h>

#include "Utils/cli.h"
#include "pwr_monitor.h"

void adcDebug(uint8_t argc, char **argv)
{
  float vin, current;
  pwr_monitor_get(&vin, &current);
  printf("Power Monitor: %4.4f V %4.4f A\n", vin, current);
}

const sTermEntry_t adcEntry =
{ "a", "Sample ADC", adcDebug };

const sTermEntry_t *cli_entries[] =
{
      &hEntry,
      &helpEntry,
      &rebootEntry,
      &bootEntry,
      &adcEntry,
	  0
};
