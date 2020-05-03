/*
 * nokia_lcd.c
 *
 *  Created on: May 3, 2020
 *      Author: jerasmus
 */
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "spi.h"

#define DISPLAY_REFRESH 100
extern  uint8_t FontLookup[][5];

typedef enum
{
    FONT_1X = 1,
    FONT_2X = 2
} LcdFontSize;

void lcd_str(char *str);

static int tick = 0;
static uint8_t  LcdCache [ 512 ];
static int LcdCacheIdx = 0;
static int UpdateLcd = 0;
static int LoWaterMark = 0;
static int HiWaterMark = 503;

static void lcd_cmd(uint8_t cmd)
{
  HAL_GPIO_WritePin(SPI1_DC_GPIO_Port, SPI1_DC_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SPI1_CE_GPIO_Port, SPI1_CE_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, &cmd, 1, 1000);
  HAL_GPIO_WritePin(SPI1_DC_GPIO_Port, SPI1_DC_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SPI1_CE_GPIO_Port, SPI1_CE_Pin, GPIO_PIN_SET);
}

static void lcd_data(uint8_t *data, int len)
{
  HAL_GPIO_WritePin(SPI1_CE_GPIO_Port, SPI1_CE_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, data, len, 1000);
  HAL_GPIO_WritePin(SPI1_CE_GPIO_Port, SPI1_CE_Pin, GPIO_PIN_SET);
}

void lcd_update()
{
    if ( LoWaterMark < 0 )
        LoWaterMark = 0;
    else if ( LoWaterMark >= 504 )
      LoWaterMark = 503;
    if ( HiWaterMark < 0 )
        HiWaterMark = 0;
    else if ( HiWaterMark >= 504 )
        HiWaterMark = 503;

    //  Set base address according to LoWaterMark.
    lcd_cmd( 0x80 | (LoWaterMark % 84) );
    lcd_cmd( 0x40 | (LoWaterMark / 84) );

    //  Serialize the video buffer.
    int len = (HiWaterMark - LoWaterMark) + 1;
    lcd_data( &LcdCache[LoWaterMark], len);

    //  Reset watermark pointers.
    LoWaterMark = 503;
    HiWaterMark = 0;
}

void nokia_lcd_init()
{
  tick = HAL_GetTick() + DISPLAY_REFRESH;

  HAL_GPIO_WritePin(SPI1_RESET_GPIO_Port, SPI1_RESET_Pin, GPIO_PIN_RESET);
  HAL_Delay(500);
  HAL_GPIO_WritePin(SPI1_RESET_GPIO_Port, SPI1_RESET_Pin, GPIO_PIN_SET);
  HAL_Delay(100);

  lcd_cmd( 0x21 );  // LCD Extended Commands.
  lcd_cmd( 0xC8 );  // Set LCD Vop (Contrast).
  lcd_cmd( 0x06 );  // Set Temp coefficent.
  lcd_cmd( 0x13 );  // LCD bias mode 1:48.
  lcd_cmd( 0x20 );  // LCD Standard Commands, Horizontal addressing mode.
  lcd_cmd( 0x0C );  // LCD in normal mode.

  memset(LcdCache, 0x00, 504);

//  LcdChr(FONT_1X, 'A');
//  LcdChr(FONT_2X, 'J');
//  LcdChr(FONT_2X, 'a');
//  LcdChr(FONT_2X, 'n');
//  LcdChr(FONT_2X, 'u');
//  LcdChr(FONT_2X, 's');
//  LcdChr(FONT_1X, ' ');
//  LcdChr(FONT_1X, 'E');
//  LcdChr(FONT_1X, 'r');
//  LcdChr(FONT_1X, 'a');
//  LcdChr(FONT_2X, 'K');
}


