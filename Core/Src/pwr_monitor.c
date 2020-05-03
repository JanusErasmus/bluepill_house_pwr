/*
 * pwr_monitor.c
 *
 *  Created on: 03 May 2020
 *      Author: jerasmus
 */
#include <stdio.h>
#include <math.h>

#include "adc.h"
#include "tim.h"
#include "nokia_lcd.h"

#define PWR_SAMPLE_RATE 2000

static int adc_start = 0;
static volatile int adc_sampled = 0;
static int adc_index = 0;
static uint16_t adc_samples[160];
static float raw_vin[80];
static float raw_current[80];
static int measure_index = 0;
static float vin_measure[8];
static float current_measure[8];
static float _vin = 0;
static float _current = 0;
static uint32_t tick = 0;

void pwr_monitor_get(float *vin, float *current)
{
  *vin = _vin;
  *current = _current;
}

void pwr_monitor_init()
{
  tick = HAL_GetTick() + PWR_SAMPLE_RATE;
}

void pwr_monitor_run()
{
  if(tick < HAL_GetTick())
  {
    tick = HAL_GetTick() + PWR_SAMPLE_RATE;

    lcd_set_pwr(_vin, _current);

    if(adc_start <= 0)
    {
      adc_start = 8;
      HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_1);
    }
  }

  if(adc_sampled)
  {
    float max_vin = -99999;
    float min_vin = 99999;
    float max_current = -99999;
    float min_current = 99999;
    int index = 0;
    for (int k = 0; k < 160; ++k)
    {
      raw_vin[index] = adc_samples[k] * 0.000805860805860806;
      if(raw_vin[index] > max_vin)
        max_vin = raw_vin[index];
      if(raw_vin[index] < min_vin)
        min_vin = raw_vin[index];

      k++;
      raw_current[index] = adc_samples[k] * 0.000805860805860806;
      if(raw_current[index] > max_current)
        max_current = raw_current[index];
      if(raw_current[index] < min_current)
        min_current = raw_current[index];

      index++;
    }

    float offset_vin = min_vin + ((max_vin - min_vin) / 2.0);
    float offset_current = min_current + ((max_current - min_current) / 2.0);
    for (int k = 0; k < 80; ++k)
    {
      raw_vin[k] -= offset_vin;
      raw_current[k] -= offset_current;
      // printf("%0.4f,%0.4f\n", raw_vin[k], raw_current[k]);
    }


    float mean_vin = 0;
    float mean_current = 0;
    for (int k = 0; k < 80; ++k)
    {
      mean_vin += (raw_vin[k] * raw_vin[k]);
      mean_current += (raw_current[k] * raw_current[k]);
    }

    mean_vin /= 80.0;
    mean_current /= 80.0;

    float vin = sqrtf(mean_vin) * 368.0;
    float current = sqrtf(mean_current) * 25.8;
    // printf("V: %5.4f A: %5.4f\n", vin, current);

    vin_measure[measure_index] = vin;
    current_measure[measure_index] = current;
    measure_index++;

    adc_sampled = 0;
    adc_start--;
  }

  if(measure_index >= 8)
  {
    float vin = 0;
    float current = 0;
    measure_index = 0;
    for (int k = 0; k < 8; ++k)
    {
      vin += vin_measure[k];
      current += current_measure[k];
    }
    vin /= 8;
    current /= 8;
    if(current < 0.2)
      current = 0;

    _vin = vin;
    _current = current;

    // printf(" V: %4.4f A: %4.4f\n", _vin, _current);
  }
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  // HAL_GPIO_TogglePin(SAMPLE_PULSE_GPIO_Port, SAMPLE_PULSE_Pin);
  if(adc_index >= 160)
  {
    if(adc_start <= 0)
      HAL_TIM_OC_Stop_IT(&htim1, TIM_CHANNEL_1);

    adc_sampled = 1;
    adc_index = 0;
  }

  if(adc_sampled == 0)
  {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    adc_samples[adc_index] = HAL_ADC_GetValue(&hadc1);
    adc_index++;
  }
}
