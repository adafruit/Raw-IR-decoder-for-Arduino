#include <Arduino.h>

bool decodeMitsubishiElectric(byte *bytes, int byteCount)
{
  // If this looks like a Mitsubishi FD-25 or FE code...
  if ( byteCount == 36 && bytes[0] == 0x23 &&
       ( (memcmp(bytes, bytes+18, 17) == 0) ||
         ((memcmp(bytes, bytes+18, 14) == 0) && bytes[32] == 0x24) ) ){
    Serial.println(F("Looks like a Mitsubishi FD / FE / MSY series protocol"));

    // Check if the checksum matches
    byte checksum = 0;

    for (int i=0; i<17; i++) {
      checksum += bytes[i];
    }

    if (checksum == bytes[17]) {
      Serial.println(F("Checksum matches"));
    } else {
      Serial.println(F("Checksum does not match"));
    }

    // Power mode
    switch (bytes[5]) {
      case 0x00:
        Serial.println(F("POWER OFF"));
        break;
      case 0x20:
        Serial.println(F("POWER ON"));
        break;
      default:
        Serial.println(F("POWER unknown"));
        break;
    }

    // Operating mode
    Serial.print(F("MODE "));
    switch (bytes[6] & 0x38) { // 0b00111000
      case 0x38:
        Serial.println(F("FAN"));
        break;
      case 0x20:
        Serial.println(F("AUTO"));
        break;
      case 0x08:
        if (bytes[15] == 0x20) {
          Serial.println(F("MAINTENANCE HEAT (FE only)"));
        } else {
          Serial.println(F("HEAT"));
        }
        break;
      case 0x18:
        Serial.println(F("COOL"));
        break;
      case 0x10:
        Serial.println(F("DRY"));
        break;
      default:
        Serial.println(F("unknown"));
        break;
    }

    // I-See
    Serial.print(F("I-See: "));
    switch (bytes[6] & 0x40) { // 0b01000000
      case 0x40:
        Serial.println(F("ON"));
        break;
      case 0x00:
        Serial.println(F("OFF"));
        break;
    }

    // Clean
    Serial.print(F("Clean: "));
    switch (bytes[14] & 0x04) { // 0b00000100
      case 0x04:
        Serial.println(F("ON"));
        break;
      case 0x00:
        Serial.println(F("OFF"));
        break;
    }


    // Plasma
    Serial.print(F("Plasma: "));
    switch (bytes[15] & 0x40) { // 0b01000000
      case 0x40:
        Serial.println(F("ON"));
        break;
      case 0x00:
        Serial.println(F("OFF"));
        break;
    }

    // Temperature
    Serial.print(F("Temperature: "));
    if (bytes[7] == 0x00) {
      Serial.println(F("10"));
    } else {
      Serial.println(bytes[7] + 16);
    }

    // Fan speed
    Serial.print(F("FAN "));
    switch (bytes[9] & 0x07) { // 0b00000111
      case 0x00:
        Serial.println(F("AUTO"));
        break;
      case 0x01:
        Serial.println(F("1"));
        break;
      case 0x02:
        Serial.println(F("2"));
        break;
      case 0x03:
        Serial.println(F("3"));
        break;
      case 0x04:
        Serial.println(F("4"));
        break;
      default:
        Serial.println(F("unknown"));
        break;
    }

    // Vertical air direction
    Serial.print(F("VANE: "));
    switch (bytes[9] & 0xF8) { // 0b11111000
      case 0x40: // 0b01000
        Serial.println(F("AUTO1?"));
        break;
      case 0x48: // 0b01001
        Serial.println(F("UP"));
        break;
      case 0x50: // 0b01010
        Serial.println(F("UP-1"));
        break;
      case 0x58: // 0b01011
        Serial.println(F("UP-2"));
        break;
      case 0x60: // 0b01100
        Serial.println(F("UP-3"));
        break;
      case 0x68: // 0b01101
        Serial.println(F("DOWN"));
        break;
      case 0x78: // 0b01111
        Serial.println(F("SWING"));
        break;
      case 0x80: // 0b10000
        Serial.println(F("AUTO2?"));
        break;
      case 0xB8: // 0b10111
        Serial.println(F("AUTO3?"));
        break;
      default:
        Serial.println(F("unknown"));
        break;
    }

    // Horizontal air direction
    Serial.print(F("WIDE VANE: "));
    switch (bytes[8] & 0xF0) { // 0b11110000
      case 0x00:
        Serial.println(F("AREA"));
        break;
      case 0x10:
        Serial.println(F("LEFT"));
        break;
      case 0x20:
        Serial.println(F("MIDDLE LEFT"));
        break;
      case 0x30:
        Serial.println(F("MIDDLE"));
        break;
      case 0x40:
        Serial.println(F("MIDDLE RIGHT"));
        break;
      case 0x50:
        Serial.println(F("RIGHT"));
        break;
      case 0xC0:
        Serial.println(F("SWING"));
        break;
      default:
        Serial.println(F("unknown"));
        break;
    }

    // Horizontal air direction, area mode
    Serial.print(F("AREA MODE: "));
    switch (bytes[13] & 0xC0) { // 0b11000000
      case 0x00:
        Serial.println(F("OFF"));
        break;
      case 0x40:
        Serial.println(F("LEFT"));
        break;
      case 0x80:
        Serial.println(F("AUTO"));
        break;
      case 0xC0:
        Serial.println(F("RIGHT"));
        break;
    }

    // Installation position
    Serial.print(F("INSTALL POSITION: "));
    switch (bytes[14] & 0x18) { // 0b00011000
      case 0x08:
        Serial.println(F("LEFT"));
        break;
      case 0x10:
        Serial.println(F("CENTER"));
        break;
      case 0x18:
        Serial.println(F("RIGHT"));
        break;
      default:
        Serial.println(F("unknown"));
        break;
    }

    
    return true;
  }

  return false;
}
