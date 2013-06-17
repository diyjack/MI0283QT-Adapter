#include <stdint.h>
#include "cmsis/LPC11xx.h"
#include "settings.h"
#include "main.h"
#include "lcd.h"
#include "lcd_mi0283qt9.h"

#ifdef LCD_MI0283QT9


#define LCD_PORT        2
#define DATA_PINS       0xFF
#define DATA_WR(x)      GPIO_SETPORT(LCD_PORT, DATA_PINS, x)
#define RST_PIN         8
#define CS_PIN          11
#define RS_PIN          10
#define WR_PIN          9
#define RD_PIN          5 //GPIO3[5]

#define INIT_PINS()     { GPIO_PORT(LCD_PORT)->DIR  |= (1<<RST_PIN) | (1<<CS_PIN) | (1<<RS_PIN) | (1<<WR_PIN) | DATA_PINS; \
                          GPIO_PORT(LCD_PORT)->DATA |= (1<<RST_PIN) | (1<<CS_PIN) | (1<<RS_PIN) | (1<<WR_PIN) | DATA_PINS; \
                          GPIO_PORT(3)->DIR  |= (1<<RD_PIN); \
                          GPIO_PORT(3)->DATA |= (1<<RD_PIN); }


void lcd_drawstop(void)
{
  //lcd_disable();

  return;
}


void lcd_draw(uint32_t color)
{
  return lcd_wrdata16(color);
}


void lcd_drawstart(void)
{
  //lcd_enable();

  lcd_wrcmd8(LCD_CMD_WRITE);

  return;
}


void lcd_setarea(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1)
{
  //lcd_enable();

  lcd_wrcmd8(LCD_CMD_COLUMN);
  lcd_wrdata16(y0);
  lcd_wrdata16(y1);

  lcd_wrcmd8(LCD_CMD_PAGE);
  lcd_wrdata16(x0);
  lcd_wrdata16(x1);

  //lcd_disable();

  return;
}


uint32_t lcd_setbias(uint32_t o)
{
  uint32_t param, w, h;

  #define MEM_Y   (7) //MY row address order
  #define MEM_X   (6) //MX column address order 
  #define MEM_V   (5) //MV row / column exchange 
  #define MEM_L   (4) //ML vertical refresh order
  #define MEM_H   (2) //MH horizontal refresh order
  #define MEM_BGR (3) //RGB-BGR Order 

  switch(o)
  {
    default:
    case 0:
    //case 36:
    //case 360:
      o     = 0;
      w     = LCD_WIDTH;
      h     = LCD_HEIGHT;
      param = (1<<MEM_BGR) | (1<<MEM_X) | (1<<MEM_Y);
      break;

    case 9:
    case 90:
      o     = 90;
      w     = LCD_HEIGHT;
      h     = LCD_WIDTH;
      param = (1<<MEM_BGR) | (1<<MEM_X) | (1<<MEM_V);
      break;

    case 18:
    case 180:
      o     = 180;
      w     = LCD_WIDTH;
      h     = LCD_HEIGHT;
      param = (1<<MEM_BGR) | (1<<MEM_L);
      break;

    case 27:
    case 270:
      o     = 270;
      w     = LCD_HEIGHT;
      h     = LCD_WIDTH;
      param = (1<<MEM_BGR) | (1<<MEM_Y) | (1<<MEM_V);
      break;
  }

  lcd_enable();

  lcd_wrcmd8(LCD_CMD_MEMACCESS_CTRL);
  lcd_wrdata8(param);

  lcd_setarea(0, 0, w-1, h-1);

  lcd_disable();

  return (w<<16) | (h<<0);
}


void __attribute__((optimize("Os"))) lcd_invert(uint32_t on)
{
  lcd_enable();

  if(on == 0)
  {
     lcd_wrcmd8(LCD_CMD_INV_OFF);
  }
  else
  {
     lcd_wrcmd8(LCD_CMD_INV_ON);
  }

  lcd_disable();

  return;
}


void __attribute__((optimize("Os"))) lcd_power(uint32_t on)
{
  lcd_enable();

  if(on == 0)
  {
    lcd_wrcmd8(LCD_CMD_DISPLAY_OFF);
    delay_ms(20);
    lcd_wrcmd8(LCD_CMD_SLEEPIN);
    delay_ms(120);
  }
  else
  {
    lcd_wrcmd8(LCD_CMD_SLEEPOUT);
    delay_ms(120);
    lcd_wrcmd8(LCD_CMD_DISPLAY_ON);
    delay_ms(20);
  }

  lcd_disable();

  return;
}


