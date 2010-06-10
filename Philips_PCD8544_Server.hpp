#pragma once

#include "Philips_PCD8544.hpp"
#include <Upacket/Servers/SimpleServer.hpp>

#ifndef DEBUGprint_MISC
#define DEBUGprint_MISC(...)
#endif

namespace Philips_PCD8544{

template <typename LCD_t>
class StringLCDServer : public SimpleServer, public Process {
  LCD_t *lcd;

public:
  StringLCDServer(LCD_t *new_lcd)
  : lcd(new_lcd)
  { }

Status::Status_t process(){
  // Packet to process?
  if(! packetPending()) return Status::Status__Good;

  DEBUGprint_MISC("SLS: Prc pk\n");

  // Data in packet?
  MAP::Data_t *data_ptr = offsetPacket.packet->get_data(offsetPacket.headerOffset);
  if(data_ptr == NULL) return finishedWithPacket();

  // Clear screen
  lcd->clear();

  // Write packet contents out to screen, beginning at first row and performing a linefeed
  // when the right edge of the screen is encountered.
  for(uint8_t Y = 1; Y <= LCD_t::MAX_Y_FONT; Y++){
    // Start at leftmost edge of screen
    lcd->gotoXYFont(1,Y);
    while(data_ptr < offsetPacket.packet->back()){
      uint8_t response = lcd->chr(FONT_1X, *data_ptr);
      if(response != OK){
      // If wrapped, the character has actually been written.
        if(response == OK_WITH_WRAP) data_ptr++;
      // Move to next line
        break;
      }
      // Move to next character
      data_ptr++;
    }
  }

  // Update screen
  lcd->update();

  return finishedWithPacket();
}
};

/*
class GalvLCDServer : public SimpleServer {
  process(){
    // Packet to process?
    if(! packetPending()) return;

    // Data in packet?
    int32_t raw_value;
    if(! packet->sourceC78Signed(raw_value, packet->get_data())) return;
    FixedPoint<uint16_t, 14> value(raw_value, true); 

    // Display galvanometer
    LCDClear();
    LCDGalvDisplay(value);
    LCDUpdate();
  }
};
*/

// End namespace: Philips_PCD8544
}

