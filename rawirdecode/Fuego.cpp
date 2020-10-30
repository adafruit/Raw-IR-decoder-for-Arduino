#include <Arduino.h>

bool decodeFuego(byte *bytes, int byteCount)
{
  // If this looks like a Chinese Fuego, Vivax, Classe, NEO, Galanz, Simbio, Beko code...
  // Remote control GZ-1002B-E3
  if ( byteCount == 14 &&
       bytes[0] == 0x23 &&
       bytes[1] == 0xCB &&
       bytes[2] == 0x26 ) {
    Serial.println(F("Looks like a Fuego etc. protocol"));

    // Check if the checksum matches
    byte checksum = 0;

    for (int i=0; i<13; i++) {
      checksum += bytes[i];
    }

    if (checksum == bytes[13]) {
      Serial.println(F("Checksum matches"));
    } else {
      Serial.println(F("Checksum does not match"));
    }

    // Power mode
    switch (bytes[5] & 0x04) {
      case 0x00:
        Serial.println(F("POWER OFF"));
        break;
      case 0x04:
        Serial.println(F("POWER ON"));
        break;
    }

    // Operating mode
    switch (bytes[6] & 0x07) {
      case 0x00: //???
        Serial.println(F("MODE AUTO"));
        break;
      case 0x01:
        Serial.println(F("MODE HEAT"));
        break;
      case 0x03:
        Serial.println(F("MODE COOL"));
        break;
      case 0x02:
        Serial.println(F("MODE DRY"));
        break;
      case 0x07:
        Serial.println(F("MODE FAN"));
        break;
      }

    // Temperature
    Serial.print(F("Temperature: "));
    Serial.println(31 - (bytes[7] & 0x0F));

    // Fan speed
    switch (bytes[8] & 0x07) {
      case 0x00:
        Serial.println(F("FAN: AUTO"));
        break;
      case 0x02:
        Serial.println(F("FAN: 1"));
        break;
      case 0x03:
        Serial.println(F("FAN: 2"));
        break;
      case 0x05:
        Serial.println(F("FAN: 3"));
        break;
    }

    // Vertical air direction
    Serial.print(F("Vertical air direction: "));
    switch (bytes[8] & 0x38) {
      case 0x00:
        Serial.println(F("AUTO"));
        break;
      case 0x08:
        Serial.println(F("UP"));
        break;
      case 0x10:
        Serial.println(F("MIDDLE UP"));
        break;
      case 0x18:
        Serial.println(F("MIDDLE"));
        break;
      case 0x20:
        Serial.println(F("MIDDLE DOWN"));
        break;
      case 0x28:
        Serial.println(F("DOWN"));
        break;
      case 0x38:
        Serial.println(F("SWING"));
        break;
    }

    return true;
  }

  return false;
}
