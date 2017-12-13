#include <Arduino.h>

/*
Decoder for Fujitsu remote control

Pulls apart packet and shows the meaning of each part.

Fujitsu Model AR-RAH1E remote control codes by Andrew Errington 2017
Based on Fujitsu AR-RY16 codes by David Abrams 2009

Carrier frequency   38 kHz +/- 1%
Carrier period      26.3 us

Basic unit          410 us +/- 3% (approx. 16 cycles of carrier)

Leader:             125 (0x7C) cycles carrier on
followed by         62 (0x3E) cycles carrier off

Trailer             16 (0x10) cycles carrier on
followed by         at least 305 (0x130) cycles carrier off

Data '1' bit        16 (0x10) cycles carrier on
                    16 (0x10) cycles carrier off

Data '0' bit        16 (0x10) cycles carrier on
                    46 (0x2E) cycles carrier off

There are two kinds of packet. A short packet is 7 bytes long and
usually encodes a simple command, such as toggling a feature on or off.
A full packet is 16 bytes long, and encodes the full state of the
remote control including the mode, fan speed, temperature, etc.

Short packet (7 bytes)

Byte  0 Marker code (M1)        0x14
Byte  1 Marker code (M2)        0x63
Byte  2 Signal code             0x00
Byte  3 Custom code (C1)        0x10
Byte  4 Sub custom code (C2)    0x10
Byte  5 Command code (D)        0xXX
Byte  6 Checksum                0x??


The a/c units and remote can be configured with one of four signal
codes (A, b, c, or d). This allows different a/c units to be controlled
by different remotes in the same room.

Signal code (Byte 2<5:4>)

0x00    Signal code A (default)
0x01    Signal code b
0x02    Signal code c
0x03    Signal code d


Byte 5 contains the command. If it's not 0xFE then we have a short
packet, with a simple function.

Command codes (Byte 5<7:0>)

0x02    Off
0x03    Test
0x09    Toggle economy mode
0x3F    Filter Reset*
0x2D    Super quiet*
0x6C    Step horizontal vane up/down
0x79    Step vertical vane left/right
0xFE    Full packet

* Not for AR-RAH1E


Checksum (Byte 6<7:0>) (short packet)

Checksum = 0xFF - Byte 5



Full packet (16 bytes)

Byte  0 Marker code (M1)        0x14
Byte  1 Marker code (M2)        0x63
Byte  2 Signal code             0x00
Byte  3 Custom code (C1)        0x10
Byte  4 Sub custom code (C2)    0x10
Byte  5 Command code (D)        0xFE (indicates full packet)
Byte  6 Unknown                 0x09
Byte  7 Unknown                 0x30
Byte  8 <7:4> temperature
        <3:1> unknown
        <0>   on/off state
Byte  9 <7:4> timer mode
        <3:0> mode
Byte 10 <7:4> swing mode
        <3:0> fan mode
Byte 11 Timer off value low byte
Byte 12 <7:4> timer on value bits 3:0
        <3>   new timer off value
        <2:0> timer off value bits 10:8
Byte 13 <7>   new timer on value
        <6:0> timer on value bits 10:4
Byte 14 Unknown                 0x20
Byte 15 Checksum                0x??


Temperature (Byte 8<7:4>)

0x00    16C (60F)*      0x08    24C (76F)
0x01    17C (62F)*      0x09    25C (78F)
0x02    18C (64F)       0x0A    26C (80F)
0x03    19C (66F)       0x0B    27C (82F)
0x04    20C (68F)       0x0C    28C (84F)
0x05    21C (70F)       0x0D    29C (86F)
0x06    22C (72F)       0x0E    30C (88F)
0x07    23C (74F)       0x0F    Unknown

Byte 8<0>, if 1, unit was off and is now on, if 0 unit was already on

* For AR-RAH1E, heat setting is 16-30C, cool setting is 18-30C


Timer mode (Byte 9<7:4>)

0x00    Timer off
0x01    Sleep mode
0x02    Timer off time
0x03    Timer on time
0x04    Off -> On


Mode (Byte 9<3:0>)

0x00    Auto
0x01    Cool
0x02    Dry
0x03    Fan
0x04    Heat


Swing mode (Byte 10<7:4>)

0x00    Off
0x01    Horizontal vanes oscillation up/down
0x02    Vertical vanes oscillation left/right
0x03    Both hor. and vert. vanes oscillation


Fan mode (Byte 10<3:0>)

0x00    Auto
0x01    High
0x02    Medium
0x03    Low
0x04    Quiet


Timer modes (Byte 11-13)

Timer off

Timer off value (Byte 12<2:0> Byte 11<7:0>)
11-bit value (0 to 595)
For Sleep mode, number of minutes to run before turning off.
For Timer Off mode, number of minutes from now until turn off.

New timer off value (Byte 12<3>)
Set when new Sleep or Timer Off value is sent.

Timer on

Timer on value (Byte 13<6:0> Byte 12<7:4>)
11-bit value, number of minutes from now until turn on.

New timer on value (Byte 13<7>)
Set when new Timer On value is sent.


Unknown (Byte 14)

For AR-RY16 and AR-RAH1E constant 0x20
Some reports indicate this may be an Eco setting for some models


Checksum (Byte 15)

Checksum = 0 - (sum of Bytes 7 to 14)

*/

