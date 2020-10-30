#include <Arduino.h>


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

byte bitReverse(byte x)
{
  //          01010101  |         10101010
  x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
  //          00110011  |         11001100
  x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
  //          00001111  |         11110000
  x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
  return x;
}

bool decodeCarrier1(byte *bytes, int byteCount)
{
  // If this looks like a Carrier code...
  if ( byteCount == 18 && bytes[0] == 0x4F && bytes[1] == 0xB0 && (memcmp(bytes, bytes+9, 9) == 0)) {
    Serial.println(F("Looks like a Carrier protocol #1"));

  // Check if the checksum matches
  byte checksum = 0x00;

  for (byte i = 0; i < 8; i++) {
    checksum ^= bitReverse(bytes[i]);
  }

    switch (bytes[6] & 0x0F) {
      case 0x00:
        Serial.println(F("FAN: AUTO"));
        break;
      case 0x02:
        Serial.println(F("FAN: 1"));
        break;
      case 0x06:
        Serial.println(F("FAN: 2"));
        break;
      case 0x01:
        Serial.println(F("FAN: 3"));
        break;
      case 0x05:
        Serial.println(F("FAN: 4"));
        break;
      case 0x03:
        Serial.println(F("FAN: 5"));
        break;
    }

    switch (bytes[6] & 0xF0) {
      case 0xE0:
        Serial.println(F("MODE: OFF"));
        break;
      case 0x00:
        Serial.println(F("MODE: AUTO"));
        break;
      case 0x80:
        Serial.println(F("MODE: COOL"));
        break;
      case 0x40:
        Serial.println(F("MODE: DRY"));
        break;
      case 0xC0:
        Serial.println(F("MODE: HEAT"));
        break;
      case 0x20:
        Serial.println(F("MODE: FAN"));
        break;
    }

    checksum = bitReverse(checksum);

    const byte temperatures[]  = { 17, 25, 21, 29, 19, 27, 23, 00, 18, 26, 22, 30, 20, 28, 24 };


    Serial.print(F("Temperature: "));
    Serial.println(temperatures[bytes[5]]);

    char bin1[9];
    char bin2[9];
    char bin3[9];

    snprintf(bin1, sizeof(bin1), BYTETOBINARYPATTERN, BYTETOBINARY(checksum));
    snprintf(bin2, sizeof(bin2), BYTETOBINARYPATTERN, BYTETOBINARY(bytes[8]));
    snprintf(bin3, sizeof(bin3), BYTETOBINARYPATTERN, BYTETOBINARY(bytes[6]));


    Serial.print(F("ModeFan  "));
    Serial.println(bin3);


    Serial.print(F("Checksum "));
    Serial.print(bin1);

    if (checksum == bytes[8]) {
      Serial.println(F(" matches"));
    } else {
      Serial.println(F(" does not match real"));
      Serial.print(F("checksum "));
      Serial.println(bin2);
    }
    return true;
  }

  return false;
}

bool decodeCarrier2(byte *bytes, int byteCount)
{
  // If this looks like a Carrier code...
  if ( byteCount == 12 && ((bytes[0] == 0x4D && bytes[1] == 0xB2) || (bytes[0] == 0xAD && bytes[1] == 0x52)) && (memcmp(bytes, bytes+6, 6) == 0)) {
    Serial.println(F("Looks like a Carrier protocol #2"));

    if (bytes[0] == 0xAD && bytes[1] == 0x52)
    {
      if (bytes[4] == 0x55)
      {
        Serial.println(F("MODE: Frost guard"));
      }
      else
      {
        Serial.println(F("MODE: Turbo mode"));        
      }      
    }
    else
    {
      switch (bytes[2] & 0x20) {
        case 0x00:
          Serial.println(F("POWER: OFF"));
          break;
        case 0x20:
          Serial.println(F("POWER: ON"));
          break;
      }

      switch (bytes[2] & 0x07) {
        case 0x05:
          Serial.println(F("FAN: AUTO"));
          break;
        case 0x00:
          Serial.println(F("FAN: AUTO/DRY AUTO"));
          break;
        case 0x01:
          Serial.println(F("FAN: 1"));
          break;
        case 0x02:
          Serial.println(F("FAN: 2"));
          break;
        case 0x04:
          Serial.println(F("FAN: 3"));
          break;
      }

      switch (bytes[4] & 0x30) {
        case 0x10:
          Serial.println(F("MODE: AUTO"));
          break;
        case 0x00:
          Serial.println(F("MODE: COOL"));
          break;
        case 0x30:
          Serial.println(F("MODE: HEAT"));
          break;
        case 0x20:
          if ((bytes[4] & 0x0F) == 0x07) {
            Serial.println(F("MODE: FAN"));
          } else {
            Serial.println(F("MODE: DRY"));
          }
          break;
      }

      const byte temperatures[]  = { 17, 28, 24, 25, 20, 29, 21, 31, 18, 27, 23, 26, 19, 30, 22 };
      //                              0   1   2   3   4   5   6   7   8   9  10  11  12  13  14

      Serial.print(F("Temperature: "));
      Serial.println(temperatures[bytes[4] & 0x0F]);
    }

    // Check if the checksum matches
    uint8_t checksum1 = ~bytes[2];
    uint8_t checksum2 = ~bytes[4];

    if (checksum1 == bytes[3] && checksum2 == bytes[5]) {
      Serial.println(F("Checksum matches"));
    } else {
      Serial.println(F("Checksum does not match"));
    }

    return true;
  }

  return false;
}

