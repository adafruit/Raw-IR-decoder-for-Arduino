#include <Arduino.h>

bool decodeBallu(byte *bytes, int byteCount)
{
  // If this looks like a Ballu code...
  if ( byteCount == 21 && bytes[0] == 0x83 && bytes[1] == 0x06 ) {
    Serial.println(F("Looks like a Ballu protocol"));

    // Power mode
    // TBD

    // Operating mode
    switch (bytes[3] & 0x07) {
      case 0x00:
        Serial.println(F("MODE HEAT"));
        break;
      case 0x02:
        Serial.println(F("MODE COOL"));
        break;
      case 0x03:
        Serial.println(F("MODE DRY"));
        break;
      case 0x04:
        Serial.println(F("MODE FAN"));
        break;
      }

    // Temperature
    Serial.print(F("Temperature: "));
    Serial.println((bytes[3] >> 4) + 16);

    // Fan speed
    switch (bytes[02] & 0x03) {
      case 0xA0:
        Serial.println(F("FAN: AUTO"));
        break;
      case 0x01:
        Serial.println(F("FAN: 1"));
        break;
      case 0x02:
        Serial.println(F("FAN: 2"));
        break;
      case 0x03:
        Serial.println(F("FAN: 3"));
        break;
    }

    // Check if the checksum matches
    // TBD

    return true;
  }

  return false;
}
