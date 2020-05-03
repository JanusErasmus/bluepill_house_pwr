/*
 * pwr_monitor.h
 *
 *  Created on: 03 May 2020
 *      Author: jerasmus
 */

#ifndef SRC_PWR_MONITOR_H_
#define SRC_PWR_MONITOR_H_

void pwr_monitor_init();
void pwr_monitor_run();
void pwr_monitor_get(float *vin, float *current);

#endif /* SRC_PWR_MONITOR_H_ */
