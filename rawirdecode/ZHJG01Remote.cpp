
/******************************************************************************** 
 *  Airconditional remote control decoder for:
 *  
 *  ZH/JG-01 Remote control https://www.google.com/search?q=zh/JG-01
 *  
 *  The ZH/JG-01 remote control is used for many locally branded Split airconditioners, 
 *  so it is better to name this protocol by the name of the protocol rather then the
 *  name of the Airconditioner. For this project I used a TACHIAIR airconditioner.
 *  
 * For airco-brands: 
 *  Tachiair
 *  Chigo
 *  Etc.
 *  
 ***********************************************************************************  
 *  SUMMARY FUNCTIONAL DESCRIPTION
 **********************************************************************************
 *  The remote sends a 6 Byte message which contains all possible settings every
 *  time.
 *  
 *  Every UNeven Byte (01,03,05,07 and 09)  hold command data
 *  Every EVEN Byte (00,02,04,06,08 and 10) hold a checksum of the corresponding
 *  command by inverting the bits, for example:
 *  
 *  The checksum byte[0]   = 0x2A = B0010 1010
 *  The identifier byte[1] = 0xD5 = B1101 0101
 *  
 *  So, you can check the message by:
 *  - inverting the bits of the checksum byte with the corresponding command, they 
 *    should be the same, or
 *  - Summing up the checksum byte and the corresponding command,
 *    they should always add up to 0xFF = B11111111 = 255
 *  
 *  TIMINGS 
 *  (Only ZH/JG-01 (option 6) work)
 *  Space:        Not used
 *  Header Mark:  6550 us
 *  Header Space: 0 us
 *  Bit Mark:     560 us
 *  Zero Space:   1530 us
 *  One Space:    3750 us
 *  
 * ****************************************************************************** 
 *  Written by: Abílio Costa
 *  Date: 2023-07-03
 *  Version: 1.0
 *******************************************************************************/

#include <Arduino.h> 

byte reverseByte(byte input) {
  byte output = 0;
  for (int i = 0; i < 8; ++i) {
    output <<= 1;
    output |= (input & 1);
    input >>= 1;
  }
  return output;
}

bool decodeZHJG01remote(byte *bytes, int byteCount)
{

/********************************************************************************
 * Is this a ZH/JG-01 Remote?
 * Message should by 6 bytes, 52 symbols, with valid checksum.
 * 
 *******************************************************************************/
  
  if (byteCount != 6) {
    return false;
  }
  
  bool checksum = true;
  for (int i = 0; i < 6; i+=2) {
    checksum &= (bytes[i] + bytes[i+1] == 255);
  }

  if (!checksum)  {
    return false;
  }

  Serial.println(F("Looks like a ZH/JG-01 remote control protocol"));

/********************************************************************************
 * Byte[1]: Turbo, Eco, Fan, Vertical Swing
 * TURBO ON:         B1x0xxxxx
 * ECO ON:           B1x1xxxxx
 * TURBO/ECO OFF:    B0xxxxxxx
 * FAN1:             Bx11xxxxx
 * FAN2:             Bx10xxxxx
 * FAN3:             Bx01xxxxx
 * FAN AUTO:         Bx00xxxxx
 * VERTICAL FIXED:   Bxxx10xxx
 * VERTICAL SWING:   Bxxx01xxx
 * VERTICAL WIND:    Bxxx00xxx
 *******************************************************************************/
  // TURBO ON/OFF
  Serial.print(F("bytes[1] bit0+2 Turbo:          "));
  switch (bytes[1]& B10100000) {
    case B10000000:
      Serial.println(F("On"));
      break;
    default:
      Serial.println(F("Off"));
  }

  // FAN ON/OFF
  Serial.print(F("bytes[1] bit0+2 Eco:            "));
  switch (bytes[1]& B10100000) {
    case B10100000:
      Serial.println(F("On"));
      break;
    default:
      Serial.println(F("Off"));
  }

  Serial.print(F("bytes[1] bit1-2 Fan Speed:      "));
  switch (bytes[1] & B01100000) {
    case B01100000:
      Serial.println(F("1"));
      break;
    case B01000000:
      Serial.println(F("2"));
      break;
    case B00100000:
      Serial.println(F("3"));
      break;
    case B00000000:
      Serial.println(F("Auto"));
      break;
    default:
      Serial.println(F("Unknown"));
  }

  // Vertical air swing
  Serial.print(F("bytes[1] bit3-4 Vertical:       "));
  switch (bytes[1] & B00011000) {
    case B00001000:
      Serial.println(F("Swing"));
      break;
    case B00000000:
      Serial.println(F("Wind"));
      break;
    case B00010000:
      Serial.println(F("Fixed"));
      break;
    default:
      Serial.println(F("Unknown"));
  }

/********************************************************************************
 * Byte[3]: Temp, Power, Mode
 * TEMP:      Bttttxxxx
 * POWER ON:  Bxxxx1xxx
 * POWER OFF: Bxxxx0xxx
 * MODE HEAT: Bxxxxx100
 * MODE VENT: Bxxxxx011
 * MODE DRY:  Bxxxxx010
 * MODE COOL: Bxxxxx001
 * MODE AUTO: Bxxxxx000
 *******************************************************************************/

  // TEMPERATURE
  byte tempByte = 0;
  Serial.print("bytes[3]        Temperature:    ");
  Serial.print(((bytes[3] >> 4) & 0b1111) + 17);
  Serial.println("°C");

  // POWER ON/OFF
  Serial.print(F("bytes[3] bit4   Power:          "));
  switch (bytes[3] & B00001000) {
    case B00000000:
      Serial.println(F("Off"));
      break;
    case B00001000:
      Serial.println(F("On"));
      break;
    default:
      Serial.println(F("Unknown"));
  }

  // MODE
  Serial.print(F("bytes[3] bit5-7 Mode:           "));
  switch (bytes[3] & B00000111) {
    case B00000000:
      Serial.println(F("Auto"));
      break;
    case B00000001:
      Serial.println(F("Cool"));
      break;
    case B00000011:
      Serial.println(F("Ventilate"));
      break;
    case B00000010:
      Serial.println(F("Dry"));
      break;
    case B00000100:
      Serial.println(F("Heat"));
      break;
    default:
      Serial.println("Unknown"); 
  }

  return true;
}

