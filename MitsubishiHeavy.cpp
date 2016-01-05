#include <Arduino.h>

bool decodeMitsubishiHeavy(byte *bytes, int byteCount)
{
  // If this looks like a Mitsubishi Heavy...
  if ( byteCount == 11 && bytes[0] == 0x52 && bytes[1] == 0xAE && bytes[2] == 0xC3 && bytes[3] == 0x26) {
    Serial.println(F("Looks like a Mitsubishi Heavy protocol"));

     // Power mode
    switch (bytes[9] & 0x08) {
      case 0x00:
        Serial.println(F("POWER ON"));
        break;
      case 0x08:
        Serial.println(F("POWER OFF"));
        break;
    }

    // Operating mode
    switch (bytes[9] & 0x07) {
      case 0x07:
        Serial.println(F("MODE AUTO"));
        break;
      case 0x03:
        Serial.println(F("MODE HEAT"));
        break;
      case 0x06:
        Serial.println(F("MODE COOL"));
        break;
      case 0x05:
        Serial.println(F("MODE DRY"));
        break;
      case 0x04:
        Serial.println(F("MODE FAN"));
        break;
    }

    // Temperature
    Serial.print(F("Temperature: "));
    Serial.println((~((bytes[9] & 0xF0) >> 4) & 0x0F) + 17);

    // Fan speed
    switch (bytes[7] & 0xE0) {
      case 0xE0:
        Serial.println(F("FAN AUTO"));
        break;
      case 0xA0:
        Serial.println(F("FAN 1"));
        break;
      case 0x80:
        Serial.println(F("FAN 2"));
        break;
      case 0x60:
        Serial.println(F("FAN 3"));
        break;
      case 0x20:
        Serial.println(F("FAN HIGH SPEED"));
        break;
      case 0x00:
        Serial.println(F("FAN SILENT"));
        break;
    }

    return true;
  }

  return false;
}
