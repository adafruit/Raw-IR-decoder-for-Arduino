
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
 *  Every EVEN Byte (00,02,04,06,08 and 10) hold command data
 *  Every UNeven Byte (01,03,05,07 and 09)  hold a checksum of the corresponding
 *  command by inverting the bits, for example:
 *  
 *  The identifier byte[0] = 0xD5 = B1101 0101
 *  The checksum byte[1]   = 0x2A = B0010 1010
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
 * Byte[0]: Turbo, Eco, Fan, Vertical Swing
 * TURBO ON:         B0x1xxxxx
 * ECO ON:           B0x0xxxxx
 * TURBO/ECO OFF:    B1xxxxxxx
 * FAN1:             Bx00xxxxx
 * FAN2:             Bx01xxxxx
 * FAN3:             Bx10xxxxx
 * FAN AUTO:         Bx11xxxxx
 * VERTICAL FIXED:   Bxxx01xxx
 * VERTICAL SWING:   Bxxx10xxx
 * VERTICAL WIND:    Bxxx11xxx
 *******************************************************************************/
  // TURBO ON/OFF
  Serial.print(F("bytes[0] bit0+2 Turbo:          "));
  switch (bytes[0]& B10100000) {
    case B00100000:
      Serial.println(F("On"));
      break;
    default:
      Serial.println(F("Off"));
  }

  // FAN ON/OFF
  Serial.print(F("bytes[0] bit0+2 Eco:            "));
  switch (bytes[0]& B10100000) {
    case B00000000:
      Serial.println(F("On"));
      break;
    default:
      Serial.println(F("Off"));
  }

  Serial.print(F("bytes[0] bit1-2 Fan Speed:      "));
  switch (bytes[0] & B01100000) {
    case B00000000:
      Serial.println(F("1"));
      break;
    case B00100000:
      Serial.println(F("2"));
      break;
    case B01000000:
      Serial.println(F("3"));
      break;
    case B01100000:
      Serial.println(F("Auto"));
      break;
    default:
      Serial.println(F("Unknown"));
  }

  // Vertical air swing
  Serial.print(F("bytes[0] bit3-4 Vertical:       "));
  switch (bytes[0] & B00011000) {
    case B00010000:
      Serial.println(F("Swing"));
      break;
    case B00011000:
      Serial.println(F("Wind"));
      break;
    case B00001000:
      Serial.println(F("Fixed"));
      break;
    default:
      Serial.println(F("Unknown"));
  }

/********************************************************************************
 * Byte[2]: Temp, Power, Mode
 * TEMP:      Bttttxxxx
 * POWER ON:  Bxxxx0xxx
 * POWER OFF: Bxxxx1xxx
 * MODE HEAT: Bxxxxx011
 * MODE VENT: Bxxxxx100
 * MODE DRY:  Bxxxxx101
 * MODE COOL: Bxxxxx110
 * MODE AUTO: Bxxxxx111
 *******************************************************************************/

  // TEMPERATURE
  byte tempByte = 0;
  Serial.print("bytes[2]        Temperature:    ");
  Serial.print(((~bytes[2] >> 4) & 0b1111) + 17);
  Serial.println("°C");

  // POWER ON/OFF
  Serial.print(F("bytes[2] bit4   Power:          "));
  switch (bytes[2] & B00001000) {
    case B00001000:
      Serial.println(F("Off"));
      break;
    case B00000000:
      Serial.println(F("On"));
      break;
    default:
      Serial.println(F("Unknown"));
  }

  // MODE
  Serial.print(F("bytes[2] bit5-7 Mode:           "));
  switch (bytes[2] & B00000111) {
    case B00000111:
      Serial.println(F("Auto"));
      break;
    case B00000110:
      Serial.println(F("Cool"));
      break;
    case B00000100:
      Serial.println(F("Ventilate"));
      break;
    case B00000101:
      Serial.println(F("Dry"));
      break;
    case B00000011:
      Serial.println(F("Heat"));
      break;
    default:
      Serial.println("Unknown"); 
  }

  return true;
}

