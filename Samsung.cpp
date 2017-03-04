#include <Arduino.h>

// Samsung with remote ARH-465

bool decodeSamsung(byte *bytes, int byteCount)
{
  // If this looks like a Samsung code...
  if (bytes[0] == 0x02
      && ((byteCount == 21 && bytes[1] == 0xB2) || (byteCount == 14 && bytes[1] == 0x92))
      && bytes[2] == 0x0F) {
    Serial.println(F("Looks like a Samsung protocol"));

    // Power mode
    if (byteCount == 21)
    {
      Serial.println(F("POWER OFF"));
      return true;
    }
    Serial.println(F("POWER ON"));

    // Operating mode
    switch (bytes[12] & 0xF0) {
      case 0x00:
        Serial.println(F("MODE AUTO"));
        break;
      case 0x10:
        Serial.println(F("MODE COOL"));
        break;
      case 0x20:
        Serial.println(F("MODE DRY"));
        break;
      case 0x30:
        Serial.println(F("MODE FAN"));
        break;
      case 0x40:
        Serial.println(F("MODE HEAT"));
        break;
    }

    // Temperature
    Serial.print(F("Temperature: "));
    Serial.println((bytes[11] >> 4) + 16);

    // Fan speed
    switch (bytes[12] & 0x0F) {
      case 0x01:
        Serial.println(F("FAN: AUTO"));
        break;
      case 0x05:
        Serial.println(F("FAN: 1"));
        break;
      case 0x09:
        Serial.println(F("FAN: 2"));
        break;
      case 0x0B:
        Serial.println(F("FAN: 3"));
        break;
      case 0x0F:
        Serial.println(F("FAN: 4"));
        break;
    }

    // Turbo mode
    Serial.print(F("Turbo mode: "));
    switch (bytes[9] & 0xF0) {
      case 0xA0:
        Serial.println(F("ON"));
        break;
      case 0xF0:
        Serial.println(F("OFF"));
        break;
    }   

    // Check if the checksum matches
    byte originalChecksum = bytes[8];
    bytes[8] = 0;
    byte checksum = 0x00;

    // Calculate the byte 8 checksum
    // Count the number of ONE bits on message bytes 7-14
    for (uint8_t j=7; j<15; j++) {
      uint8_t samsungByte = bytes[j];
      for (uint8_t i=0; i<8; i++) {
        if ( (samsungByte & 0x01) == 0x01 ) {
          checksum++;
        }
        samsungByte >>= 1;
      }
    }

    checksum = 34 - checksum;
    checksum <<= 4;
    checksum |= 0x02;

    Serial.print(F("Checksum '0x"));
    Serial.print(checksum, HEX);

    if ( originalChecksum == checksum ) {
      Serial.println(F("' matches"));
    } else {
      Serial.print(F("' does not match 0x"));
      Serial.println(originalChecksum, HEX);
    }
    return true;
  }

  return false;
}
