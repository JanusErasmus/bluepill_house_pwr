/*
 * commands.c
 *
 *  Created on: May 1, 2020
 *      Author: jerasmus
 */
#include "Utils/cli.h"

extern void adc_sample();

void adcDebug(uint8_t argc, char **argv)
{
  printf("Sampling Vin...\n");
  adc_sample();
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
