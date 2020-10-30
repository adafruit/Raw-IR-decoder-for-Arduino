#include <Arduino.h> // Hitachi RAR-5E1 remote

bool decodeHitachi(byte *bytes, int byteCount)
{
  if (byteCount == 28 && bytes[0] == 0x01 && bytes[1] == 0x10) {
    Serial.println(F("Looks like a Hitachi protocol"));

     // Operating mode
     Serial.print(F("Mode: "));
     switch (bytes[10]) {
       case 0x02:
         Serial.println(F("Auto"));
         break;
       case 0x03:
         Serial.println(F("Heat"));
         break;
       case 0x04:
         Serial.println(F("Cool"));
         break;
       case 0x05:
         Serial.println(F("Dry"));
         break;
     }

     // Fan speed
     Serial.print(F("Fan speed: "));
     switch (bytes[13]) {
       case 0x01:
         Serial.println(F("Auto"));
         break;
       case 0x02:
         Serial.println(F("1"));
         break;
       case 0x03:
         Serial.println(F("2"));
         break;
       case 0x04:
         Serial.println(F("3"));
         break;
       case 0x05:
         Serial.println(F("4"));
         break;
     }

     // Vertical air swing
     Serial.print(F("Vertical air swing: "));
     switch (bytes[14] & 0x01) {
       case 0x00:
         Serial.println(F("Off"));
         break;
       case 0x01:
         Serial.println(F("On"));
         break;
     }

     // Horisontal air Swing
     Serial.print(F("Horisontal air swing: "));
     switch (bytes[15] & 0x01) {
       case 0x00:
         Serial.println(F("Off"));
         break;
       case 0x01:
         Serial.println(F("On"));
         break;
     }

     // Power
     Serial.print(F("Power: "));
     switch (bytes[17]) {
       case 0x00:
         Serial.println(F("Off"));
         break;
       case 0x80:
         Serial.println(F("On"));
         break;
     }

     // ECO mode
     Serial.print(F("ECO: "));
     switch (bytes[25]) {
       case 0x00:
         Serial.println(F("Off"));
         break;
       case 0x02:
         Serial.println(F("On"));
         break;
     }

     //Temperature
     Serial.print("Temperature: ");
     Serial.println(bytes[11] >> 1);

     int checksum = 1086;

     for (byte i = 0; i < 27; i++) {
       checksum -= bytes[i];
     }

     if ( bytes[27] == checksum ) {
       Serial.println(F("Checksum matches"));
     } else {
       Serial.println(F("Checksum does not match"));
     }
     return true;
   }

   return false;
}

