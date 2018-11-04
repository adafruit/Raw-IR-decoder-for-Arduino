#include <Arduino.h>

bool decodeGree(byte *bytes, int pulseCount)
{
  // If this looks like a Gree code...
  if ( pulseCount == 71 ) {
    Serial.println(F("Looks like a Gree protocol"));

    // Check if the checksum matches
    uint8_t checksum = (
      (bytes[0] & 0x0F) +
      (bytes[1] & 0x0F) +
      (bytes[2] & 0x0F) +
      (bytes[3] & 0x0F) +
      (bytes[5] & 0xF0) >> 4 +
      (bytes[6] & 0xF0) >> 4 +
      (bytes[7] & 0xF0) >> 4 +
      0x0A) & 0xF0;

    if (checksum == bytes[8]) {
      Serial.println(F("Checksum matches"));
    } else {
      Serial.println(F("Checksum does not match"));
    }

    // Power mode
    switch (bytes[0] & 0x08) {
      case 0x00:
        Serial.println(F("POWER OFF"));
        break;
      case 0x08:
        Serial.println(F("POWER ON"));
        break;
    }

    // Operating mode
    switch (bytes[0] & 0x07) {
      case 0x00:
        Serial.println(F("MODE AUTO"));
        break;
      case 0x04:
        Serial.println(F("MODE HEAT"));
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
      }

    // Temperature
    Serial.print(F("Temperature: "));
    Serial.println((bytes[1] & 0x0F) + 16);

    // Fan speed
    switch (bytes[0] & 0x30) {
      case 0x00:
        Serial.println(F("FAN: AUTO"));
        break;
      case 0x10:
        Serial.println(F("FAN: 1"));
        break;
      case 0x20:
        Serial.println(F("FAN: 2"));
        break;
      case 0x30:
        Serial.println(F("FAN: 3"));
        break;
    }

    // Sleep mode
    switch (bytes[0] & 0x80) {
      case 0x80:
        Serial.println(F("SLEEP: ON"));
        break;
      case 0x00:
        Serial.println(F("SLEEP: OFF"));
        break;
    }

    // Air direction
    switch (bytes[0] & 0x40) {
      case 0x40:
        Serial.println(F("SWING: ON"));
        break;
      case 0x00:
        Serial.println(F("SWING: OFF"));
        break;
    }

    return true;
  }

  return false;
}
