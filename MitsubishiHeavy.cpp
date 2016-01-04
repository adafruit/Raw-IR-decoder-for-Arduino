#include <Arduino.h>

bool decodeMitsubishiHeavy(byte *bytes, int byteCount)
{
  // If this looks like a Mitsubishi Heavy...
  if ( byteCount == 11 && bytes[0] == 0x52 && bytes[1] == 0xAE && bytes[2] == 0xC3 && bytes[3] == 0x26) {
    Serial.println("Looks like a Mitsubishi Heavy protocol");

     // Power mode
    switch (bytes[9] & 0x08) {
      case 0x00:
        Serial.println("POWER ON");
        break;
      case 0x08:
        Serial.println("POWER OFF");
        break;
    }

    // Operating mode
    switch (bytes[9] & 0x07) {
      case 0x07:
        Serial.println("MODE AUTO");
        break;
      case 0x03:
        Serial.println("MODE HEAT");
        break;
      case 0x06:
        Serial.println("MODE COOL");
        break;
      case 0x05:
        Serial.println("MODE DRY");
        break;
      case 0x01:
        Serial.println("MODE FAN");
        break;
    }

    // Temperature
    Serial.print("Temperature: ");
    Serial.println((~((bytes[9] & 0xF0) >> 4) & 0x0F) + 17);

    // Fan speed
    switch (bytes[7] & 0xE0) {
      case 0xE0:
        Serial.println("FAN AUTO");
        break;
      case 0xA0:
        Serial.println("FAN 1");
        break;
      case 0x80:
        Serial.println("FAN 2");
        break;
      case 0x60:
        Serial.println("FAN 3");
        break;
      case 0x20:
        Serial.println("FAN HIGH SPEED");
        break;
      case 0x00:
        Serial.println("FAN SILENT");
        break;
    }

    return true;
  }

  return false;
}
