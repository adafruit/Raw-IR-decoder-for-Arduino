#include <Arduino.h>

// PoC Airwell RC-3 remote support. 
// Works with https://www.aliexpress.com/item/A-C-controller-Air-Conditioner-air-conditioning-remote-control-FOR-YORK-Electra-Airwell-Emailair-RC-4/32634241533.html
// Reads fine in receive mode 1 or 2
// Only supports 2 operation modes — ♺ and ❄
// Reports:
//  - Mode: 1 - ♺, 2 - ❄
//  - Fan speed: 1, 2, 3, Auto
//  - Set temperature
//  - If power button is pressed, reports it

bool decodeAirwell(char* symbols, int bitCount)
{
  // If wrong package length 
  if (bitCount != 110) { return false; }
  // If incorrect header
  if (symbols[1] != 'H' || symbols[2] != 'h') { return false; }
  // If incorrect first payload footer
  if (symbols[35] != 'H' || symbols[36] != 'h') { return false; }

  Serial.println(F("Looks like an Airwell RC-3 remote"));

  if (symbols[3] == 'H' && symbols[4] == 'h' && (symbols[5] == '0' || symbols[5] == '1')) {
    Serial.print(F("POWER"));
    return true;
  }

  int acMode;
       if (symbols[3] == '0' && symbols[4] == '1' && symbols[5] == '0') { acMode = 1; }
  else if (symbols[3] == '0' && symbols[4] == '0' && symbols[5] == '1') { acMode = 2; }
  else {
    Serial.println(F("Mode: UNKNOWN"));
    return false;
  }

  Serial.print(F("Mode: "));
  Serial.println(acMode);

  int acSpeed;
  if      (symbols[6] == 'H' && symbols[7] == 'h' && symbols[8] == '0' && symbols[9] == '0') { acSpeed = 1; }
  else if (symbols[6] == 'H' && symbols[7] == 'h' && symbols[8] == 'H' && symbols[9] == 'h') { acSpeed = 2; }
  else if (symbols[6] == '0' && symbols[7] == 'H' && symbols[8] == 'h' && symbols[9] == '0') { acSpeed = 3; }
  else if (symbols[6] == '0' && symbols[7] == '0' && symbols[8] == 'H' && symbols[9] == 'h') { acSpeed = 0; }
  else {
    Serial.println(F("Speed: UNKNOWN"));
    return false;
  }

  Serial.print(F("Speed: "));
  Serial.println((acSpeed == 0) ? "AUTO" : String(acSpeed));

  int acTemp;
  if      (symbols[13] == '0' && symbols[14] == '0' && symbols[15] == '0' && symbols[16] == '1' && symbols[17] == 'H' && symbols[18] == 'h') { acTemp = 16; }
  else if (symbols[13] == '0' && symbols[14] == '0' && symbols[15] == '1' && symbols[16] == 'H' && symbols[17] == 'h' && symbols[18] == '0') { acTemp = 17; }
  else if (symbols[13] == '0' && symbols[14] == '0' && symbols[15] == '1' && symbols[16] == '0' && symbols[17] == 'H' && symbols[18] == 'h') { acTemp = 18; }
  else if (symbols[13] == '0' && symbols[14] == '1' && symbols[15] == 'H' && symbols[16] == 'h' && symbols[17] == '0' && symbols[18] == '0') { acTemp = 19; }
  else if (symbols[13] == '0' && symbols[14] == '1' && symbols[15] == 'H' && symbols[16] == 'h' && symbols[17] == 'H' && symbols[18] == 'h') { acTemp = 20; }
  else if (symbols[13] == '0' && symbols[14] == '1' && symbols[15] == '0' && symbols[16] == 'H' && symbols[17] == 'h' && symbols[18] == '0') { acTemp = 21; }
  else if (symbols[13] == '0' && symbols[14] == '1' && symbols[15] == '0' && symbols[16] == '0' && symbols[17] == 'H' && symbols[18] == 'h') { acTemp = 22; }
  else if (symbols[13] == '1' && symbols[14] == 'H' && symbols[15] == 'h' && symbols[16] == '0' && symbols[17] == '0' && symbols[18] == '0') { acTemp = 23; }
  else if (symbols[13] == '1' && symbols[14] == 'H' && symbols[15] == 'h' && symbols[16] == '1' && symbols[17] == 'H' && symbols[18] == 'h') { acTemp = 24; }
  else if (symbols[13] == '1' && symbols[14] == 'H' && symbols[15] == 'h' && symbols[16] == 'H' && symbols[17] == 'h' && symbols[18] == '0') { acTemp = 25; }
  else if (symbols[13] == '1' && symbols[14] == 'H' && symbols[15] == 'h' && symbols[16] == '0' && symbols[17] == 'H' && symbols[18] == 'h') { acTemp = 26; }
  else if (symbols[13] == '1' && symbols[14] == '0' && symbols[15] == 'H' && symbols[16] == 'h' && symbols[17] == '0' && symbols[18] == '0') { acTemp = 27; }
  else if (symbols[13] == '1' && symbols[14] == '0' && symbols[15] == 'H' && symbols[16] == 'h' && symbols[17] == 'H' && symbols[18] == 'h') { acTemp = 28; }
  else if (symbols[13] == '1' && symbols[14] == '0' && symbols[15] == '0' && symbols[16] == 'H' && symbols[17] == 'h' && symbols[18] == '0') { acTemp = 29; }
  else if (symbols[13] == '1' && symbols[14] == '0' && symbols[15] == '0' && symbols[16] == '0' && symbols[17] == 'H' && symbols[18] == 'h') { acTemp = 30; }
  else {
    Serial.println(F("Temp: UNKNOWN"));
    return false;
  }

  Serial.print(F("Temp: "));
  Serial.println(acTemp);

  return true;
}


/* Example payloads. They are repeated thrice in 110 symbol packet, only the first repeat is shown:

         M  FAN    TEMP 
        / \/  \   /    \
16/1: Hh010Hh000000001Hh0000000000000001Hh
17/1: Hh010Hh00000001Hh00000000000000001Hh
18/1: Hh010Hh000000010Hh0000000000000001Hh
19/1: Hh010Hh0000001Hh000000000000000001Hh
20/1: Hh010Hh0000001HhHh0000000000000001Hh
20/2: Hh010HhHh00001HhHh0000000000000001Hh
20/3: Hh0100Hh000001HhHh0000000000000001Hh
20/A: Hh01000Hh00001HhHh0000000000000001Hh
21/1: Hh010Hh00000010Hh00000000000000001Hh
22/1: Hh010Hh000000100Hh0000000000000001Hh
23/1: Hh010Hh000001Hh0000000000000000001Hh
24/1: Hh010Hh000001Hh1Hh0000000000000001Hh
25/1: Hh010Hh000001HhHh00000000000000001Hh
26/1: Hh010Hh000001Hh0Hh0000000000000001Hh
27/1: Hh010Hh0000010Hh000000000000000001Hh
28/1: Hh010Hh0000010HhHh0000000000000001Hh
29/1: Hh010Hh00000100Hh00000000000000001Hh
30/1: Hh010Hh000001000Hh0000000000000001Hh
30/2: Hh010HhHh0001000Hh0000000000000001Hh
30/3: Hh0100Hh00001000Hh0000000000000001Hh
30/A: Hh01000Hh0001000Hh0000000000000001Hh 
 
*/
