#include <Arduino.h>

bool decodeHyundai(byte *bytes, int pulseCount)
{
  // If this looks like a Hyundai code...
  // Remote control Y512F2, Hyundai split-unit air conditioner
  if ( pulseCount == 38 ) {
    Serial.println(F("Looks like a Hyundai protocol"));

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
