/*
 *
 * Name         :  pcd8544.h
 *
 * Description  :  This is header file for the PCD8544 graphic LCD driver.
 *                 Based on the code written by Sylvain Bissonette
 *
 * Author       :  Fandi Gunawan <fandigunawan@gmail.com>
 * Website      :  http://fandigunawan.wordpress.com
 *
 * Credit       :  Sylvain Bissonette (2003)
 *                 Louis Frigon (2003)
 *                 Other people (2010)
 *
 * License      :  GPL v. 3
 *
 * Compiler     :  WinAVR, GCC for AVR platform
 *                 Tested version :
 *                 - 20070525
 *                 - 20071221
 *                 - 20081225
 * Compiler note:  Please be aware of using older/newer version since WinAVR
 *                 is in extensive development. Please compile with parameter -O1
 *
 * History      :
 * Please refer to pcd8544.c
 */

#pragma once

#include <ATcommon/arch/avr/avr.hpp>

namespace Philips_PCD8544 {

/* For return value */
static const uint8_t OK = 0;
static const uint8_t OUT_OF_BORDER = 1;
static const uint8_t OK_WITH_WRAP = 2;

static const uint8_t LCD_X_RES = 84;    /* x resolution */
static const uint8_t LCD_Y_RES = 48;    /* y resolution */
static const uint8_t EMPTY_SPACE_BARS = 2;
static const uint8_t BAR_X            = 5;
static const uint8_t BAR_Y            = 38;

static const uint8_t MAX_X_FONT = LCD_X_RES / 6;
static const uint8_t MAX_Y_FONT = LCD_Y_RES / 8;

/* Cache size in bytes ( 84 * 48 ) / 8 = 504 bytes */
static const uint16_t LCD_CACHE_SIZE = ( LCD_X_RES * LCD_Y_RES ) / 8;

/* Type definition */
typedef uint8_t  byte;

// Modified by ANC 2010-04-24 (to eliminate signedness). (Was a signed int.)
typedef uint16_t CacheIndex_t;

/* Enumeration */
typedef enum {
    LCD_CMD  = 0,
    LCD_DATA = 1
} LcdCmdData;

typedef enum {
    PIXEL_OFF =  0,
    PIXEL_ON  =  1,
    PIXEL_XOR =  2
} PixelMode;

typedef enum {
    FONT_1X = 1,
    FONT_2X = 2
} LcdFontSize;

uint8_t get_font_byte(uint8_t x, uint8_t y);

// Architecture-specific delay routine.
static void Delay ( void );

template <typename SPI_bus_t, typename LCD_DC_pin_t, typename LCD_CE_pin_t, typename LCD_RST_pin_t>
class Philips_PCD8544 {
private:
// Architecture-specific hardware.
      SPI_bus_t SPI_bus;
   LCD_DC_pin_t LCD_DC_pin;
   LCD_CE_pin_t LCD_CE_pin;
  LCD_RST_pin_t LCD_RST_pin;

/* Cache buffer in SRAM 84*48 bits or 504 bytes */
  byte screenCache[ LCD_CACHE_SIZE ];

// Modified to eliminate signedness [ANC 2010-04-24]
/* Cache index */
  CacheIndex_t CacheIdx;
/* Lower part of water mark */
  CacheIndex_t LoWaterMark;
/* Higher part of water mark */
  CacheIndex_t HiWaterMark;

/* Variable to decide whether update Lcd Cache is active/nonactive */
  bool updateActive;


public:
  Philips_PCD8544(SPI_bus_t &new_SPI_bus, LCD_DC_pin_t &new_LCD_DC_pin, LCD_CE_pin_t &new_LCD_CE_pin, LCD_RST_pin_t &new_LCD_RST_pin)
  : SPI_bus(new_SPI_bus), LCD_DC_pin(new_LCD_DC_pin), LCD_CE_pin(new_LCD_CE_pin), LCD_RST_pin(new_LCD_RST_pin)
  { }
// When the pin and SPI bus objects are stateless, use this constructor.
  Philips_PCD8544()
  { }

/* Function prototypes */
  void send    ( byte data, LcdCmdData cd );
  void init       ( void );
  void clear      ( void );
  void update     ( void );
  void image      ( const byte *imageData );
  void contrast   ( byte contrast );
  byte gotoXYFont ( byte x, byte y );
  byte chr        ( LcdFontSize size, byte ch );
  byte str        ( LcdFontSize size, byte dataArray[] );
  byte fStr       ( LcdFontSize size, const byte *dataPtr );
  byte pixel      ( byte x, byte y, PixelMode mode );
  byte line       ( byte x1, byte x2, byte y1, byte y2, PixelMode mode );
  byte rect       ( byte x1, byte x2, byte y1, byte y2, PixelMode mode );
  byte singleBar  ( byte baseX, byte baseY, byte height, byte width, PixelMode mode );
  byte bars       ( byte data[], byte numbBars, byte width, byte multiplier );
};

// Inline for templates
#include "Philips_PCD8544.cpp"

// End namespace: Philips_PCD8544
}

