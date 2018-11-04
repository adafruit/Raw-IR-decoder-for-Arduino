#include <Arduino.h>

bool decodeGree_YAC(byte *bytes, int pulseCount)
{
  // If this looks like a Gree code...
  if ( pulseCount == 142 ) {
    Serial.println(F("Looks like a Gree YAC protocol"));

    // Check if the checksum matches
    uint8_t checksum0 = (
      (bytes[0] & 0x0F) +
      (bytes[1] & 0x0F) +
      (bytes[2] & 0x0F) +
      (bytes[3] & 0x0F) +
      ((bytes[4] & 0xF0) >> 4) +
      ((bytes[5] & 0xF0) >> 4) +
      ((bytes[6] & 0xF0) >> 4) +
      0x0A) & 0xF;

	uint8_t checksum1 = (
      (bytes[8] & 0x0F) +
      (bytes[9] & 0x0F) +
      (bytes[10] & 0x0F) +
      (bytes[11] & 0x0F) +
      ((bytes[12] & 0xF0) >> 4) +
      ((bytes[13] & 0xF0) >> 4) +
      ((bytes[14] & 0xF0) >> 4) +
      0x0A) & 0xF;

	  
	//byte 7[7:4] contais checksum for bytes 0-6
	//byte 15[7:4] contais checksum for bytes 8-14
    if ((checksum0 == (bytes[7]>>4)) &&  (checksum1 == (bytes[15]>>4))) { 
      Serial.println(F("Checksum matches"));
    } else {
      Serial.println(F("Checksum does not match"));
    }

    // Power mode
	// in bytes[0] and bytes[8]
    if (bytes[0] == bytes[8]){	
		switch (bytes[0] & 0x08) {
		  case 0x00:
			Serial.println(F("POWER OFF"));
			break;
		  case 0x08:
			Serial.println(F("POWER ON"));
			break;
		}
	}

	// Eco mode (8C)
	// bit2 of byte 7 (bits 7:4 are checksum)
	switch (bytes[7] & 0x04) {
	  case 0x00:
		Serial.println(F("ECO OFF"));
		break;
	  case 0x04:
		Serial.println(F("ECO ON"));
		break;
	}
	
	// Turbo mode
	// byte[2] bit 4
	switch (bytes[2] & 0x10) {
		case 0x00:
			Serial.println(F("TURBO OFF"));
			break;
		case 0x10:
			Serial.println(F("TURBO ON"));
			break;
	}	
	
    // Operating mode
	// in bytes[0] and bytes[8]
    if (bytes[0] == bytes[8]){
		switch (bytes[0] & 0x07) {
		  case 0x00:
			Serial.println(F("MODE AUTO"));
			break;
		  case 0x04:
			Serial.println(F("MODE HEAT"));
			break;
		  case 0x01:
			Serial.println(F("MODE COOL"));
			break;
		  case 0x02:
			Serial.println(F("MODE DRY"));
			break;
		  case 0x03:
			Serial.println(F("MODE FAN"));
			break;
		}
	}
	
    // Temperature
	// temp in two identical bytes 1 and 9, bits [3:0]
	// bits [7:4] are used with timers but can be left as 0
	if (bytes[1]==bytes[9]){ 
		Serial.print(F("Temperature: "));
		Serial.println((bytes[1] & 0x0F) + 16);
	}
	
    // Fan speed
	if (bytes[0] == bytes[8]){ //same fan speed in two bytes
		// note: fan speed 0-5 also in byte 14 bits [6:4] as 0-5
		// speed 0-3 in bytes[0] and bytes[8] bits [5:4] 0-3, speed 4-5 set as 3.
		switch (bytes[0] & 0x30) {
		  case 0x00:
			Serial.println(F("FAN: AUTO"));
			break;
		  case 0x10:
			Serial.println(F("FAN: 1"));
			break;
		  case 0x20:
			Serial.println(F("FAN: 2"));
			break;
		  case 0x30:
			switch (bytes[14] & 0x70) {
				case 0x30:
					Serial.println(F("FAN: 3"));	
					break;
				case 0x40:
					Serial.println(F("FAN: 4"));	
					break;
				case 0x50:
					Serial.println(F("FAN: 5"));	
					break;
			}
		    break;
		}
    }
		
    // Sleep mode
	// Note that YAC1FB has also Sleep 3-mode which sends 24 bytes and is not decoded here
    switch (bytes[0] & 0x80) {
      case 0x80:
        Serial.println(F("SLEEP: ON 1"));
        break;
      case 0x00:
		if ((bytes[12] & 0x01) == 1){
			Serial.println(F("SLEEP: ON 2"));
		} else {
			Serial.println(F("SLEEP: OFF"));
		}
        break;
    }

    // Air direction swing
	// in bytes[0] and bytes[8]
    if (bytes[0] == bytes[8]){		
		switch (bytes[0] & 0x40) {
		  case 0x40:
			Serial.println(F("SWING: ON"));
			break;
		  case 0x00:
			Serial.println(F("SWING: OFF"));
			break;
		}
	}
	
	// Vertical air direction
	switch (bytes[4] & 0x0F){
		case 0x00:
			Serial.println(F("VERT. VANE STOP"));
			break;
		case 0x01:
			Serial.println(F("VERT. VANE SWEEP UP-DOWN (1-5)"));
			break;		
		case 0x02:
			Serial.println(F("VERT. VANE UP (1)"));
			break;
		case 0x03:
			Serial.println(F("VERT. VANE HIGH (2)"));			
			break;			
		case 0x04:
			Serial.println(F("VERT. VANE CENTER (3)"));
			break;
		case 0x05:
			Serial.println(F("VERT. VANE LOW (4)"));			
			break;			
		case 0x06:
			Serial.println(F("VERT. VANE DOWN (5) "));
			break;			
		case 0x07:
			Serial.println(F("VERT. VANE SWEEP 3-5"));			
			break;			
		case 0x09:
			Serial.println(F("VERT. VANE SWEEP 2-4"));
			break;			
		case 0x0B:
			Serial.println(F("VERT. VANE SWEEP 1-3"));
			break;			
    }
	
	// Horizontal air direction
	switch (bytes[4] & 0xF0){
		case 0x00:
			Serial.println(F("HORIZ. VANE STOP"));
			break;
		case 0x10:
			Serial.println(F("HORIZ. VANE SWEEP LEFT-RIGHT (1-5)"));
			break;			
		case 0x20:
			Serial.println(F("HORIZ. VANE FAR LEFT (1)"));
			break;			
		case 0x30:
			Serial.println(F("HORIZ. VANE LEFT (2)"));
			break;			
		case 0x40:
			Serial.println(F("HORIZ. VANE CENTER (3)"));
			break;			
		case 0x50:
			Serial.println(F("HORIZ. VANE RIGHT (4)"));
			break;			
		case 0x60:
			Serial.println(F("HORIZ. VANE FAR RIGHT (5)"));
			break;		
		case 0xC0:
			Serial.println(F("HORIZ. VANE LEFT (1) AND RIGHT (5)"));
			break;				
		case 0xD0:
			Serial.println(F("HORIZ. VANE SWEEP TO CENTER"));
			break;				
	}
	
	// Display temperature
	// bytes[5] bit [0:1]
	switch (bytes[5] & 0x3) {
		case 0x00:
			Serial.println(F("DISPLAY: SET TEMP"));
			break;
		case 0x01:
			Serial.println(F("DISPLAY: SET TEMP"));
			break;
		case 0x02:
			Serial.println(F("DISPLAY: INDOOR TEMP"));
			break;
		case 0x03:
			Serial.println(F("DISPLAY: OUTDOOR TEMP"));
			break;

	}	
    return true;
  }

  return false;
}

