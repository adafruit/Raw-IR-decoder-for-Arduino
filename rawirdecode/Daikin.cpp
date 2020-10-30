#include <Arduino.h>

bool decodeDaikin(byte *bytes, int byteCount)
{
  // If this looks like a Daikin code...
  if ( byteCount == 35 && bytes[0] == 0x11 && bytes[1] == 0xDA && bytes[2] == 0x27 ) {
    Serial.println(F("Looks like a Daikin protocol"));

    // Power mode
    switch (bytes[21] & 0x01) {
      case 0x00:
        Serial.println(F("POWER OFF"));
        break;
      case 0x01:
        Serial.println(F("POWER ON"));
        break;
    }

    // Operating mode
    switch (bytes[21] & 0x70) {
      case 0x00:
        Serial.println(F("MODE AUTO"));
        break;
      case 0x40:
        Serial.println(F("MODE HEAT"));
        break;
      case 0x30:
        Serial.println(F("MODE COOL"));
        break;
      case 0x20:
        Serial.println(F("MODE DRY"));
        break;
      case 0x60:
        Serial.println(F("MODE FAN"));
        break;
      }

    // Temperature
    Serial.print(F("Temperature: "));
    Serial.println(bytes[22] / 2);

    // Fan speed
    switch (bytes[24] & 0xF0) {
      case 0xB0:
        // Outdoor unit quiet
        Serial.println(F("FAN: QUIET"));
        break;
      case 0xA0:
        Serial.println(F("FAN: AUTO"));
        break;
      case 0x30:
        Serial.println(F("FAN: 1"));
        break;
      case 0x40:
        Serial.println(F("FAN: 2"));
        break;
      case 0x50:
        Serial.println(F("FAN: 3"));
        break;
      case 0x60:
        Serial.println(F("FAN: 4"));
        break;
      case 0x70:
        Serial.println(F("FAN: 5"));
        break;
    }

    // Other flags
    Serial.print(F("FLAGS: "));
    if (bytes[29] & 0x01) Serial.print(F("POWERFUL "));
    if (bytes[32] & 0x04) Serial.print(F("ECONO "));
    if (bytes[24] & 0x0F) Serial.print(F("SWING "));
    if (bytes[29] & 0x20) Serial.print(F("QUIET"));
    Serial.println();

    // Check if the checksum matches
    byte checksum = 0x00;

    for (byte i = 0; i < 7; i++) {
      checksum += bytes[i];
    }

    if ( bytes[7] == checksum ) {
      Serial.println(F("Checksum 1 matches"));
    } else {
      Serial.println(F("Checksum 1 does not match"));
    }

    checksum = 0x00;

    for (byte i = 8; i < 15; i++) {
      checksum += bytes[i];
    }

    if ( bytes[15] == checksum ) {
      Serial.println(F("Checksum 2 matches"));
    } else {
      Serial.println(F("Checksum 2 does not match"));
    }

    checksum = 0x00;

    for (byte i = 16; i < 34; i++) {
      checksum += bytes[i];
    }

    if ( bytes[34] == checksum ) {
      Serial.println(F("Checksum 3 matches"));
    } else {
      Serial.println(F("Checksum 3 does not match"));
    }
    return true;
  }

  return false;
}