static byte header[] = {0x14,0x63};

bool decodeFujitsu(byte *bytes, int byteCount)
{
  if(byteCount > sizeof(header) && memcmp(bytes, header, sizeof(header)) == 0) {
    Serial.println(F("Looks like a Fujitsu protocol"));

    byte checksum = 0;
   
    //Calculate checksum
    if (byteCount == 7) {
        checksum = 0xFF - bytes[5];
    } else {
        for (int i=7; i<byteCount-1; i++) {
          checksum -= bytes[i];
        }
    }

    if (checksum == bytes[byteCount-1]) {
      Serial.println(F("Checksum matches"));
    } else {
      Serial.print(F("Checksum does not match: 0x"));
      Serial.print(checksum, HEX);
      Serial.print(F(" vs 0x"));
      Serial.println(bytes[byteCount-1], HEX);
    }


    // Now pick apart the packet

    if (bytes[5] == 0xFE) {
      if (byteCount != 16) {
        Serial.println(F("Long command (byte 5 is 0xFE), but packet is not 16 bytes long!"));
        return false;
      } else {
        Serial.println(F("Long command (byte 5 is 0xFE)."));
      }
    }
   
    if (bytes[5] != 0xFE) {
      if (byteCount != 7) {
        Serial.println(F("Short command (byte 5 is not 0xFE), but packet is not 7 bytes long!"));
        return false;
      } else {
        Serial.println(F("Short command (byte 5 is not 0xFE)."));
      }
    }


    // Bytes 0, 1, 3, and 4 are (probably) constant

    Serial.print(F("Byte 0: 0x"));
    Serial.print(bytes[0], HEX);
    Serial.print(F(" expected 0x14 "));
    if (bytes[0] == 0x14) {
      Serial.println(F("[OK]"));
    } else {
      Serial.println(F("[Error]"));
    }
    
    Serial.print(F("Byte 1: 0x"));
    Serial.print(bytes[1], HEX);
    Serial.print(F(" expected 0x63 "));
    if (bytes[1] == 0x63) {
      Serial.println(F("[OK]"));
    } else {
      Serial.println(F("[Error]"));
    }

    // Byte 2 contains the Remote Control Unit Signal Code (Abcd)
    // Default is A

    Serial.print(F("Byte 2: 0x"));
    Serial.println(bytes[2], HEX);

    // Remote Control Unit Signal Code
    Serial.print(F("Signal Code, byte 2<5:4>: 0x"));
    Serial.print((bytes[2] & 0x30) >> 4, HEX);
    Serial.print(F("\t\t"));

    switch ((bytes[2] & 0x30) >> 4) {
      case 0x00:
        Serial.println(F("A (Default)"));
        break;
      case 0x01:
        Serial.println(F("b"));
        break;
      case 0x02:
        Serial.println(F("c"));
        break;
      case 0x03:
        Serial.println(F("d"));
        break;
    }


    Serial.print(F("Byte 3: 0x"));
    Serial.print(bytes[3], HEX);
    Serial.print(F(" expected 0x10 "));
    if (bytes[3] == 0x10) {
      Serial.println(F("[OK]"));
    } else {
      Serial.println(F("[Error]"));
    }

    Serial.print(F("Byte 4: 0x"));
    Serial.print(bytes[4], HEX);
    Serial.print(F(" expected 0x10 "));
    if (bytes[4] == 0x10) {
      Serial.println(F("[OK]"));
    } else {
      Serial.println(F("[Error]"));
    }


    // Byte 5 encodes the command.
    // If it's not 0xFE then it's a short command (byte 5 is the command)
    // If it is 0xFE then it's a long command and data is encoded
    // in the following bytes.

    Serial.print(F("Byte 5: 0x"));
    Serial.print(bytes[5], HEX);

    if (bytes[5] != 0xFE) {
      // Short command
      Serial.print(F(" Short command:\t\t"));

      switch (bytes[5]) {
        case 0x02:
          Serial.println(F("POWER OFF"));
          break;
        case 0x03:
          Serial.println(F("TEST"));
          break;
        case 0x09:
          Serial.println(F("TOGGLE ECONOMY MODE"));
          break;
        case 0x2D:
          // Not AR-RAH1E
          Serial.println(F("SUPER QUIET"));
          break;
        case 0x39:
          // Not AR-RAH1E
          Serial.println(F("HIGH POWER"));
          break;
        case 0x3F:
          // Not AR-RAH1E
          Serial.println(F("FILTER RESET"));
          break;
        case 0x6C:
          Serial.println(F("STEP HORIZONTAL VANE UP/DOWN"));
          break;
        case 0x79:
          Serial.println(F("STEP VERTICAL VANE LEFT/RIGHT"));
          break;
        default:
          Serial.println(F("UNKNOWN"));
          break;
      }

    // Byte 6 is checksum in short command mode

    Serial.print(F("Byte 6 (Checksum): 0x"));
    Serial.println(bytes[6], HEX);

    return true;
      
    }


    // Otherwise, long command
    
    Serial.println(F(" Long command"));


    // Bytes 6 and 7 are constant

    Serial.print(F("Byte 6: 0x"));
    Serial.print(bytes[6], HEX);
    Serial.print(F(" expected 0x09 "));
    if (bytes[6] == 0x09) {
      Serial.println(F("[OK]"));
    } else {
      Serial.println(F("[Error]"));
    }

    Serial.print(F("Byte 7: 0x"));
    Serial.print(bytes[7], HEX);
    Serial.print(F(" expected 0x30 "));
    if (bytes[7] == 0x30) {
      Serial.println(F("[OK]"));
    } else {
      Serial.println(F("[Error]"));
    }


    // Byte 8 encodes temperature

    Serial.print(F("Byte 8: 0x"));
    Serial.println(bytes[8], HEX);

    Serial.print(F("Temperature, byte 8<7:4>: 0x"));
    Serial.print(bytes[8] >> 4, HEX);

    if ((bytes[8] >> 4) != 0x0F) {
      Serial.print(F("\t\t"));
      Serial.print((bytes[8] >> 4) + 16);
      Serial.print(F("C ("));
      Serial.print((bytes[8] >> 3) + 60);
      Serial.println(F("F)"));
    } else {
      Serial.println(F(" UNKNOWN"));
    }

    Serial.print(F("Unit state, byte 8 <0>: 0x"));
    Serial.print(bytes[8] & 0x01,HEX);
    
    if ((bytes[8] & 0x01) == 0x01) {
      Serial.println(F("\t\tUnit was off and is now on."));
    } else {
      Serial.println(F("\t\tUnit was already on."));
    }

    Serial.print(F("Unknown bits, Byte 8 <3:1>: 0x"));
    Serial.println(bytes[8] & 0x0E >> 1,HEX);


    // Byte 9 encodes timer and main operating mode

    Serial.print(F("Byte 9: 0x"));
    Serial.println(bytes[9], HEX);

    // Timer mode
    Serial.print(F("Timer mode, byte 9<7:4>: 0x"));
    Serial.print(bytes[9] >> 4, HEX);
    Serial.print(F("\t\t"));

    switch (bytes[9] >> 4) {
      case 0x00:
        Serial.println(F("TIMER OFF"));
        break;
      case 0x01:
        Serial.println(F("TIMER SLEEP"));
        break;
      case 0x02:
        Serial.println(F("SET TIMER OFF TIME"));
        break;
      case 0x03:
        Serial.println(F("SET TIMER ON TIME"));
        break;
      case 0x04:
        Serial.println(F("SET TIMER (OFF -> ON)"));
        break;
      default:
        Serial.println(F("UNKNOWN"));
        break;
    }

    // Main mode
    Serial.print(F("Main mode, byte 9<3:0>: 0x"));
    Serial.print(bytes[9] & 0x0F, HEX);
    Serial.print(F("\t\t"));

    switch (bytes[9] & 0x0F) {
      case 0x00:
        Serial.println(F("MODE AUTO"));
        break;
      case 0x01:
        Serial.println(F("MODE COOL"));
        break;
      case 0x02:
        Serial.println(F("MODE DRY"));
        break;
      case 0x03:
        Serial.println(F("MODE FAN"));
        break;
      case 0x04:
        Serial.println(F("MODE HEAT"));
        break;
      case 0x08:
        Serial.println(F("MODE COIL DRY"));
        break;
      case 0x0B:
        Serial.println(F("MODE 10C HEAT"));
        break;
      default:
        Serial.println(F("MODE UNKNOWN"));
        break;
    }


    // Byte 10 encodes swing mode and fan speed

    Serial.print(F("Byte 10: 0x"));
    Serial.println(bytes[10], HEX);

    // Swing mode
    // This may actually be two individual bits
    // i.e. byte 10<4> 1 = oscillate horizontal vane 0 = horizontal vane still
    // byte 10<5> 1 = oscillate vertical vane 0 = vertical vane still
    // byte 10(7:6> unknown
    // For now it is treated as a 4-bit field with four known values
    Serial.print(F("Swing mode, byte 10<7:4>: 0x"));
    Serial.print(bytes[10] >> 4, HEX);
    Serial.print(F("\t\t"));

    switch (bytes[10] >> 4) {
      case 0x00:
        Serial.println(F("OSCILLATION OFF/VANES STILL"));
        break;
      case 0x01:
        Serial.println(F("OSCILLATE HORIZONTAL VANE UP/DOWN"));
        break;
      case 0x02:
        Serial.println(F("OSCILLATE VERTICAL VANE LEFT/RIGHT"));
        break;
      case 0x03:
        Serial.println(F("OSCILLATE HORIZONTAL AND VERTICAL VANES"));
        break;
      default:
        Serial.println(F("VANE UNKNOWN"));
        break;
    }

    // Fan speed
    Serial.print(F("Fan speed, byte 9<10:0>: 0x"));
    Serial.print(bytes[10] & 0x0F, HEX);
    Serial.print(F("\t\t"));

    switch (bytes[10] & 0x07) {
      case 0x00:
        Serial.println(F("FAN AUTO"));
        break;
      case 0x01:
        Serial.println(F("FAN HIGH"));
        break;
      case 0x02:
        Serial.println(F("FAN MED"));
        break;
      case 0x03:
        Serial.println(F("FAN LOW"));
        break;
      case 0x04:
        Serial.println(F("FAN QUIET"));
        break;
      default:
        Serial.println(F("FAN UNKNOWN"));
        break;
    }


    // Bytes 11, 12 and 13 encode the timer operation
    
    Serial.print(F("Byte 11: 0x"));
    Serial.println(bytes[11], HEX);

    Serial.print(F("Byte 12: 0x"));
    Serial.println(bytes[12], HEX);

    Serial.print(F("Byte 13: 0x"));
    Serial.println(bytes[13], HEX);


    Serial.print(F("Timer off, byte 12<2:0> 0x"));
    Serial.print(bytes[12] & 0x03, HEX);
    
    Serial.print(F(" byte 11<7:0>: 0x"));
    Serial.print(bytes[11], HEX);
    Serial.print(F(" 0x"));
    Serial.print(bytes[12] & 0x03, HEX);
    Serial.println(bytes[11], HEX);

    unsigned int timer_off = (unsigned int) (bytes[12] & 0x03);
    timer_off = (timer_off << 8) + bytes[11];
    Serial.print(F("Off time: 0x"));
    Serial.print(timer_off, HEX);
    Serial.print(F("\t\t\t\t"));
    Serial.print((int)timer_off/60);
    Serial.print(F("h "));
    Serial.print(timer_off%60);
    Serial.print(F("m ("));
    Serial.print(timer_off);
    Serial.println(F(" minutes)"));


    Serial.print(F("Timer off value flag, byte 12<3>: 0x"));
    Serial.print(bytes[12] & 0x08 >> 3,HEX);
    
    if ((bytes[12] & 0x08) == 0x08) {
      Serial.println(F("\tNew timer OFF value."));
    } else {
      Serial.println(F("\tTimer OFF value unchanged."));
    }


    Serial.print(F("Timer on, byte 13<6:0> 0x"));
    Serial.print(bytes[13] & 0x3F, HEX);
    Serial.print(F(" byte 12<7:4>: 0x"));
    Serial.print(bytes[12]>>4, HEX);
    Serial.print(F(" 0x"));
    Serial.print(bytes[13] & 0x3F << 4);
    Serial.println(bytes[12] >> 4, HEX);

    unsigned int timer_on = (unsigned int) (bytes[13] & 0x3F);
    timer_on = (timer_on << 4) + (bytes[12] >> 4);
    Serial.print(F("On time: 0x"));
    Serial.print(timer_on, HEX);
    Serial.print(F("\t\t\t\t"));
    Serial.print((int)timer_on/60);
    Serial.print(F("h "));
    Serial.print(timer_on%60);
    Serial.print(F("m ("));
    Serial.print(timer_on);
    Serial.println(F(" minutes)"));

    
    Serial.print(F("Timer on value flag, byte 13<7>: 0x"));
    Serial.print(bytes[13] & 0x80 >> 7,HEX);
    
    if ((bytes[13] & 0x80) == 0x80) {
      Serial.println(F("\tNew timer ON value."));
    } else {
      Serial.println(F("\tTimer ON value unchanged."));
    }


    // Byte 14 is constant
    // Might be a bit for ECO mode on certain models.
    Serial.print(F("Byte 14: 0x"));
    Serial.print(bytes[14], HEX);
    Serial.print(F(" expected 0x20 "));
    if (bytes[14] == 0x20) {
      Serial.println(F("[OK]"));
    } else {
      Serial.println(F("[Error]"));
    }


    // Byte 15 is the checksum

    Serial.print(F("Byte 15 (Checksum): 0x"));
    Serial.println(bytes[15], HEX);

    return true;

  }

  return false;

}
