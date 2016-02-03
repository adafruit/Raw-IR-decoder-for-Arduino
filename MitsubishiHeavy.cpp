#include <Arduino.h>

bool decodeMitsubishiHeavy(byte *bytes, int byteCount)
{
  // If this looks like a Mitsubishi Heavy...
  if ( byteCount == 11 && bytes[0] == 0x52 && bytes[1] == 0xAE && bytes[2] == 0xC3 && bytes[3] == 0x26 && bytes[4] == 0xD9) {
    Serial.println(F("Looks like a Mitsubishi Heavy ZJ-S protocol"));
    Serial.println(F("Model SRKxxZJ-S Remote Control RKX502A001C"));
    
     // Power mode
    switch (bytes[9] & 0x08) {
      case 0x00:
        Serial.println(F("POWER ON"));
        break;
      case 0x08:
        Serial.println(F("POWER OFF"));
        break;
    }

    // Operating mode
    switch (bytes[9] & 0x07) {
      case 0x07:
        Serial.println(F("MODE AUTO"));
        break;
      case 0x03:
        Serial.println(F("MODE HEAT"));
        break;
      case 0x06:
        Serial.println(F("MODE COOL"));
        break;
      case 0x05:
        Serial.println(F("MODE DRY"));
        break;
      case 0x04:
        Serial.println(F("MODE FAN"));
        break;
    }

    // Temperature
    Serial.print(F("Temperature: "));
    Serial.println((~((bytes[9] & 0xF0) >> 4) & 0x0F) + 17);

    // Fan speed
    switch (bytes[7] & 0xE0) {
      case 0xE0:
        Serial.println(F("FAN AUTO"));
        break;
      case 0xA0:
        Serial.println(F("FAN 1"));
        break;
      case 0x80:
        Serial.println(F("FAN 2"));
        break;
      case 0x60:
        Serial.println(F("FAN 3"));
        break;
      case 0x20:
        Serial.println(F("HI POWER MODE"));
        break;
      case 0x00:
        Serial.println(F("ECONO MODE"));
        break;
    }

    // Vertical air direction
    Serial.print(F("Vertical air direction: "));
    switch ((bytes[5] & 0b00000010) | (bytes[7] & 0b00011000)) {
      case 0x0A:
        Serial.println(F("SWING"));
        break;
      case 0x02:
        Serial.println(F("UP"));
        break;
      case 0x18:
        Serial.println(F("MIDDLE UP"));
        break;
      case 0x10:
        Serial.println(F("MIDDLE"));
        break;
      case 0x08:
        Serial.println(F("MIDDLE DOWN"));
        break;
      case 0x00:
        Serial.println(F("DOWN"));
        break;
      case 0x1A:
        Serial.println(F("STOP"));
        break;
      }

    // Horizontal air direction
    Serial.print(F("Horizontal air direction: "));
    switch (bytes[5] & 0b11001100) {
      case 0xC8:
        Serial.println(F("LEFT"));
        break;
      case 0x88:
        Serial.println(F("MIDDLE LEFT"));
        break;   
      case 0x48:
        Serial.println(F("MIDDLE"));
        break;
      case 0x08:
        Serial.println(F("MIDDLE RIGHT"));
        break;
      case 0xC4:
        Serial.println(F("RIGHT"));
        break;
      case 0x84:
        Serial.println(F("LEFT RIGHT"));
        break;  
      case 0x44:
        Serial.println(F("RIGHT LEFT"));
        break;                                            
      case 0x4C:
        Serial.println(F("SWING"));
        break;
      case 0xCC:
        Serial.println(F("STOP"));
        break;
      case 0x04:
        Serial.println(F("3D AUTO"));
        break; 
      }

    // Clean
    Serial.print(F("Clean: "));
    switch (bytes[5] & 0x20) {
      case 0x00:
        Serial.println(F("ON"));
        break;
      case 0x20:
        Serial.println(F("OFF"));
        break; 
    } 

    return true;
  } else if ( byteCount == 19 && bytes[0] == 0x52 && bytes[1] == 0xAE && bytes[2] == 0xC3 && bytes[3] == 0x1A && bytes[4] == 0xE5) {
    Serial.println(F("Looks like a Mitsubishi Heavy ZM-S protocol"));
    Serial.println(F("Model SRKxxZM-S Remote Control RLA502A700B"));
    
     // Power mode
    switch (bytes[5] & 0x08) {
      case 0x00:
        Serial.println(F("POWER ON"));
        break;
      case 0x08:
        Serial.println(F("POWER OFF"));
        break;
    }

    // Operating mode
    switch (bytes[5] & 0x07) {
      case 0x07:
        Serial.println(F("MODE AUTO"));
        break;
      case 0x03:
        Serial.println(F("MODE HEAT"));
        break;
      case 0x06:
        Serial.println(F("MODE COOL"));
        break;
      case 0x05:
        Serial.println(F("MODE DRY"));
        break;
      case 0x04:
        Serial.println(F("MODE FAN"));
        break;
    }

    // Temperature
    Serial.print(F("Temperature: "));
    Serial.println((~(bytes[7]) & 0x0F) + 17);

    // Fan speed
    switch (bytes[9] & 0x0F) {
      case 0x0F:
        Serial.println(F("FAN AUTO"));
        break;
      case 0x0E:
        Serial.println(F("FAN 1"));
        break;
      case 0x0D:
        Serial.println(F("FAN 2"));
        break;
      case 0x0C:
        Serial.println(F("FAN 3"));
        break;
      case 0x0B:
        Serial.println(F("FAN 4"));
        break;        
      case 0x07:
        Serial.println(F("HI POWER MODE"));
        break;
      case 0x09:
        Serial.println(F("ECONO MODE"));
        break;
    }

    // Vertical air direction
    Serial.print(F("Vertical air direction: "));
    switch ((bytes[11] & 0b11100000)) {
      case 0xE0:
        Serial.println(F("SWING"));
        break;
      case 0xC0:
        Serial.println(F("UP"));
        break;
      case 0xA0:
        Serial.println(F("MIDDLE UP"));
        break;
      case 0x80:
        Serial.println(F("MIDDLE"));
        break;
      case 0x60:
        Serial.println(F("MIDDLE DOWN"));
        break;
      case 0x40:
        Serial.println(F("DOWN"));
        break;
      case 0x20:
        Serial.println(F("STOP"));
        break;
      }

    // Horizontal air direction
    Serial.print(F("Horizontal air direction: "));
    switch (bytes[13] & 0x0F) {
      case 0x0E:
        Serial.println(F("LEFT"));
        break;
      case 0x0D:
        Serial.println(F("MIDDLE LEFT"));
        break;   
      case 0x0C:
        Serial.println(F("MIDDLE"));
        break;
      case 0x0B:
        Serial.println(F("MIDDLE RIGHT"));
        break;
      case 0x0A:
        Serial.println(F("RIGHT"));
        break;
      case 0x08:
        Serial.println(F("LEFT RIGHT"));
        break;  
      case 0x09:
        Serial.println(F("RIGHT LEFT"));
        break;                                            
      case 0x0F:
        Serial.println(F("SWING"));
        break;
      case 0x07:
        Serial.println(F("STOP"));
        break;
      }

    // 3D Auto
    Serial.print(F("3D Auto: "));
    switch (bytes[11] & 0b00010010) {
      case 0x00:
        Serial.println(F("ON"));
        break;
      case 0x12:
        Serial.println(F("OFF"));
        break; 
    } 
          
    // Night setback
    Serial.print(F("Night setback: "));
    switch (bytes[15] & 0x40) {
      case 0x00:
        Serial.println(F("ON"));
        break;
      case 0x40:
        Serial.println(F("OFF"));
        break; 
    }
    
    // Silent mode
    Serial.print(F("Silent mode: "));
    switch (bytes[15] & 0x80) {
      case 0x00:
        Serial.println(F("ON"));
        break;
      case 0x80:
        Serial.println(F("OFF"));
        break; 
    }   

    // Clean and alergen
    Serial.print(F("Clean or Alergen: "));
    switch (bytes[5] & 0x60) {
      case 0x00:
        Serial.println(F("CLEAN"));
        break;
      case 0x20:
        Serial.println(F("ALERGEN"));
        break; 
      case 0x60:
        Serial.println(F("OFF"));
        break;         
    } 
    return true;
  }  
  return false;
}
