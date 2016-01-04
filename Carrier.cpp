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

bool decodeCarrier(byte *bytes, int byteCount)
{
  // If this looks like a Carrier code...
  if ( byteCount == 18 && bytes[0] == 0x4F && bytes[1] == 0xB0 && (memcmp(bytes, bytes+9, 9) == 0)) {
    Serial.println("Looks like a Carrier protocol");

    // Check if the checksum matches
    byte checksum = 0;

    checksum = bitReverse(bytes[0]) +
               bitReverse(bytes[1]) +
               bitReverse(bytes[2]) +
               bitReverse(bytes[3]) +
               bitReverse(bytes[4]) +
               bitReverse(bytes[5]) +
               bitReverse(bytes[6]) +
               bitReverse(bytes[7]);

    switch (bytes[6] & 0x0F) {
      case 0x00:
        Serial.println("FAN: AUTO");
        break;
      case 0x02:
        Serial.println("FAN: 1");
        break;
      case 0x06:
        Serial.println("FAN: 2");
        break;
      case 0x01:
        Serial.println("FAN: 3");
        break;
      case 0x05:
        Serial.println("FAN: 4");
        break;
      case 0x03:
        Serial.println("FAN: 5");
        break;
    }

    switch (bytes[6] & 0xF0) {
      case 0xE0:
        Serial.println("MODE: OFF");
        break;
      case 0x00:
        Serial.println("MODE: AUTO");
        checksum += 0x02;
        switch (bytes[6] & 0x0F) {
          case 0x02: // FAN1
          case 0x03: // FAN5
          case 0x06: // FAN2
            checksum += 0x80;
            break;
        }
        break;
      case 0x80:
        Serial.println("MODE: COOL");
        break;
      case 0x40:
        Serial.println("MODE: DRY");
        checksum += 0x02;
        break;
      case 0xC0:
        Serial.println("MODE: HEAT");
        switch (bytes[6] & 0x0F) {
          case 0x05: // FAN4
          case 0x06: // FAN2
            checksum += 0xC0;
            break;
        }
        break;
      case 0x20:
        Serial.println("MODE: FAN");
        checksum += 0x02;
        switch (bytes[6] & 0x0F) {
          case 0x02: // FAN1
          case 0x03: // FAN5
          case 0x06: // FAN2
            checksum += 0x80;
            break;
        }
        break;
    }

    checksum = bitReverse(checksum);

    const byte temperatures[]  = { 17, 25, 21, 29, 19, 27, 23, 00, 18, 26, 22, 30, 20, 28, 24 };


    Serial.print("Temperature: ");
    Serial.println(temperatures[bytes[5]]);

    char bin1[9];
    char bin2[9];
    char bin3[9];

    snprintf(bin1, sizeof(bin1), BYTETOBINARYPATTERN, BYTETOBINARY(checksum));
    snprintf(bin2, sizeof(bin2), BYTETOBINARYPATTERN, BYTETOBINARY(bytes[8]));
    snprintf(bin3, sizeof(bin3), BYTETOBINARYPATTERN, BYTETOBINARY(bytes[6]));


    Serial.print("ModeFan  ");
    Serial.println(bin3);


    Serial.print("Checksum ");
    Serial.print(bin1);

    if (checksum == bytes[8]) {
      Serial.println(" matches");
    } else {
      Serial.println(" does not match real");
      Serial.print("checksum ");
      Serial.println(bin2);
    }
    return true;
  }

  return false;
}
