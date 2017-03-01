#include <Arduino.h>

static byte header[] = {0x14,0x63,0x00,0x10,0x10};

bool decodeFujitsu(byte *bytes, int byteCount)
{
  if(byteCount > sizeof(header) && memcmp(bytes, header, sizeof(header)) == 0) {
    Serial.println(F("Looks like a Fujitsu protocol"));

    byte checksum = 0;
   
    //Calculate checksum
    if (byteCount == 7) {
        checksum = 0xFF - bytes[5];
    } else {
        for (int i=7; i<byteCount-1; i++) {
          checksum -= bytes[i];
        }
    }

    if (checksum == bytes[byteCount-1]) {
      Serial.println(F("Checksum matches"));
    } else {
      Serial.print(F("Checksum does not match: "));
      Serial.print(checksum, HEX);
      Serial.print(F(" vs "));
      Serial.println(bytes[byteCount-1], HEX);
    }

    // Power mode? Main command?
    switch (bytes[5]) {
      case 0x02:
        Serial.println(F("POWER OFF"));
        break;
      case 0xFE:
        Serial.println(F("POWER ON"));
        break;
      case 0x6C:
        Serial.println(F("SWING SET"));
        break;
      default:
        Serial.println(F("POWER unknown"));
        break;
    }

    // Temperature
    Serial.print(F("Temperature: "));
    Serial.println((bytes[8] >> 4) + 16);

    // Operating mode
    switch (bytes[9] & 0xFF) {
      case 0x00:
        Serial.println(F("MODE AUTO"));
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
      case 0x04:
        Serial.println(F("MODE HEAT"));
        break;
      case 0x08:
        Serial.println(F("MODE COIL DRY"));
        break;
      case 0x0B:
        Serial.println(F("MODE 10c HEAT"));
        break;
      case 0x10:
        Serial.println(F("MODE SLEEP"));
        break;
      case 0x20:
        Serial.println(F("SET TIMER (OFF)"));
        break;
      case 0x30:
        Serial.println(F("SET TIMER (ON)"));
        break;
      case 0x40:
        Serial.println(F("SET TIMER (OFF -> ON)"));
        break;
      default:
        Serial.println(F("MODE unknown"));
        break;
    }

    // Fan speed
    switch (bytes[10] & 0x07) {
      case 0x00:
        Serial.println(F("FAN AUTO"));
        break;
      case 0x01:
        Serial.println(F("FAN 1"));
        break;
      case 0x02:
        Serial.println(F("FAN 2"));
        break;
      case 0x03:
        Serial.println(F("FAN 3"));
        break;
      case 0x04:
        Serial.println(F("FAN 4"));
        break;
      default:
        Serial.println(F("FAN unknown"));
        break;
    }

    // Fan swing
    switch (bytes[10] & 0xF0) {
      case 0x10:
        Serial.println(F("VANE: SWING"));
        break;
      case 0x00:
        Serial.println(F("VANE: STILL"));
        break;
      default:
        Serial.println(F("VANE: unknown"));
        break;
    }

  /*
   * Not sure what's going on here, but incrementing time on the remote by 5
   * minutes, increments this value by 5 as well.
  */
  int time = (int)bytes[12] << 8 + bytes[11];
  Serial.print("Time: ");
  Serial.println(time);
   
  //Economy mode? I'm guessing more modes are encoded here on fancier models.
  switch (bytes[14]) {
      case 0x00:
        Serial.println(F("ECONOMY: ON"));
        break;
      case 0x20:
        Serial.println(F("ECONOMY: OFF"));
        break;
      default:
        Serial.println(F("ECONOMY: unknown"));
        break;
    }

    return true;
  }

  return false;
}
