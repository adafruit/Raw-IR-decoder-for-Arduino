#include <Arduino.h>

bool decodeDaikin(byte *bytes, int byteCount)
{
  // If this looks like a Daikin code...
  if ( byteCount == 35 && bytes[0] == 0x11 && bytes[1] == 0xDA && bytes[2] == 0x27 ) {
    Serial.println("Looks like a Daikin protocol");

    // Power mode
    switch (bytes[21] & 0x01) {
      case 0x00:
        Serial.println("POWER OFF");
        break;
      case 0x01:
        Serial.println("POWER ON");
        break;
    }

    // Operating mode
    switch (bytes[21] & 0x70) {
      case 0x00:
        Serial.println("MODE AUTO");
        break;
      case 0x40:
        Serial.println("MODE HEAT");
        break;
      case 0x30:
        Serial.println("MODE COOL");
        break;
      case 0x20:
        Serial.println("MODE DRY");
        break;
      case 0x60:
        Serial.println("MODE FAN");
        break;
      }

    // Temperature
    Serial.print("Temperature: ");
    Serial.println(bytes[22] / 2);

    // Fan speed
    switch (bytes[24] & 0xF0) {
      case 0xA0:
        Serial.println("FAN: AUTO");
        break;
      case 0x30:
        Serial.println("FAN: 1");
        break;
      case 0x40:
        Serial.println("FAN: 2");
        break;
      case 0x50:
        Serial.println("FAN: 3");
        break;
      case 0x60:
        Serial.println("FAN: 4");
        break;
      case 0x70:
        Serial.println("FAN: 5");
        break;
    }

    // Check if the checksum matches
    byte checksum = 0x00;

    for (byte i = 0; i < 7; i++) {
      checksum += bytes[i];
    }

    if ( bytes[7] == checksum ) {
      Serial.println("Checksum 1 matches");
    } else {
      Serial.println("Checksum 1 does not match");
    }

    checksum = 0x00;

    for (byte i = 8; i < 15; i++) {
      checksum += bytes[i];
    }

    if ( bytes[15] == checksum ) {
      Serial.println("Checksum 2 matches");
    } else {
      Serial.println("Checksum 2 does not match");
    }

    checksum = 0x00;

    for (byte i = 16; i < 34; i++) {
      checksum += bytes[i];
    }

    if ( bytes[34] == checksum ) {
      Serial.println("Checksum 3 matches");
    } else {
      Serial.println("Checksum 3 does not match");
    }
    return true;
  }

  return false;
}
