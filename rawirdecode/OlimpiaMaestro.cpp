#include <Arduino.h>

// from Carrier.cpp
#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte)  \
  (byte & 0x80 ? 1 : 0), \
  (byte & 0x40 ? 1 : 0), \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0)

bool decodeOlimpiaMaestro(byte *bytes, int byteCount)
{
  // If this looks like an Olimpia code...
  if (byteCount == 11 && bytes[0] == 0x5A) {
    Serial.println(F("Looks like an Olimpia-Splendid Maestro protocol"));

    // Determine if the unit is off or in a specific mode (bits 8–10)
    switch ((bytes[1] & 0x07)) {
      case 0b001:
        Serial.println(F("POWER ON"));
        Serial.println(F("MODE COOL"));
        break;
      case 0b010:
        Serial.println(F("POWER ON"));
        Serial.println(F("MODE HEAT"));
        break;
      case 0b011:
        Serial.println(F("POWER ON"));
        Serial.println(F("MODE DRY"));
        break;
      case 0b100:
        Serial.println(F("POWER ON"));
        Serial.println(F("MODE FAN"));
        break;
      case 0b101:
        Serial.println(F("POWER ON"));
        Serial.println(F("MODE AUTO"));
        break;
      case 0b000:
        Serial.println(F("POWER OFF"));
        break;
      default:
        Serial.println(F("MODE UNKNOWN"));
    }

    // Temperature (bits 72–76)
    Serial.print(F("Temperature: "));
    if (bytes[9] & 0x20) {
      uint8_t encodedTemp = ((bytes[9] & 0x1F));
      Serial.print(((((float) encodedTemp) / 2) + 15) * (1.8) + 32);
      Serial.println(F("°F"));
    } else {
      uint8_t encodedTemp = ((bytes[9] & 0x1F));
      Serial.print((((float) encodedTemp) / 2) + 15);
      Serial.println(F("°C"));
    }

    // Fan speed
    switch ((bytes[1] & 0x18) >> 3) {
      case 0b00:
        Serial.println(F("FAN: LOW"));
        break;
      case 0b01:
        Serial.println(F("FAN: MEDIUM"));
        break;
      case 0b10:
        Serial.println(F("FAN: HIGH"));
        break;
      case 0b11:
        Serial.println(F("FAN: AUTO"));
        break;
    }

    // Other flags
    Serial.print(F("FLAGS: "));
    if (bytes[1] & 0x04) Serial.print(F("FLAP SWING "));
    if (bytes[7] & 0x04) Serial.print(F("ECONO "));
    if (bytes[1] & 0x02) Serial.print(F("LOW NOISE "));
    Serial.println();

    // Check if the checksum matches
    byte checksum = 0x00;

    for (int x = 0; x < 10; x++) {
      checksum += bytes[x];
    }

    if ( bytes[10] == checksum ) {
      Serial.println(F("Checksum matches"));
    } else {
      Serial.print(F("Checksum does not match.\nCalculated | Received\n  "));
      Serial.printf(BYTETOBINARYPATTERN, BYTETOBINARY(checksum));
      Serial.print(F(" | "));
      Serial.printf(BYTETOBINARYPATTERN, BYTETOBINARY(bytes[10]));
      Serial.println();
    }

    return true;
  }

  return false;
}
