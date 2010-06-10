
// AVR architecture font retrieval.

#include <ATcommon/arch/avr/avr.hpp>

namespace Philips_PCD8544 {

// This table defines the standard ASCII characters in a 5x7 dot format.
const uint8_t FontLookup [91][5] PROGMEM =
#include "../../sbFont.hpp"

uint8_t get_font_byte(uint8_t x, uint8_t y){
  return pgm_read_byte(&( FontLookup[x][y] ) );
}

// End namespace: Philips_PCD8544
};

