#include <Arduino.h>

bool decodeBallu(byte *bytes, int byteCount)
{
  // If this looks like a Ballu, BGH code...
  // Remote control "J1-05(E)"
  if ( byteCount == 21 && bytes[0] == 0x83 && bytes[1] == 0x06 ) {
    Serial.println(F("Looks like a Ballu, BGH protocol"));

    // Power mode
    //its send same signal for on/off its depends unit status 
    if((bytes[2] >> 2) == 0x01){
    Serial.println(F("On/Off action "));
     }

    // TBD
    //bytes 6 and 7 are remote control time (hour and minutes)

    // Operating mode
    //Serial.print(F("Hex Mode ")); Serial.print (bytes[3],BIN); Serial.print (" ");  Serial.println (bytes[3] & 0x07); 
    switch (bytes[3] & 0x07) {
      case 0x00:
        Serial.println(F("MODE HEAT"));
        break;
      case 0x02:
        Serial.println(F("MODE COOL"));
        break;
      case 0x03:
        Serial.println(F("MODE DRY"));
        break;
      case 0x04:
        Serial.println(F("MODE FAN"));
        break;
      }

    // swing 
    if((bytes[2] >> 4) == 0x08 && (bytes[8] >> 4) == 0x04 ){
       Serial.println(F("Vertical Swing"));

    }
   
   
    // Temperature
    Serial.print(F("Temperature: "));
    Serial.println((bytes[3] >> 4) + 16);

    // Fan speed
    switch (bytes[02] & 0x03) {
      case 0x00:
        Serial.println(F("FAN: AUTO"));
        break;
      case 0x01:
        Serial.println(F("FAN: 1"));
        break;
      case 0x02:
        Serial.println(F("FAN: 2"));
        break;
      case 0x03:
        Serial.println(F("FAN: 3"));
        break;
    }

    // pressed buton	byte 15
    // Serial.print(F("Button Pressed: ")); Serial.print (bytes[15],BIN); Serial.print (" | ");  Serial.println (bytes[15], HEX); 
    switch (bytes[15]) {
      case 0x01:
        Serial.println(F("Button:	on/off"));
        break;
      case 0x02:
        Serial.println(F("Button: Temp"));
        break;
      case 0x04:
        Serial.println(F("Button: Super"));
        break;
      case 0x06:
        Serial.println(F("Button: Modo"));
        break;
      case 0x07:
        Serial.println(F("Button: Vertical Swing"));
        break;
      case 0x08:
        Serial.println(F("Button: Horizontal Swing"));
        break;
      case 0x11:
        Serial.println(F("Button: Fan"));
        break;
      case 0x17:
        Serial.println(F("Button: Smart"));
        break;
      case 0x0D:
        Serial.println(F("Button: iFeel"));
        break;
    }

    // Check if the checksum matches 
    // it has 2 checksums one at byte 13 and other at byte 20
    // first is XOR of bytes 2 to 12 
    // second is XOR of bytes 14 to 19
    byte checksum = 0;
    for (int i=2; i<13; i++) {
      // Serial.print(bytes[i], HEX);  Serial.print(" | "); Serial.println(bytes[i], BIN);
      checksum ^= bytes[i];
      // Serial.print(checksum, HEX);  Serial.print(" | "); Serial.println(checksum, BIN);
    }
    if (checksum == bytes[13]) {
      Serial.println(F("Firs Checksum matches"));
    } else {
      Serial.println(F("Fist Checksum does not match"));
    }
    checksum = 0;
    for (int i=14; i<20; i++) {
     // Serial.print(bytes[i], HEX);  Serial.print(" | "); Serial.println(bytes[i], BIN);
      checksum ^= bytes[i];
     // Serial.print(checksum, HEX);  Serial.print(" | "); Serial.println(checksum, BIN);
    }
    if (checksum == bytes[20]) {
      Serial.println(F("Second Checksum matches"));
    } else {
      Serial.println(F("Second Checksum does not match"));
    }

    return true;
  }

  return false;
}
