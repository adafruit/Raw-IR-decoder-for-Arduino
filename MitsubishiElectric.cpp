#include <Arduino.h>

bool decodeMitsubishiElectric(byte *bytes, int byteCount)
{
  // If this looks like a Mitsubishi FD-25 or FE code...
  if ( byteCount == 36 && bytes[0] == 0x23 && (memcmp(bytes, bytes+18, 17) == 0)) {
    Serial.println("Looks like a Mitsubishi FD / FE series protocol");

    // Check if the checksum matches
    byte checksum = 0;

    for (int i=0; i<17; i++) {
      checksum += bytes[i];
    }

    if (checksum == bytes[17]) {
      Serial.println("Checksum matches");
    } else {
      Serial.println("Checksum does not match");
    }

    // Power mode
    switch (bytes[5]) {
      case 0x00:
        Serial.println("POWER OFF");
        break;
      case 0x20:
        Serial.println("POWER ON");
        break;
      default:
        Serial.println("POWER unknown");
        break;
    }

    // Operating mode
    switch (bytes[6] & 0x38) { // 0b00111000
      case 0x38:
        Serial.println("MODE FAN");
        break;
      case 0x20:
        Serial.println("MODE AUTO");
        break;
      case 0x08:
        if (bytes[15] == 0x20) {
          Serial.println("MODE MAINTENANCE HEAT (FE only)");
        } else {
          Serial.println("MODE HEAT");
        }
        break;
      case 0x18:
        Serial.println("MODE COOL");
        break;
      case 0x10:
        Serial.println("MODE DRY");
        break;
      default:
        Serial.println("MODE unknown");
        break;
    }

    // I-See
    switch (bytes[6] & 0x40) { // 0b01000000
      case 0x40:
        Serial.println("I-See: ON");
        break;
      case 0x00:
        Serial.println("I-See: OFF");
        break;
    }

    // Plasma
    switch (bytes[15] & 0x40) { // 0b01000000
      case 0x40:
        Serial.println("Plasma: ON");
        break;
      case 0x00:
        Serial.println("Plasma: OFF");
        break;
    }

    // Temperature
    Serial.print("Temperature: ");
    if (bytes[7] == 0x00) {
      Serial.println("10");
    } else {
      Serial.println(bytes[7] + 16);
    }

    // Fan speed
    switch (bytes[9] & 0x07) { // 0b00000111
      case 0x00:
        Serial.println("FAN AUTO");
        break;
      case 0x01:
        Serial.println("FAN 1");
        break;
      case 0x02:
        Serial.println("FAN 2");
        break;
      case 0x03:
        Serial.println("FAN 3");
        break;
      case 0x04:
        Serial.println("FAN 4");
        break;
      default:
        Serial.println("FAN unknown");
        break;
    }

    // Vertical air direction
    switch (bytes[9] & 0xF8) { // 0b11111000
      case 0x40: // 0b01000
        Serial.println("VANE: AUTO1?");
        break;
      case 0x48: // 0b01001
        Serial.println("VANE: UP");
        break;
      case 0x50: // 0b01010
        Serial.println("VANE: UP-1");
        break;
      case 0x58: // 0b01011
        Serial.println("VANE: UP-2");
        break;
      case 0x60: // 0b01100
        Serial.println("VANE: UP-3");
        break;
      case 0x68: // 0b01101
        Serial.println("VANE: DOWN");
        break;
      case 0x78: // 0b01111
        Serial.println("VANE: SWING");
        break;
      case 0x80: // 0b10000
        Serial.println("VANE: AUTO2?");
        break;
      case 0xB8: // 0b10111
        Serial.println("VANE: AUTO3?");
        break;
      default:
        Serial.println("VANE: unknown");
        break;
    }

    // Horizontal air direction
    switch (bytes[8] & 0xF0) { // 0b11110000
      case 0x10:
        Serial.println("WIDE VANE: LEFT");
        break;
      case 0x20:
        Serial.println("WIDE VANE: MIDDLE LEFT");
        break;
      case 0x30:
        Serial.println("WIDE VANE: MIDDLE");
        break;
      case 0x40:
        Serial.println("WIDE VANE: MIDDLE RIGHT");
        break;
      case 0x50:
        Serial.println("WIDE VANE: RIGHT");
        break;
      case 0xC0:
        Serial.println("WIDE VANE: SWING");
        break;
      default:
        Serial.println("WIDE VANE: unknown");
        break;
    }

    return true;
  }

  return false;
}
