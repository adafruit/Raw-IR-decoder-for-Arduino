#include <Arduino.h>

bool decodeNibe(byte *originalBytes, char* symbols, int bitCount)
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

    // Night (low night temp I think)
    switch (bytes[9] & 0x01) {
      case 0x00:
        Serial.println(F("NIGHT MODE OFF"));
        break;
      case 0x01:
        Serial.println(F("NIGHT MODE ON"));
        break;
    }

    // Filter
    switch (bytes[9] & 0x04) {
      case 0x00:
        Serial.println(F("FILTER OFF"));
        break;
      case 0x04:
        Serial.println(F("FILTER ON"));
        break;
    }

    // Power mode
    switch (bytes[9] & 0x08) {
      case 0x00:
        Serial.println(F("POWER OFF"));
        break;
      case 0x08:
        Serial.println(F("POWER ON"));
        break;
    }

    // iFeel
    switch (bytes[9] & 0x32) {
      case 0x00:
        Serial.println(F("IFEEL OFF"));
        break;
      case 0x32:
        Serial.println(F("IFEEL ON"));
        break;
    }

    // Operating mode
    switch (bytes[2] & 0x0F) {
      case 0x00:
        Serial.println(F("MODE COOL"));
        break;
      case 0x02:
        Serial.println(F("MODE DRY"));
        break;
      case 0x04:
        Serial.println(F("MODE COOL AUTO"));
        break;
      case 0x08:
        Serial.println(F("MODE HEAT"));
        break;
    }

    // Fan speed
    Serial.print(F("Fan speed: "));
    switch (bytes[3] & 0x06) {
      case 0x00:
        Serial.println(F("AUTO"));
        break;
      case 0x02:
        Serial.println(F("1"));
        break;
      case 0x04:
        Serial.println(F("2"));
        break;
      case 0x06:
        Serial.println(F("3"));
        break;
    }

    // Vertical air direction
    Serial.print(F("Vertical air direction: "));
    switch (bytes[4]) {
      case 0x00:
        Serial.println(F("AUTO"));
        break;
      case 0x01:
        Serial.println(F("UP"));
        break;
      case 0x02:
        Serial.println(F("MIDDLE UP"));
        break;
      case 0x03:
        Serial.println(F("MIDDLE"));
        break;
      case 0x04:
        Serial.println(F("MIDDLE"));
        break;
      case 0x05:
        Serial.println(F("MIDDLE DOWN"));
        break;
      case 0x06:
        Serial.println(F("DOWN"));
        break;
      case 0x07:
        Serial.println(F("SWING"));
        break;
    }

    Serial.print("Temperature: ");
    int8_t temperature = (((bytes[2] & 0xF0) >> 4) + ((bytes[3] & 0x01) << 4)) + 4;
    Serial.println(temperature);



    // TODO: How to calculate the checksum? This algorithm seems to get bits 0, 6 and 7 right, but how about the other bits?
    // Here's some material for testing:
    //
    // Hh001101011010111100000101100001010101000000000111000000001111100110100000000001000101001001
    // Hh001101011010111100000111100001010101000000000111000000001111100110100000000001000101001011
    // Hh001101011010111100000100010001010101000000000111000000001110010110100000000001000111011000
    // Hh001101011010111100000110010001010101000000000111000000001110010110100000000001000111011010
    // Hh001101011010111100000101010001010101000000000111000000001110010110100000000001000111011001
    // Hh001101011010111100000111010001010101000000000111000000001110010110100000000001000111011011
    // Hh001101011010111100000100110001010101000000000111000000001110010110100000000001000100111000
    // Hh001101011010111100000110110001010101000000000111000000001110010110100000000001000100111010
    // Hh001101011010111100000101110001010101000000000111000000001110010110100000000001000100111001
    // Hh001101011010111100000111110001010101000000000111000000001110010110100000000001000100111011
    // Hh001101011010111100000100001001010101000000000111000000001111010110100000000001000110100100
    // Hh001101011010111100000110001001010101000000000111000000001111010110100000000001000110100110
    // Hh001101011010111100000101001001010101000000000111000000001111010110100000000001000110100101
    // Hh001101011010111100000111001001010101000000000111000000001111010110100000000001000110100111
    // Hh001101011010111100000100101001010101000000000111000000001111010110100000000001000101100100
    // Hh001101011010111100000110101001010101000000000111000000001111010110100000000001000101100110
    // Hh001101011010111100000101101001010101000000000111000000001111010110100000000001000101100101
    // Hh001101011010111100000111101001010101000000000111000000001111110110100000000001000101101111
    // Hh001101011010111100000100011001010101000000000111000000001111110110100000000001000111101100
    // Hh001101011010111100000110011001010101000000000111000000001111110110100000000001000111101110
    // Hh001101011010111100000101011001010101000000000111000000001111110110100000000001000111101101
    // Hh001101011010111100000111011001010101000000000111000000001111110110100000000001000111101111
    // Hh001101011010111100000100111001010101000000000111000000001111110110100000000001000100011100

    byte checksum = 0b10011010;

    for (int i=0; i<10; i++) {
      checksum ^= originalBytes[i];
    }

    Serial.print(bytes[10], BIN);
    Serial.println(F(" <- original checksum"));
    Serial.print(checksum, BIN);
    Serial.println(F(" <- calculated checksum"));

    if (((checksum & 0b11000000) == (bytes[10] & 0b11000000)) && ((checksum & 0b00000001) == (bytes[10] & 0b00000001))) {
      Serial.println(F("Checksum matches at least partially"));
    } else {
      Serial.println(F("Checksum does not match"));
    }

    return true;
  }

  return false;
}



