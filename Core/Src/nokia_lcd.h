/*
 * nokia_lcd.h
 *
 *  Created on: May 3, 2020
 *      Author: jerasmus
 */

#ifndef SRC_NOKIA_LCD_H_
#define SRC_NOKIA_LCD_H_

void nokia_lcd_init();
void nokia_lcd_run();

void lcd_str(char *str);
void lcd_set_pwr(float vin, float current);


#endif /* SRC_NOKIA_LCD_H_ */
