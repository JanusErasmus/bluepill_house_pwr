/*
 * wrap_cpp.cpp
 *
 *  Created on: 09 May 2020
 *      Author: jerasmus
 */
#include <stdio.h>
#include "usart.h"
#include "sonoff_pipe.h"
#include "pwr_monitor.h"
#include "nokia_lcd.h"

#define UPDATE_RATE 4000 //Match the sample rate of the power monitor mean measurement
#define REPORT_RATE 600000 // 10 min
//limit Sonoff report rate to 30 s
#define MIN_REPORT_RATE 30000
static float prev_vin = 0;
static float prev_current = 0;

SonoffPipe pipe;
static uint32_t tick = 0;
static uint32_t report_tick = 0;
static uint32_t last_report = 0;

void handleMessage(const char* line)
{
  printf("MQTT: %s\n", line);

}

int sonoff_report()
{
  float vin, current;
  pwr_monitor_get(&vin, &current);

  char json[128];
  sprintf(json, "{\"uptime\":%d,"
      "\"temp\": 0.0,"
      "\"voltages\":[%0.3f,%0.3f,0,0]"
      "}",
      (int)HAL_GetTick(),
      vin,
      current
  );

  // printf("Sonoff publish: %s\n", msg);
  if(pipe.publish(json))
  {
    prev_vin = vin;
    prev_current = current;
    report_tick = HAL_GetTick();
    last_report = HAL_GetTick() + REPORT_RATE;

    return 1;
  }

  return 0;
}

extern "C" {

  void esp_handle_byte(uint8_t byte)
  {
      pipe.handleByte(byte);
  }

  int esp_transmit(uint8_t *buf, int len)
  {
    //the sonoff is not fast enough to receive the whole message at once, delay each byte
    for (int k = 0; k < len; ++k)
    {
      if(HAL_UART_Transmit(&huart2, &buf[k], 1, 300)  != HAL_OK)
        return -1;

      HAL_Delay(100);
    }

    return len;
  }

 void cpp_init()
 {
   pipe.setTransmitCB(esp_transmit);
   pipe.setReceivedCB(handleMessage);
 }

 void cpp_run()
 {
    pipe.run();

   if(tick < HAL_GetTick())
   {
     tick = HAL_GetTick() + UPDATE_RATE;

     float vin, current;
     if(pwr_monitor_get(&vin, &current))
     {
       lcd_set_pwr(vin, current);

       int changed = 0;
       if((prev_vin + 2 < vin) || (vin < prev_vin - 2))
       {
         changed = 1;
       }
       if((prev_current + 0.5 < current) || (current < prev_current - 0.5))
       {
         changed = 1;
       }

       if(changed)
       {
         //Prevent excessive reporting, by limiting to the minimum report rate
         if(report_tick + MIN_REPORT_RATE < HAL_GetTick())
         {
           sonoff_report();
         }
       }

       //report on regular intervals
       if(last_report < HAL_GetTick())
       {
         sonoff_report();
       }
     }
   }
 }

 int esp_idle()
 {
   return pipe.isIdle();
 }

 void sonoffDebug(uint8_t argc, char **argv)
 {
   printf("Reporting via Sonoff: %s\n", sonoff_report()?"OK":"KO");
 }

}
