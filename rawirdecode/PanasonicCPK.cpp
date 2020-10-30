#include <Arduino.h>

bool decodePanasonicCKP(byte *bytes, int byteCount)
{
  // If this looks like a Panasonic CKP code...
  if ( byteCount == 16 && bytes[10] == 0x36 ) {
    Serial.println(F("Looks like a Panasonic CKP protocol"));

    switch (bytes[2] & 0x07)
    {
      case 0x06:
        Serial.println(F("AUTO"));
        break;
      case 0x04:
        Serial.println(F("HEAT"));
        break;
      case 0x02:
        Serial.println(F("COOL"));
        break;
      case 0x03:
        Serial.println(F("DRY"));
        break;
      case 0x01:
        Serial.println(F("FAN"));
        break;
      }

    if ((bytes[2] & 0x08) != 0x08)
    {
      Serial.println(F("POWER SWITCH"));
    }

    switch (bytes[0] & 0xF0)
    {
      case 0xF0:
        Serial.println(F("FAN AUTO"));
        break;
      case 0x20:
        Serial.println(F("FAN 1"));
        break;
      case 0x30:
        Serial.println(F("FAN 2"));
        break;
      case 0x40:
        Serial.println(F("FAN 3"));
        break;    
      case 0x50:
        Serial.println(F("FAN 4"));
        break;    
      case 0x60:
        Serial.println(F("FAN 5"));
        break;        
    }

    Serial.print(F("Temperature: "));
    Serial.println((bytes[0] & 0x0F) + 15);

    return true;
  }

  return false;
}