bool decodeCarrier3(byte *bytes, int byteCount)
{
  // If this looks like a Carrier code...
  if ( byteCount == 20 && bytes[0] == 0x4F && bytes[1] == 0xB0 && (memcmp(bytes, bytes+10, 10) == 0)) {
    Serial.println(F("Looks like a Carrier protocol #3"));

    // Check if the checksum matches
    byte checksum = 0x00;

    for (byte i = 0; i < 9; i++) {
      checksum ^= bitReverse(bytes[i]);
    }

    switch (bytes[6] & 0x0F) {
      case 0x00:
        Serial.println(F("FAN: AUTO"));
        break;
      case 0x02:
        Serial.println(F("FAN: 1"));
        break;
      case 0x06:
        Serial.println(F("FAN: 2"));
        break;
      case 0x01:
        Serial.println(F("FAN: 3"));
        break;
      case 0x05:
        Serial.println(F("FAN: 4"));
        break;
      case 0x03:
        Serial.println(F("FAN: 5"));
        break;
    }

    switch (bytes[6] & 0xF0) {
      case 0xE0:
        Serial.println(F("MODE: OFF"));
        break;
      case 0x00:
        Serial.println(F("MODE: AUTO"));
        break;
      case 0x80:
        Serial.println(F("MODE: COOL"));
        break;
      case 0x40:
        Serial.println(F("MODE: DRY"));
        break;
      case 0xC0:
        Serial.println(F("MODE: HEAT"));
        break;
      case 0x20:
        Serial.println(F("MODE: FAN"));
        break;
    }
	
	if (bytes[2] == 0x20 && bytes[3] == 0xDF)
    {
      if (bytes[8] == 0x20)
      {
        Serial.println(F("ADVANCED: Frost guard (+8°C) Mode"));
      }
      else if (bytes[8] == 0xC0)
      {
        Serial.println(F("ADVANCED: Eco/Sleep Mode"));
      }
      else if (bytes[8] == 0x80)
      {
        Serial.println(F("ADVANCED: Hi POWER Mode"));
      }      
    }
	
    checksum = bitReverse(checksum);

    const byte temperatures[]  = { 17, 25, 21, 29, 19, 27, 23, 00, 18, 26, 22, 30, 20, 28, 24 };


    Serial.print(F("Temperature: "));
    Serial.println(temperatures[bytes[5]]);

    char bin1[9];
    char bin2[9];
    char bin3[9];

    snprintf(bin1, sizeof(bin1), BYTETOBINARYPATTERN, BYTETOBINARY(checksum));
    snprintf(bin2, sizeof(bin2), BYTETOBINARYPATTERN, BYTETOBINARY(bytes[9]));
    snprintf(bin3, sizeof(bin3), BYTETOBINARYPATTERN, BYTETOBINARY(bytes[6]));


    Serial.print(F("ModeFan  "));
    Serial.println(bin3);


    Serial.print(F("Checksum "));
    Serial.print(bin1);

    if (checksum == bytes[9]) {
      Serial.println(F(" matches"));
    } else {
      Serial.println(F(" does not match real"));
      Serial.print(F("checksum "));
      Serial.println(bin2);
    }
    return true;
  }

  return false;
}

bool decodeCarrier(byte *bytes, int byteCount)
{
  return decodeCarrier1(bytes, byteCount) || decodeCarrier2(bytes, byteCount) || decodeCarrier3(bytes, byteCount);
}