void lcd_chr ( LcdFontSize size, uint8_t ch )
{
  uint8_t i, c;
  uint8_t b1, b2;
  int  tmpIdx;

  if ( LcdCacheIdx < LoWaterMark )
  {
    //  Update low marker.
    LoWaterMark = LcdCacheIdx;
  }

  if (ch < 0x20)
  {
    ch = 148;
  }

  if (ch > 151) // Convert ISO8859-1 to ascii
  {

    if (ch == 0xc0) ch = 133;
    else if (ch == 0xc2) ch = 131;
    else if (ch == 0xc7) ch = 128;
    else if (ch == 0xc9) ch = 144;
    else if (ch == 0xca) ch = 136;
    else if (ch == 0xce) ch = 140;
    else if (ch == 0xe0) ch = 133;
    else if (ch == 0xe2) ch = 131;
    else if (ch == 0xe7) ch = 135;
    else if (ch == 0xe8) ch = 138;
    else if (ch == 0xe9) ch = 130;
    else if (ch == 0xea) ch = 136;
    else if (ch == 0xeb) ch = 137;
    else if (ch == 0xee) ch = 140;
    else if (ch == 0xef) ch = 139;
    else if (ch == 0xf4) ch = 147;
    else if (ch == 0xf9) ch = 151;
    else if (ch == 0xfb) ch = 150;
    else ch = 148;
  }

  if ( size == FONT_1X )
  {
    for ( i = 0; i < 5; i++ )
    {
      LcdCache[LcdCacheIdx++] = FontLookup[ch - 32][i] << 1;
    }
  }
  else if ( size == FONT_2X )
  {
    tmpIdx = LcdCacheIdx - 84;

    if ( tmpIdx < LoWaterMark )
    {
      LoWaterMark = tmpIdx;
    }

    if ( tmpIdx < 0 )
      return;


    for ( i = 0; i < 5; i++ )
    {
      c = FontLookup[ch - 32][i] << 1;

      b1 =  (c & 0x01) * 3;
      b1 |= (c & 0x02) * 6;
      b1 |= (c & 0x04) * 12;
      b1 |= (c & 0x08) * 24;

      c >>= 4;
      b2 =  (c & 0x01) * 3;
      b2 |= (c & 0x02) * 6;
      b2 |= (c & 0x04) * 12;
      b2 |= (c & 0x08) * 24;

      LcdCache[tmpIdx++] = b1;
      LcdCache[tmpIdx++] = b1;
      LcdCache[tmpIdx + 82] = b2;
      LcdCache[tmpIdx + 83] = b2;
    }

    //  Update x cursor position.
    LcdCacheIdx += 11;
  }

  if ( LcdCacheIdx > HiWaterMark )
  {
    //  Update high marker.
    HiWaterMark = LcdCacheIdx;
  }

  //  Horizontal gap between characters.
  LcdCache[LcdCacheIdx++] = 0x00;
  UpdateLcd = 1;
}

void lcd_str(char *str)
{
  while(*str)
  {
    lcd_chr(FONT_2X, *str++);
  }
}

void lcd_str_sml(char *str)
{
  while(*str)
  {
    lcd_chr(FONT_1X, *str++);
  }
}

void lcd_pixel ( uint8_t x, uint8_t y, uint8_t mode )
{
    int  index;
    uint8_t offset;
    uint8_t data;

    if ( x > 84 )
      return;

    if ( y > 48 )
      return;

    index = ((y / 8) * 84) + x;
    offset  = y - ((y / 8) * 8);
    data = LcdCache[index];

    if ( mode == 0 )
    {
        data &= (~(0x01 << offset));
    }
    else if ( mode == 1 )
    {
        data |= (0x01 << offset);
    }

    LcdCache[index] = data;

    if ( index < LoWaterMark )
    {
        //  Update low marker.
        LoWaterMark = index;
    }

    if ( index > HiWaterMark )
    {
        //  Update high marker.
        HiWaterMark = index;
    }

    UpdateLcd = 1;
}

void nokia_lcd_run()
{
  if(tick < HAL_GetTick())
  {
    tick = HAL_GetTick() + DISPLAY_REFRESH;

    if(UpdateLcd)
    {
      UpdateLcd = 0;
      lcd_update();
    }

//    lcd_pixel(x, y, 1);
//    y++;
//    if(x >= 84)
//    {
//      x = 0;
//    }
//
//    if(y >= 48)
//    {
//      x++;
//      y = 0;
//    }
  }
}


static float _vin = 0;
static float _current = 0;
void lcd_set_pwr(float vin, float current)
{
  int changed = 0;
  if((_vin + 1 < vin) || (vin < _vin - 1))
  {
    changed = 1;
    _vin = vin;
  }
  if((_current + 0.1 < current) || (current < _current - 0.1))
  {
    changed = 1;
    _current = current;
  }

  if(!changed)
    return;

  char temp[64];
  sprintf(temp, "%03.1f V", _vin);
  LcdCacheIdx = 84;
  lcd_str(temp);
  sprintf(temp, "%1.3f A", _current);
  LcdCacheIdx = 336;
  lcd_str(temp);

  sprintf(temp, "%6.3f kW", (_current * _vin) / 1000.0);
  LcdCacheIdx = 420;
  lcd_str_sml(temp);
}
