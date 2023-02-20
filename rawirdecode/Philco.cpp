#include <Arduino.h>

// Philco with remote KT-L12010C

bool decodePhilco(byte *bytes, int byteCount)
{  
  if (byteCount == 21 && bytes[0] == 0x83 && bytes[1] == 0x06) {   
    Serial.println(F("Looks like a Philco protocol"));

    if (bytes[2] & 0x04 && bytes[15] == 0x01) {
      Serial.println(F("POWER ON"));
    }

    {
      Serial.print(F("Current time: "));
      Serial.print(bytes[6] & 0x5F);
      Serial.print(F(":"));
      Serial.println(bytes[7]);
    }

    // Operating mode
    if((bytes[4] & 0xF) == 0x02) {
      switch (bytes[13]) {
        case 0x07:
          Serial.println(F("MODE COOL"));
          break;
        case 0xF1:
          Serial.println(F("MODE DRY"));
          break;
        case 0xF7:
          Serial.println(F("MODE FAN"));
          break;
        case 0xD2:
          Serial.println(F("MODE HEAT"));
          break;
      }
    } else if((bytes[3] & 0xF) == 0x1) {
      Serial.println(F("MODE SMART"));

      if((bytes[2] & 0xF0) == 0x00) {
        Serial.println(F("SMART: --"));
      } else if((bytes[2] & 0xF0) == 0x10 && bytes[16] == 0x00) {
        Serial.println(F("SMART: 1"));
      } else if((bytes[2] & 0xF0) == 0x50 && bytes[16] == 0x00) {
        Serial.println(F("SMART: -1"));
      } else if((bytes[2] & 0xF0) == 0x20) {
        switch (bytes[16]) {
          case 0x00:
            Serial.println(F("SMART: 2"));
            break;
          case 0x04:
            Serial.println(F("SMART: 3"));
            break;
          case 0x08:
            Serial.println(F("SMART: 4"));
            break;
          case 0x0C:
            Serial.println(F("SMART: 5"));
            break;
          case 0x10:
            Serial.println(F("SMART: 6"));
            break;
          case 0x14:
            Serial.println(F("SMART: 7"));
            break;
        }
      } else if((bytes[2] & 0xF0) == 0x60) {
        switch (bytes[16]) {
          case 0x00:
            Serial.println(F("SMART: -2"));
            break;
          case 0x04:
            Serial.println(F("SMART: -3"));
            break;
          case 0x08:
            Serial.println(F("SMART: -4"));
            break;
          case 0x0C:
            Serial.println(F("SMART: -5"));
            break;
          case 0x10:
            Serial.println(F("SMART: -6"));
            break;
          case 0x14:
            Serial.println(F("SMART: -7"));
            break;
        }
      }
    }

    if (bytes[3] == 0x73) {
      Serial.println(F("MODE DRY"));

      if((bytes[2] & 0xF0) == 0x00) {
        Serial.println(F("DRY: --"));
      } else if((bytes[2] & 0xF0) == 0x10 && bytes[16] == 0x00) {
        Serial.println(F("DRY: 1"));
      } else if((bytes[2] & 0xF0) == 0x50 && bytes[16] == 0x00) {
        Serial.println(F("DRY: -1"));
      } else if((bytes[2] & 0xF0) == 0x20) {
        switch (bytes[16]) {
          case 0x00:
            Serial.println(F("DRY: 2"));
            break;
          case 0x04:
            Serial.println(F("DRY: 3"));
            break;
          case 0x08:
            Serial.println(F("DRY: 4"));
            break;
          case 0x0C:
            Serial.println(F("DRY: 5"));
            break;
          case 0x10:
            Serial.println(F("DRY: 6"));
            break;
          case 0x14:
            Serial.println(F("DRY: 7"));
            break;
        }
      } else if((bytes[2] & 0xF0) == 0x60) {
        switch (bytes[16]) {
          case 0x00:
            Serial.println(F("DRY: -2"));
            break;
          case 0x04:
            Serial.println(F("DRY: -3"));
            break;
          case 0x08:
            Serial.println(F("DRY: -4"));
            break;
          case 0x0C:
            Serial.println(F("DRY: -5"));
            break;
          case 0x10:
            Serial.println(F("DRY: -6"));
            break;
          case 0x14:
            Serial.println(F("DRY: -7"));
            break;
        }
      }
    }

    // Temperature target
    if ((bytes[3] & 0xF) == 0x2 || (bytes[3] & 0xF) == 0x0) {
      Serial.print(F("Selected temperature: "));
      Serial.print((bytes[3] >> 4) + 16);
      Serial.println(F("°"));
    }

    // Temperature real
    if(bytes[11] == 0x80) {
      Serial.print(F("Current temperature: "));
      Serial.print((bytes[12] & 0x0F) + 16);
      Serial.println(F("°"));
    }

    // Fan speed
    switch (bytes[2] & 0x03) {
      case 0x00:
        Serial.println(F("FAN: AUTO"));
        break;
      case 0x03:
        Serial.println(F("FAN: 1 (low)"));
        break;
      case 0x02:
        Serial.println(F("FAN: 2 (medium)"));
        break;
      case 0x01:
        Serial.println(F("FAN: 3 (high)"));
        break;
    }

    if (bytes[2] == 0x03 && bytes[14] == 0x04 && bytes[15] == 0x0B) {
      Serial.println(F("QUIET: ON"));
    } else if (bytes[14] == 0x00 && bytes[15] == 0x0B) {
      Serial.println(F("QUIET: OFF)"));
    }

    // Air direction
    if(bytes[8] == 0x80 && bytes[15] == 0x08) {
      Serial.println(F("FIN: left to right"));
    }

    if(bytes[8] == 0x40 && bytes[15] == 0x07) {
      Serial.println(F("FIN: up and down"));
    }

    if((bytes[2] & 0x0F) == 0x0B) {
      // Sleep mode
      switch (bytes[14]) {
        case 0x00:
          Serial.println(F("SLEEP: 1"));
          break;
        case 0x40:
          Serial.println(F("SLEEP: 2"));
          break;
        case 0x80:
          Serial.println(F("SLEEP: 3"));
          break;
        case 0xC0:
          Serial.println(F("SLEEP: 4"));
          break;
      }
    } else {
      Serial.println(F("SLEEP: OFF"));
    }

    // Turbo mode
    Serial.print(F("TURBO: "));
    switch (bytes[5]) {
      case 0x90:
      case 0x10:
        Serial.println(F("ON"));
        break;
      case 0x00:
        Serial.println(F("OFF"));
        break;
    }

    Serial.print(F("ECONOMY: "));
    switch (bytes[14]) {
      case 0x20:
        Serial.println(F("ON"));
        break;
      case 0x00:
        Serial.println(F("OFF"));
        break;
    }

    // Ifeel mode
    Serial.print(F("IFEEL: "));
    switch (bytes[11]) {
      case 0x80:
        Serial.println(F("ON"));
        break;
      default:
        Serial.println(F("OFF"));
        break;
    }

    //Dimmer
    if ((bytes[6] & 0x20)) {
      Serial.println(F("DIMMER: ON"));
    } else {
      Serial.println(F("DIMMER: OFF"));
    }

    // Timer  
    byte hoursOff = bytes[8];
    byte minutesOff = bytes[9] & 0x7F;
    byte hoursOn = bytes[10];
    byte minutesOn = bytes[11] & 0x7F;

    if (bytes[9] & 0x80 && bytes[15] == 0x05) {
      {
        Serial.print(F("Activate TIMER ON at "));
        Serial.print(hoursOn);
        Serial.print(F(":"));
        Serial.println(minutesOn);
      }

      {
        Serial.print(F("TIMER OFF at "));
        Serial.print(hoursOff);
        Serial.print(F(":"));
        Serial.println(minutesOff);
      }
    } else if (bytes[15] == 0x1D) {
      {
        Serial.print(F("Activate TIMER OFF at "));
        Serial.print(hoursOff);
        Serial.print(F(":"));
        Serial.println(minutesOff);
      }

      {
        Serial.print(F("TIMER ON at "));
        Serial.print(hoursOn);
        Serial.print(F(":"));
        Serial.println(minutesOn);
      }
    } else if (bytes[10] == 0x00 && bytes[11] == 0x80 && bytes[15] == 0x05) {
      Serial.println(F("Deactivate TIMER ON"));
    } else if (bytes[8] == 0x00 && bytes[9] == 0x80 && bytes[15] == 0x1D) {
      Serial.println(F("Deactivate TIMER OFF"));
    }

    {
      byte originalChecksum = bytes[13];
      byte calcuatedChecksum = 0x00;

      for (int i = 2; i < 13; i++) {
        calcuatedChecksum ^= bytes[i];
      }

      Serial.print(F("First checksum '0x"));
      Serial.print(calcuatedChecksum, HEX);

      if ( originalChecksum == calcuatedChecksum ) {
        Serial.println(F("' matches"));
      } else {
        Serial.print(F("' does not match 0x"));
        Serial.println(originalChecksum, HEX);
      }
    }

    {
      byte originalChecksum = bytes[20];
      byte calcuatedChecksum = 0x00;

      for (int i = 14; i < 20; i++) {
        calcuatedChecksum ^= bytes[i];
      }

      Serial.print(F("Second checksum '0x"));
      Serial.print(calcuatedChecksum, HEX);

      if ( originalChecksum == calcuatedChecksum ) {
        Serial.println(F("' matches"));
      } else {
        Serial.print(F("' does not match 0x"));
        Serial.println(originalChecksum, HEX);
      }
    }
    return true;
  }
  
  return false;
}
