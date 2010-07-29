// Philips PCD8544 graphic LCD driver (C++).
// Licensed under GPLv3. See license.txt or <http://www.gnu.org/licenses/>.

#pragma once

#include "Philips_PCD8544.hpp"
#include <Upacket/Servers/SimpleServer.hpp>

/*
#ifndef DEBUGprint_MISC
#define DEBUGprint_MISC(...)
#endif
*/

namespace Philips_PCD8544{

template <typename LCD_t>
class StringServer : public SimpleServer, public Process {
  LCD_t *lcd;

public:
  StringServer(LCD_t *new_lcd)
  : lcd(new_lcd)
  { }

Status::Status_t process(){
  // Packet to process?
  if(! packetPending()) return Status::Status__Good;

  DEBUGprint_MISC("SS: Prc pk\n");

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

template <typename LCD_t>
class CommandServer : public SimpleServer, public Process {
  LCD_t *lcd;

public:

  typedef uint8_t Command_t;
  static const Command_t Command__ClearScreen = 0;
  static const Command_t Command__WriteBitmap = 1;
  static const Command_t Command__ReadBitmap  = 2;
  static const Command_t Command__SetContrast = 3;
  static const Command_t Command__WriteString = 4;

  CommandServer(LCD_t *new_lcd)
  : lcd(new_lcd)
  { }

Status::Status_t process(){
  // Packet to process?
  if(! packetPending()) return Status::Status__Good;

  // Data in packet?
  MAP::Data_t *data_ptr = offsetPacket.packet->get_data(offsetPacket.headerOffset);
  if(data_ptr == NULL) return finishedWithPacket();

  DEBUGprint_FORCE("BmS:Oc%d;", *data_ptr);

  // Command 0: Clear screen
  switch(*data_ptr){
  // Clear screen
    case Command__ClearScreen:
      lcd->clear();
      lcd->update();
     break;
  // Write bitmap
    case Command__WriteBitmap: {
      data_ptr++;
    // First byte is offset. Remaining data is actual image data.
      // Assumes no more than 254 bitmap bytes per packet.
      uint8_t packet_size = offsetPacket.packet->back() - data_ptr;
      // Only proceed if at least one byte is to be written. (Data starts at next byte.)
      if(packet_size <= 1) break;
      lcd->writeBitmap(data_ptr + 1, *data_ptr, packet_size - 1);
      lcd->update();
     break;
    }
  // Read bitmap
//    case Command__ReadBitmap:
//     break;
  // Set contrast
    case Command__SetContrast: {
    // First byte is new contrast.
      data_ptr++;
      uint8_t packet_size = offsetPacket.packet->back() - data_ptr;
      if(packet_size > 0)
        lcd->contrast(*data_ptr);
      lcd->update();
     break;
    }
  // Write string
//    case Command__WriteString:
//     break;
  // Unrecognized commands are ignored.
//    default:
  }

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

