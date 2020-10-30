
/******************************************************************************** 
 *  Airconditional remote control decoder for:
 *  
 *  ZH/LT-01 Remote control https://www.google.com/search?q=zh/lt-01
 *  
 *  The ZH/LT-01 remote control is used for many locally branded Split airconditioners, 
 *  so it is better to name this protocol by the name of the protocol rather then the
 *  name of the Airconditioner. For this project I used a Eurom-airconditioner, which is 
 *  Dutch-branded and sold in the Netherlands at Hornbach.
 *  
 * For airco-brands: 
 *  Eurom
 *  Chigo
 *  Tristar
 *  Tecnomaster
 *  Elgin
 *  Geant
 *  Tekno
 *  Topair
 *  Proma
 *  Sumikura
 *  JBS
 *  Turbo Air
 *  Nakatomy
 *  Celestial Air
 *  Ager
 *  Blueway
 *  Airlux
 *  Etc.
 *  
 ***********************************************************************************  
 *  SUMMARY FUNCTIONAL DESCRIPTION
 **********************************************************************************
 *  The remote sends a 12 Byte message which contains all possible settings every
 *  time.
 *  
 *  Byte 11 (and 10) contain the remote control identifier and are always 0xD5 and
 *  0x2A respectively for the ZH/LT-01 remote control. 
 *  Every UNeven Byte (01,03,05,07 and 09) hold command data
 *  Every EVEN Byte (00,02,04,06,08 and 10) hold a checksum of the corresponding 
 *  command-, or identifier-byte by inverting the bits, for example:
 *  
 *  The identifier byte[11] = 0xD5 = B1101 0101
 *  The checksum byte[10]   = 0x2A = B0010 1010
 *  
 *  So, you can check the message by:
 *  - inverting the bits of the checksum byte with the corresponding command-, or 
 *    identifier byte, they should me the same, or
 *  - Summing up the checksum byte and the corresponding command-, or identifier byte,
 *    they should always add up to 0xFF = B11111111 = 255
 *  
 *  Control bytes:
 *  [01] - Timer (1-24 hours, Off)
 *  [03] - LAMP ON/OFF, TURBO ON/OFF, HOLD ON/OFF 
 *  [05] - Indicates which button the user pressed on the remote control
 *  [07] - POWER ON/OFF, FAN AUTO/3/2/1, SLEEP ON/OFF, AIRFLOW ON/OFF,
 *         VERTICAL SWING/WIND/FIXED
 *  [09] - MODE AUTO/COOL/VENT/DRY/HEAT, TEMPERATURE (16 - 32°C)
 *  
 *  TIMINGS 
 *  (Both Hyundai (option 4) and Samsung (option 5) timings work)
 *  Space:        Not used
 *  Header Mark:  6200 us
 *  Header Space: 7500 us
 *  Bit Mark:      475 us
 *  Zero Space:    525 us
 *  One Space:    1650 us
 *  
 * ****************************************************************************** 
 *  Written by: Marcel van der Kooij 
 *  Date: 2018-06-07
 *  Version: 1.0
 *******************************************************************************/

#include <Arduino.h> 

