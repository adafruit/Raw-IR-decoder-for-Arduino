#include <Arduino.h>

byte bitReverse(byte x);

// Toshiba RAS-10PKVP-ND

bool decodeToshiba(byte *bytes, int byteCount)
{
  // If this looks like a Toshiba code...
  if ( byteCount == 9 && bytes[0] == 0x4F && bytes[1] == 0xB0 && bytes[2] == 0xC0 && bytes[3] == 0x3F && bytes[4] == 0x80 ) {
    Serial.println(F("Looks like a Toshiba protocol"));

    // Power & operating mode
    switch (bytes[6] & 0xF0) {
      case 0x00:
        Serial.println(F("MODE AUTO"));
        break;
      case 0x80:
        Serial.println(F("MODE COOL"));
        break;
      case 0x40:
        Serial.println(F("MODE DRY"));
        break;
      case 0xC0:
        Serial.println(F("MODE HEAT"));
        break;
      case 0xE0:
        Serial.println(F("POWER OFF"));
        break;
    }

    // Temperature
    Serial.print(F("Temperature: "));
    Serial.println((bitReverse((bytes[5] & 0x07)) >> 4) + 17);

    // Fan speed
    switch (bytes[6] & 0x07) {
      case 0x00:
        Serial.println(F("FAN: AUTO"));
        break;
      case 0x02:
        Serial.println(F("FAN: 1"));
        break;
      case 0x06:
        Serial.println(F("FAN: 2"));
        break;
      case 0x01:
        Serial.println(F("FAN: 3"));
        break;
      case 0x05:
        Serial.println(F("FAN: 4"));
        break;
      case 0x03:
        Serial.println(F("FAN: 5"));
        break;
    }

    // Check if the checksum matches
    byte checksum = 0x00;

    for (byte i = 0; i < 8; i++) {
      checksum ^= bytes[i];
    }

    Serial.print(F("Checksum '0x"));
    Serial.print(checksum, HEX);

    if ( bytes[8] == checksum ) {
      Serial.println(F("' matches"));
    } else {
      Serial.print(F(" does not match "));
      Serial.println(bytes[8], HEX);
    }
    return true;
  }

  return false;
}
