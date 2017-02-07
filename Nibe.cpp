#include <Arduino.h>
bool decodeNibe(char* symbols, int bitCount)
{
  byte bytes[11];
  byte currentBit = 0;
  byte currentByte = 0;
  byte byteCount = 0;

  // If this looks like a Nibe code...
  if ( bitCount == 93 )
  {
    // Decode the string of bits to a byte array
    for (uint16_t i = 5; i < bitCount; i++) {
      if (symbols[i] == '0' || symbols[i] == '1') {
        currentByte >>= 1;
        currentBit++;

        if (symbols[i] == '1') {
          currentByte |= 0x80;
        }

        Serial.print(symbols[i]);

        if (currentBit >= 8) {
          bytes[byteCount++] = currentByte;
          currentBit = 0;
          currentByte = 0;

          Serial.print(" ");
        }
      }
    }

  // Print the byte array
  Serial.print(F("\nNibe bytes: "));
  for (int i = 0; i < byteCount; i++) {
    if (bytes[i] < 0x10) {
      Serial.print(F("0"));
    }
    Serial.print(bytes[i], HEX);
    if ( i < byteCount - 1 ) {
      Serial.print(F(","));
    }
  }
  Serial.println();

  // Power mode
    switch (bytes[9] & 0x08) {
      case 0x00:
        Serial.println(F("POWER OFF"));
        break;
      case 0x08:
        Serial.println(F("POWER ON"));
        break;
    }

    // Operating mode
    switch (bytes[2] & 0x0E) {
      case 0x08:
        Serial.println(F("MODE HEAT"));
        break;
      case 0x04:
        Serial.println(F("MODE COOL"));
        break;
      case 0x02:
        Serial.println(F("MODE DRY"));
        break;
      }

    // Temperature

    byte temperature = ((bytes[2] & 0xF0) >> 4) | ((bytes[3] & 0x01) << 5) - 12;
    Serial.print(F("Temperature: "));
    Serial.println(temperature);

    // Fan speed
    switch (bytes[3] & 0x06) {
      case 0x00:
        Serial.println(F("FAN: AUTO"));
        break;
      case 0x02:
        Serial.println(F("FAN: 1"));
        break;
      case 0x04:
        Serial.println(F("FAN: 2"));
        break;
      case 0x06:
        Serial.println(F("FAN: 3"));
        break;
    }

    // Flap position
    switch (bytes[4] & 0x0F) {
      case 0x00:
        Serial.println(F("FLAP: AUTO"));
        break;
      case 0x01:
        Serial.println(F("FLAP: 1"));
        break;
      case 0x02:
        Serial.println(F("FLAP: 2"));
        break;
      case 0x03:
        Serial.println(F("FLAP: 3"));
        break;
      case 0x04:
        Serial.println(F("FLAP: 4"));
        break;
      case 0x05:
        Serial.println(F("FLAP: 5"));
        break;
      case 0x06:
        Serial.println(F("FLAP: 6"));
        break;
      case 0x07:
        Serial.println(F("FLAP: All"));
        break;
    }

    // Check if the checksum matches
    byte checksum = 0x00;

    for (byte i = 0; i < 10; i++) {
      checksum += bytes[i];
    }

    Serial.print(F("Checksum: "));
    Serial.println(checksum, HEX);
    if ( bytes[10] == checksum ) {
      Serial.println(F("Checksum matches"));
    } else {
      Serial.println(F("Checksum does not match"));
    }

    return true;
  }
  return false;
}