bool decodeZHLT01remote(byte *bytes, int byteCount)
{

/********************************************************************************
 * Is this a ZH/LT-01 Remote?
 * Message should by 12 bytes, 100 symbols.
 * Remote identifier
 * Byte[11]: Always 0xD5, which makes Byte[10] always 0x2A (see checksum)
 * 
 *******************************************************************************/
  
  if (byteCount == 12 && bytes[10] == 0x2A && bytes[11] == 0xD5) {
    Serial.println(F("Looks like a ZH/LT-01 remote control protocol"));

/*******************************************************************************
 * Byte[01]: Timer (1-24 hours, Not used, Off, Changed, Set)
 * TIMER ON/OFF:  B1xxxxxxx
 * TIMER CHANGED: Bxx1xxxxx
 * 
 * Number of hours is determined by bit0-4:
 * 0x01 = 1 hour
 * 0x18 = 24 hours
 *******************************************************************************/

     Serial.print(F("bytes[1] bit5-7 Timer:          "));
     switch (bytes[1] & B10100000) {
       case 0x00:
         Serial.println(F("Not used"));
         break;
       case B00100000:
         Serial.println(F("Off"));
         break;
       case B10000000:
         Serial.println(F("Set"));
         break;
       case B10100000:
         Serial.println(F("Changed"));
         break;
       default:
         Serial.println("Unknown"); 
     }

     Serial.print(F("bytes[1] bit0-4 Hours:          "));
     Serial.println(bytes[1] & B00011111);



/********************************************************************************
 * Byte[03]: LAMP, TURBO, HOLD ON/OFF
 * LAMP ON:  Bxxxxxxx1 (bit0)
 * LAMP OFF: Bxxxxxxx0
 * HOLD ON:  Bxxxxx1xx (bit2)
 * HOLD ON:  Bxxxxx0xx
 * TURBO ON: Bxxxx1xxx (bit3)
 * TURBO ON: Bxxxx0xxx
 *******************************************************************************/
     // LAMP ON/OFF
     Serial.print(F("bytes[3] bit0   Lamp:           "));
     switch (bytes[3]& B00000001) {
       case B00000000:
         Serial.println(F("Off"));
         break;
       case B00000001:
         Serial.println(F("On"));
         break;
       default:
         Serial.println(F("Unknown"));
     }

     // HOLD ON/OFF
     Serial.print(F("bytes[3] bit2   Hold:           "));
     switch (bytes[3]& B00000100) {
       case B00000000:
         Serial.println(F("Off"));
         break;
       case B00000100:
         Serial.println(F("On"));
         break;
       default:
         Serial.println(F("Unknown"));
     }

     // TURBO ON/OFF
     Serial.print(F("bytes[3] bit3   Turbo:          "));
     switch (bytes[3]& B00001000) {
       case B00000000:
         Serial.println(F("Off"));
         break;
       case B00001000:
         Serial.println(F("On"));
         break;
       default:
         Serial.println(F("Unknown"));
     }

/********************************************************************************
 * Byte[05]: Button pressed
 * 0x00: POWER
 * 0x01: MODE
 * 0x02: Temperature Up
 * 0x03: Temperature Down
 * 0x04: Vertical Swing
 * 0x05: FAN
 * 0x06: TIMER
 * 0x07: Horizontal swing
 * 0x08: HOLD
 * 0x09: SLEEP
 * 0x0A: TURBO
 * 0x0B: LAMP
 *******************************************************************************/
 
   Serial.print(F("bytes[5]        Button pressed: "));
     switch (bytes[5]) {
       case 0x00:
         Serial.println(F("POWER"));
         break;
       case 0x01:
         Serial.println(F("MODE"));
         break;
       case 0x02:
         Serial.println(F("TEMP UP"));
         break;
       case 0x03:
         Serial.println(F("TEMP DOWN"));
         break;
       case 0x04:
         Serial.println(F("SWING (vertical)"));
         break;
       case 0x05:
         Serial.println(F("FAN SPEED"));
         break;
       case 0x06:
         Serial.println(F("TIMER"));
         break;
       case 0x07:
         Serial.println(F("AIR FLOW (horizontal)"));
         break;
       case 0x08:
         Serial.println(F("HOLD"));
         break;
       case 0x09:
         Serial.println(F("SLEEP"));
         break;
       case 0x0A:
         Serial.println(F("TURBO"));
         break;
       case 0x0B:
         Serial.println(F("LAMP"));
         break;
       default:
         Serial.println(F("Unknown"));
     }

/********************************************************************************
 * Byte[07]: POWER, FAN, SLEEP, HORIZONTAL, VERTICAL
 * SLEEP ON:         Bxxxxxxx1 (LSB)
 * SLEEP OFF:        Bxxxxxxx0
 * POWER ON:         Bxxxxxx1x
 * POWER OFF:        Bxxxxxx0x
 * VERTICAL SWING:   Bxxxx01xx
 * VERTICAL WIND:    Bxxxx00xx
 * VERTICAL FIXED:   Bxxxx10xx
 * HORIZONTAL SWING: Bxxx0xxxx
 * HORIZONTAL OFF:   Bxxx1xxxx
 * FAN AUTO:         Bx00xxxxx
 * FAN3:             Bx01xxxxx
 * FAN2:             Bx10xxxxx
 * FAN1:             Bx11xxxxx
 *******************************************************************************/

     // SLEEP ON/OFF
     Serial.print(F("bytes[7] bit0   Sleep:          "));
     switch (bytes[7]& B00000001) {
       case B00000000:
         Serial.println(F("Off"));
         break;
       case B00000001:
         Serial.println(F("On"));
         break;
       default:
         Serial.println(F("Unknown"));
     }

     // POWER ON/OFF
     Serial.print(F("bytes[7] bit1   Power:          "));
     switch (bytes[7]& B00000010) {
       case B00000000:
         Serial.println(F("Off"));
         break;
       case B00000010:
         Serial.println(F("On"));
         break;
       default:
         Serial.println(F("Unknown"));
     }

     // Vertical air swing
     Serial.print(F("bytes[7] bit2-3 Vertical:       "));
     switch (bytes[7] & B00001100) {
       case B00000100:
         Serial.println(F("Swing"));
         break;
       case 0x00:
         Serial.println(F("Wind"));
         break;
       case B00001000:
         Serial.println(F("Fixed"));
         break;
       default:
         Serial.println(F("Unknown"));
     }

     // Air flow (Horizontal) ON/OFF
     Serial.print(F("bytes[7] bit4   Air flow:       "));
     switch (bytes[7]& B00010000) {
       case B00000000:
         Serial.println(F("Swing"));
         break;
       case B00010000:
         Serial.println(F("Fixed"));
         break;
       default:
         Serial.println(F("Unknown"));
     }
     
     // Fan Speed
     Serial.print(F("bytes[7] bit5-6 Fan Speed:      "));
     switch (bytes[7] & B01100000) {
       case B00000000:
         Serial.println(F("Auto"));
         break;
       case B00100000:
         Serial.println(F("3"));
         break;
       case B01000000:
         Serial.println(F("2"));
         break;
       case B01100000:
         Serial.println(F("1"));
         break;
       default:
         Serial.println(F("Unknown"));
     }

/********************************************************************************
 * Byte[09]: Mode, Temperature
 * MODE AUTO: B000xxxxx
 * MODE COOL: B001xxxxx
 * MODE VENT: B011xxxxx
 * MODE DRY:  B010xxxxx
 * MODE HEAT: B100xxxxx
 * Temperature is determined by bit0-4:
 * 0x00 = 16C
 * 0x10 = 32C
 *******************************************************************************/

     // MODE
     Serial.print(F("bytes[9] bit5-7 Mode:           "));
     switch (bytes[9] & B11100000) {
       case 0x00:
         Serial.println(F("Auto"));
         break;
       case B00100000:
         Serial.println(F("Cool"));
         break;
       case B01100000:
         Serial.println(F("Ventilate"));
         break;
       case B01000000:
         Serial.println(F("Dry"));
         break;
       case B10000000:
         Serial.println(F("Heat"));
         break;
       default:
         Serial.println("Unknown"); 
     }

     //Temperature
     Serial.print("Bytes[9]        Temperature:    ");
     Serial.print((bytes[9] & B00011111)+16);
     Serial.println("°C");

// Checksum
     bool checksum = true;

     for (int i = 0; i < 12; i+=2) {
       checksum &= (bytes[i] + bytes[i+1] == 255);
     }

     Serial.println(checksum?"Checksum:       MATCHES":"Checksum:       DOES NOT MATCH");

     return true;
   }

   return false;
}