void __attribute__((optimize("Os"))) lcd_reset(void)
{
  uint32_t i;

  //init pins
  INIT_PINS();

  //hardware reset
  GPIO_CLRPIN(LCD_PORT, RST_PIN);
  delay_ms(20);
  GPIO_SETPIN(LCD_PORT, RST_PIN);
  delay_ms(120);

  lcd_enable();

  //lcd_wrcmd(LCD_CMD_RESET);
  //delay_ms(120);
  lcd_wrcmd8(LCD_CMD_DISPLAY_OFF);
  delay_ms(20);

  //send init commands
  lcd_wrcmd8(LCD_CMD_POWER_CTRLB);
  lcd_wrdata8(0x00);
  lcd_wrdata8(0x83); //83 81 AA
  lcd_wrdata8(0x30);

  lcd_wrcmd8(LCD_CMD_POWERON_SEQ_CTRL);
  lcd_wrdata8(0x64); //64 67
  lcd_wrdata8(0x03);
  lcd_wrdata8(0x12);
  lcd_wrdata8(0x81);

  lcd_wrcmd8(LCD_CMD_DRV_TIMING_CTRLA);
  lcd_wrdata8(0x85);
  lcd_wrdata8(0x01);
  lcd_wrdata8(0x79); //79 78

  lcd_wrcmd8(LCD_CMD_POWER_CTRLA);
  lcd_wrdata8(0x39);
  lcd_wrdata8(0X2C);
  lcd_wrdata8(0x00);
  lcd_wrdata8(0x34);
  lcd_wrdata8(0x02);

  lcd_wrcmd8(LCD_CMD_PUMP_RATIO_CTRL);
  lcd_wrdata8(0x20);

  lcd_wrcmd8(LCD_CMD_DRV_TIMING_CTRLB);
  lcd_wrdata8(0x00);
  lcd_wrdata8(0x00);

  lcd_wrcmd8(LCD_CMD_POWER_CTRL1);
  lcd_wrdata8(0x26); //26 25
  
  lcd_wrcmd8(LCD_CMD_POWER_CTRL2);
  lcd_wrdata8(0x11);

  lcd_wrcmd8(LCD_CMD_VCOM_CTRL1);
  lcd_wrdata8(0x35);
  lcd_wrdata8(0x3E);

  lcd_wrcmd8(LCD_CMD_VCOM_CTRL2);
  lcd_wrdata8(0xBE); //BE 94

  lcd_wrcmd8(LCD_CMD_FRAME_CTRL);
  lcd_wrdata8(0x00);
  lcd_wrdata8(0x1B); //1B 70

  //gamma control
  lcd_wrcmd8(LCD_CMD_ENABLE_3G);
  lcd_wrdata8(0x08); //08 00
  
  lcd_wrcmd8(LCD_CMD_GAMMA);
  lcd_wrdata8(0x01); //G2.2
  lcd_wrcmd8(LCD_CMD_POS_GAMMA);
  uint8_t pgama[15] = {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0x87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00};
  //uint8_t pgama[15] = {0x0F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0x87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00};
  for(i=0; i<15; i++)
  {
    lcd_wrdata8(pgama[i]);
  }
  lcd_wrcmd8(LCD_CMD_NEG_GAMMA);
  uint8_t ngama[15] = {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F};
  //uint8_t ngama[15] = {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x0F};
  for(i=0; i<15; i++)
  {
    lcd_wrdata8(ngama[i]);
  }

  lcd_wrcmd8(LCD_CMD_DISPLAY_CTRL);
  lcd_wrdata8(0x0A);
  lcd_wrdata8(0x82);
  lcd_wrdata8(0x27);
  lcd_wrdata8(0x00);

  lcd_wrcmd8(LCD_CMD_ENTRY_MODE);
  lcd_wrdata8(0x07);

  lcd_wrcmd8(LCD_CMD_PIXEL_FORMAT);
  lcd_wrdata8(0x55); //16bit

  lcd_disable();

  //set orientation
  lcd_setbias(0);

  lcd_enable();

  //clear display buffer
  lcd_drawstart();
  for(i=(LCD_WIDTH*LCD_HEIGHT); i!=0; i--)
  {
    lcd_draw(0);
  }
  lcd_drawstop();

  //display on / sleep out
  lcd_wrcmd8(LCD_CMD_SLEEPOUT);
  delay_ms(120);
  lcd_wrcmd8(LCD_CMD_DISPLAY_ON);
  delay_ms(20);

  lcd_disable();

  return;
}


void lcd_wrdata16(uint32_t data)
{
  //GPIO_SETPIN(LCD_PORT, RS_PIN); //data

  GPIO_CLRPIN(LCD_PORT, WR_PIN);
  DATA_WR(data>>8);
  GPIO_SETPIN(LCD_PORT, WR_PIN);

  //NOP();

  GPIO_CLRPIN(LCD_PORT, WR_PIN);
  DATA_WR(data);
  GPIO_SETPIN(LCD_PORT, WR_PIN);

  return;
}


void lcd_wrdata8(uint32_t data)
{
  //GPIO_SETPIN(LCD_PORT, RS_PIN); //data

  GPIO_CLRPIN(LCD_PORT, WR_PIN);
  DATA_WR(data);
  GPIO_SETPIN(LCD_PORT, WR_PIN);

  return;
}


void lcd_wrcmd8(uint32_t cmd)
{
  GPIO_CLRPIN(LCD_PORT, RS_PIN); //cmd

  GPIO_CLRPIN(LCD_PORT, WR_PIN);
  DATA_WR(cmd);
  GPIO_SETPIN(LCD_PORT, WR_PIN);

  GPIO_SETPIN(LCD_PORT, RS_PIN); //data

  return;
}


void lcd_disable(void)
{
  GPIO_SETPIN(LCD_PORT, CS_PIN);

  return;
}


void lcd_enable(void)
{
  GPIO_CLRPIN(LCD_PORT, CS_PIN);

  return;
}


#endif //LCD_MI0283QT9
