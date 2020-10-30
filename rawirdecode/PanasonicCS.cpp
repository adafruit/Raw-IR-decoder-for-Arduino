/**
 * Code based on examples
 * Checked with Panasonic Inverter Air Conditioner CS-RE18GKE indoor and CU-RE18GKE outdoor split unit
 * Model known as WA-18HP/I/ST (year 2007-2008)
 * Remote: A75C3010
 * 
 * designed with help off http://www.instructables.com/id/Reverse-engineering-of-an-Air-Conditioning-control/
 * 
 */
#include <Arduino.h>

bool decodePanasonicCS(byte *bytes, int byteCount)
{
    // Test if looks like Pansonic code
    // check via byteCount and fixed values
    if (byteCount == 27 && bytes[10 == 0xE0]){ // I just picked a value here
        Serial.println(F("Look like a Panasonic CS protocol"));

        // check on-off
        // 13th byte from start, first byte with info
        // this is not a toggle, but a state
        if ((bytes[13] & 0x01) == 0x01) { // check if the right bit set 
            Serial.println(F("Powering ON!"));
        } else {
            Serial.println(F("Powering OFF"));
        }

        // check mode
        // if this byte is used for other things, first mask the correct bits
        Serial.print(F("Mode: "));
        switch ((bytes[13]) & 0xF0){ // masks the first 4 bits 1111 0000
            case 0x00:
                Serial.println(F("Auto"));
                break;
            case 0x40:
                Serial.println(F("Heat"));
                break;
            case 0x30:
                Serial.println(F("Cool"));
                break;
            case 0x20:
                Serial.println(F("Dry"));
                break;
            default:
                Serial.println(F("Error"));
                break;
        }
        // check temp
        Serial.print(F("Temperature: "));
        byte temp = (bytes[14] & 0x1E);
        temp = temp >> 1;
        Serial.println((temp) + 16); // masks the middle 5 bits: 0x1E = 0001 1110

        // check fanspeed
        Serial.print(F("Fan speed: "));
        switch (bytes[16] & 0xF0){ // 0xF0 = 1111 0000 eerste 4 bits
            case 0xA0:
                Serial.println(F("FAN AUTO"));
                break;
            case 0x30:
                Serial.println(F("FAN 1"));
                break;
            case 0x40:
                Serial.println(F("FAN 2"));
                break;
            case 0x50:
                Serial.println(F("FAN 3"));
                break;    
            case 0x60:
                Serial.println(F("FAN 4"));
                break;    
            case 0x70:
                Serial.println(F("FAN 5"));
                break;
            default:
                Serial.println(F("Error"));
                break;
        }

        // check vertical swing
        Serial.print(F("Vertical swing: "));
        switch (bytes[16] & 0x0F){ // 0x0F = 0000 1111
            case 0x0F:
                Serial.println(F("AUTO"));
                break;
            case 0x01:
                Serial.println(F("Straight"));
                break;
            case 0x02:
                Serial.println(F("Down 1"));
                break;
            case 0x03:
                Serial.println(F("Down 2"));
                break;    
            case 0x04:
                Serial.println(F("Down 3"));
                break;    
            case 0x05:
                Serial.println(F("Down 4"));
                break;
            default:
                Serial.println(F("Error"));
                break;
        }

        // check horizontal swing
        Serial.print(F("Horizontal swing: "));
        switch (bytes[17] & 0x0F){ // 0x0F = 0000 1111
            case 0x0D:
                Serial.println(F("AUTO"));
                break;
            case 0x06:
                Serial.println(F("Center"));
                break;
            case 0x09:
                Serial.println(F("Left"));
                break;
            case 0x0A:
                Serial.println(F("Left center"));
                break;    
            case 0x0B:
                Serial.println(F("Right center"));
                break;    
            case 0x0C:
                Serial.println(F("Right"));
                break;
            default:
                Serial.println(F("Error"));
                break;
        }

        // timer A (start)
        Serial.print(F("Timer A active: "));
        if((bytes[13] & 0x02) == 0x02){ //check if second bit in byte is active
            Serial.println(F("Yes"));
        } else {
            Serial.println(F("No"));
        }        

        // timer B (end)
        Serial.print(F("Timer B active: "));
        if((bytes[13] & 0x04) == 0x04){ //check if third bit in byte is active
            Serial.println(F("Yes"));
        } else {
            Serial.println(F("No"));
        }
        // no need to investigate actual timings if you're going to automate :-)

        // checksum algorithm
        // Assumption: bitwise sum of payload bytes (so ignore header, and last byte)
        byte checksum = 0x06; // found differce, so start with 0x06
        Serial.print(F("Add bytes: "));
        for (int i = 13; i < 26; i++){
            if (bytes[i] < 0x10) {
              Serial.print(F("0"));
            }
            Serial.print(bytes[i],HEX);
            Serial.print(F("+"));
            checksum = checksum + bytes[i];
        }
        Serial.println(F("."));
        Serial.print(F("Checksum Truncate: "));
        checksum = (checksum & 0xFF); // mask out only first byte)
        Serial.println(checksum,HEX);

        if (checksum == bytes[26]){
            Serial.println(F("----- Checksum OK ------"));
        }

        // good end
        return true;
    }

    return false;
}
