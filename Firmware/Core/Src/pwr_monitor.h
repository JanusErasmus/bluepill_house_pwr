/*
 * pwr_monitor.h
 *
 *  Created on: 03 May 2020
 *      Author: jerasmus
 */

#ifndef SRC_PWR_MONITOR_H_
#define SRC_PWR_MONITOR_H_
#ifdef __cplusplus
 extern "C" {
#endif

void pwr_monitor_init();
void pwr_monitor_run();
int pwr_monitor_get(float *vin, float *current);

int pwr_monitor_busy();

#ifdef __cplusplus
 }
#endif
#endif /* SRC_PWR_MONITOR_H_ */
