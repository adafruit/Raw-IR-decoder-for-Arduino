#include <Arduino.h>

// AUX YKR-N/002E

bool decodeAUX(byte *bytes, int byteCount)
{
  // If this looks like a AUX code...
  if ( byteCount == 13 && bytes[0] == 0xC3 ) {
    Serial.println(F("Looks like a AUX protocol"));

    // Power mode
    switch (bytes[9] & 0x20) {
      case 0x00:
        Serial.println(F("POWER OFF"));
        break;
      case 0x20:
        Serial.println(F("POWER ON"));
        break;
    }

    // Operating mode
    switch (bytes[6] & 0xE0) {
      case 0x80:
        Serial.println(F("MODE HEAT"));
        break;
      case 0x20:
        Serial.println(F("MODE COOL"));
        break;
      case 0x40:
        Serial.println(F("MODE DRY"));
        break;
      case 0xC0:
        Serial.println(F("MODE FAN"));
        break;
//       case 0xC0: // TBD
//        Serial.println(F("MODE AUTO"));
//        break;
    }

    // Temperature
    Serial.print(F("Temperature: "));
    Serial.println((bytes[1] >> 3) + 8);

    // Fan speed
    switch (bytes[04] & 0xE0) {
      case 0xA0:
        Serial.println(F("FAN: AUTO"));
        break;
      case 0x60:
        Serial.println(F("FAN: 1"));
        break;
      case 0x40:
        Serial.println(F("FAN: 2"));
        break;
      case 0x20:
        Serial.println(F("FAN: 3"));
        break;
    }

    // Check if the checksum matches
    byte checksum = 0x00;

    for (byte i = 0; i < 12; i++) {
      checksum += bytes[i];
    }

    if ( bytes[12] == checksum ) {
      Serial.println(F("Checksum matches"));
    } else {
      Serial.println(F("Checksum does not match"));
    }

    return true;
  }

  return false;
}
