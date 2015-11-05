/* Raw IR decoder sketch!

 This sketch/program uses the Arduno and a PNA4602 to
 decode IR received. This can be used to make a IR receiver
 (by looking for a particular code)
 or transmitter (by pulsing an IR LED at ~38KHz for the
 durations detected

 Code is public domain, check out www.ladyada.net and adafruit.com
 for more tutorials!
 */

// We need to use the 'raw' pin reading methods
// because timing is very important here and the digitalRead()
// procedure is slower!
//uint8_t IRpin = 2;
// Digital pin #2 is the same as Pin D2 see
// http://arduino.cc/en/Hacking/PinMapping168 for the 'raw' pin mapping

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define IRpin_PIN      PINE
#define IRpin          4
#else
#define IRpin_PIN      PIND
#define IRpin          2
#endif

// the maximum pulse we'll listen for - 65 milliseconds is a long time
#define MAXPULSE 65000

// what our timing resolution should be, larger is better
// as its more 'precise' - but too large and you wont get
// accurate timing
uint16_t RESOLUTION=20;


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

// The thresholds for different symbols
// These should work with Panasonic (from DKE on), Fujitsu and Mitsubishi
// Anyway, adjust these to get reliable readings

#define MARK_THRESHOLD_BIT_HEADER    2000 // Value between BIT MARK and HEADER MARK
#define SPACE_THRESHOLD_ZERO_ONE     800  // Value between ZERO SPACE and ONE SPACE
#define SPACE_THRESHOLD_ONE_HEADER   1300 // Value between ONE SPACE and HEADER SPACE
#define SPACE_THRESHOLD_HEADER_PAUSE 8000 // Value between HEADER SPACE and PAUSE SPACE (Panasonic/Midea only)

// This set works on the Panasonic CKP
/*
#define MARK_THRESHOLD_BIT_HEADER    2000 // Value between BIT MARK and HEADER MARK
#define SPACE_THRESHOLD_ZERO_ONE     1800 // Value between ZERO SPACE and ONE SPACE
#define SPACE_THRESHOLD_ONE_HEADER   3200 // Value between ONE SPACE and HEADER SPACE
#define SPACE_THRESHOLD_HEADER_PAUSE 8000 // Value between HEADER SPACE and PAUSE SPACE (Panasonic/Midea only)
*/

// This set works on Carrier & Midea / Ultimate Pro Plus 13 FP


/*
Panasonic CKP timings:
PAUSE SPACE:  13520
HEADER MARK:  3394
HEADER SPACE: 3326
BIT MARK:     812
ZERO SPACE:   751
ONE SPACE:    2449

Panasonic DKE, JNE and NKE timings:
PAUSE SPACE:  9700
HEADER MARK:  3439
HEADER SPACE: 1599
BIT MARK:     380
ZERO SPACE:   319
ONE SPACE:    1197

Fujitsu Nocria timings:
HEADER MARK   3327
HEADER SPACE  1519
BIT MARK      364
ZERO SPACE    277
ONE SPACE     1104

Mitsubishi FD-25 timings:
HEADER MARK:  3450
HEADER SPACE: 1700
BIT MARK:     500
ZERO SPACE:   350
ONE SPACE:    1250
*/


uint32_t mark_header_avg = 0;
uint16_t mark_header_cnt = 0;
uint32_t mark_bit_avg = 0;
uint16_t mark_bit_cnt = 0;
uint32_t space_zero_avg = 0;
uint16_t space_zero_cnt = 0;
uint32_t space_one_avg = 0;
uint16_t space_one_cnt = 0;
uint32_t space_header_avg = 0;
uint16_t space_header_cnt = 0;
uint32_t space_pause_avg = 0;
uint16_t space_pause_cnt = 0;

// we will store up to 500 symbols
char symbols[500];  // decoded symbols
uint16_t currentpulse = 0; // index for pulses we're storing

void setup(void) {
  Serial.begin(9600);
  delay(1000);
  Serial.println(F("Ready to decode IR!\n\n"));
}

void loop(void) {
  char incoming = 0;

mark_header_avg = 0;
mark_header_cnt = 0;
mark_bit_avg = 0;
mark_bit_cnt = 0;
space_zero_avg = 0;
space_zero_cnt = 0;
space_one_avg = 0;
space_one_cnt = 0;
space_header_avg = 0;
space_header_cnt = 0;
space_pause_avg = 0;
space_pause_cnt = 0;

  // Only Panasonic seems to use the pause
  space_pause_avg = 0;
  space_pause_cnt = 0;

  currentpulse=0;
  receivepulses();
  printpulses();
}

void receivepulses(void) {
  uint16_t highpulse, lowpulse;  // temporary storage timing

  while (currentpulse < sizeof(symbols))
  {
     highpulse = 0;
     while (IRpin_PIN & (1 << IRpin)) {
       // pin is still HIGH

       // count off another few microseconds
       highpulse++;
       delayMicroseconds(RESOLUTION);

       // If the pulse is too long, we 'timed out' - either nothing
       // was received or the code is finished, so print what
       // we've grabbed so far, and then reset
       if ((highpulse >= MAXPULSE) && (currentpulse != 0)) {
         return;
       }
    }

    highpulse = highpulse * RESOLUTION;

    if (currentpulse > 0)
    {
      // this is a SPACE
      if ( highpulse > SPACE_THRESHOLD_HEADER_PAUSE ) {
        symbols[currentpulse] = 'W';
        // Cumulative moving average, see http://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average
        space_pause_avg = (highpulse + space_pause_cnt * space_pause_avg) / ++space_pause_cnt;
      } else if ( highpulse > SPACE_THRESHOLD_ONE_HEADER ) {
        symbols[currentpulse] = 'h';
        // Cumulative moving average, see http://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average
        space_header_avg = (highpulse + space_header_cnt * space_header_avg) / ++space_header_cnt;
      } else if ( highpulse > SPACE_THRESHOLD_ZERO_ONE ) {
        symbols[currentpulse] = '1';
        space_one_avg = (highpulse + space_one_cnt * space_one_avg) / ++space_one_cnt;
      } else {
        symbols[currentpulse] = '0';
        space_zero_avg = (highpulse + space_zero_cnt * space_zero_avg) / ++space_zero_cnt;
      }
    }
    currentpulse++;

    // same as above
    lowpulse = 0;
    while (! (IRpin_PIN & _BV(IRpin))) {
       // pin is still LOW
       lowpulse++;
       delayMicroseconds(RESOLUTION);
       if ((lowpulse >= MAXPULSE)  && (currentpulse != 0)) {
         return;
       }
    }

    // this is a MARK

    lowpulse = lowpulse * RESOLUTION;

    if ( lowpulse > MARK_THRESHOLD_BIT_HEADER ) {
      symbols[currentpulse] = 'H';
      currentpulse++;
      mark_header_avg = (lowpulse + mark_header_cnt * mark_header_avg) / ++mark_header_cnt;
    } else {
      mark_bit_avg = (lowpulse + mark_bit_cnt * mark_bit_avg) / ++mark_bit_cnt;
    }

    // we read one high-low pulse successfully, continue!
  }
}

void printpulses(void) {

  int bitCount = 0;
  byte currentByte = 0;
  byte byteCount = 0;
  byte bytes[32];

  Serial.print("Number of symbols: ");
  Serial.println(currentpulse);

  Serial.println("Symbols:");
  Serial.println(symbols+1);

  Serial.println("Timings (in us): ");
  Serial.print("PAUSE SPACE:  ");
  Serial.println(space_pause_avg);
  Serial.print("HEADER MARK:  ");
  Serial.println(mark_header_avg);
  Serial.print("HEADER SPACE: ");
  Serial.println(space_header_avg);
  Serial.print("BIT MARK:     ");
  Serial.println(mark_bit_avg);
  Serial.print("ZERO SPACE:   ");
  Serial.println(space_zero_avg);
  Serial.print("ONE SPACE:    ");
  Serial.println(space_one_avg);


  // Decode the string of bits to a byte array
  for (int i = 0; i < currentpulse; i++) {
    if (symbols[i] == '0' || symbols[i] == '1') {
      currentByte >>= 1;
      bitCount++;

      if (symbols[i] == '1') {
        currentByte |= 0x80;
      }

      if (bitCount == 8) {
        bytes[byteCount++] = currentByte;
        bitCount = 0;
      }
    } else { // Ignore bits which do not form octets
      bitCount = 0;
      currentByte = 0;
    }
  }

  // Print the byte array
  for (int i = 0; i < byteCount; i++) {
    if (bytes[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(bytes[i],HEX);
    if ( i < byteCount - 1 ) {
      Serial.print(",");
    }
  }
  Serial.println();

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
  }

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

  }

  // If this looks like a Sharp code...
  if ( byteCount == 13 && bytes[0] == 0xAA && bytes[1] == 0x5A && bytes[2] == 0xCF ) {
    Serial.println("Looks like a Sharp protocol");

    // Power mode
    switch (bytes[5]) {
      case 0x21:
        Serial.println("POWER OFF");
        break;
      case 0x31:
        Serial.println("POWER ON");
        break;
    }

    // Operating mode
    switch (bytes[6] & 0x0F) {
      case 0x01:
        if (bytes[4] == 0x00) {
          Serial.println("MODE MAINTENANCE HEAT");
        } else {
          Serial.println("MODE HEAT");
        }
        break;
      case 0x02:
        Serial.println("MODE COOL");
        break;
      case 0x03:
        Serial.println("MODE DRY");
        break;
    }

    // Temperature
    Serial.print("Temperature: ");
    if (bytes[4] == 0x00) {
      Serial.println("10");
    } else {
      Serial.println(bytes[4] + 17);
    }

    switch (bytes[6] & 0xF0) {
      case 0x20:
        Serial.println("FAN: AUTO");
        break;
      case 0x30:
        Serial.println("FAN: 1");
        break;
      case 0x50:
        Serial.println("FAN: 2");
        break;
      case 0x70:
        Serial.println("FAN: 3");
        break;
    }

    // Check if the checksum matches
    byte checksum = 0x00;

    for (byte i = 0; i < 12; i++) {
      checksum ^= bytes[i];
    }

    checksum ^= bytes[12] & 0x0F;
    checksum ^= (checksum >> 4);
    checksum &= 0x0F;

    Serial.print("Checksum '0x");
    Serial.print(checksum, HEX);

    if ( ((bytes[12] & 0xF0) >> 4) == checksum ) {
      Serial.println("' matches");
    } else {
      Serial.print(" does not match ");
      Serial.println(((bytes[12] & 0xF0) >> 4), HEX);
    }
  }

  // If this looks like a Daikin code...
  if ( byteCount == 27 && bytes[0] == 0x11 && bytes[1] == 0xDA && bytes[2] == 0x27 ) {
    Serial.println("Looks like a Daikin protocol");

    // Power mode
    switch (bytes[13] & 0x01) {
      case 0x00:
        Serial.println("POWER OFF");
        break;
      case 0x01:
        Serial.println("POWER ON");
        break;
    }

    // Operating mode
    switch (bytes[13] & 0x70) {
      case 0x00:
        Serial.println("MODE AUTO");
        break;
      case 0x40:
        Serial.println("MODE HEAT");
        break;
      case 0x30:
        Serial.println("MODE COOL");
        break;
      case 0x20:
        Serial.println("MODE DRY");
        break;
      case 0x60:
        Serial.println("MODE FAN");
        break;
      }

    // Temperature
    Serial.print("Temperature: ");
    Serial.println(bytes[14] / 2);

    // Fan speed
    switch (bytes[16] & 0xF0) {
      case 0xA0:
        Serial.println("FAN: AUTO");
        break;
      case 0x30:
        Serial.println("FAN: 1");
        break;
      case 0x40:
        Serial.println("FAN: 2");
        break;
      case 0x50:
        Serial.println("FAN: 3");
        break;
      case 0x60:
        Serial.println("FAN: 4");
        break;
      case 0x70:
        Serial.println("FAN: 5");
        break;
    }

    // Check if the checksum matches
    byte checksum = 0x00;

    for (byte i = 0; i < 7; i++) {
      checksum += bytes[i];
    }

    if ( bytes[7] == checksum ) {
      Serial.println("Checksum 1 matches");
    } else {
      Serial.println("Checksum 1 does not match");
    }

    checksum = 0x00;

    for (byte i = 8; i < 26; i++) {
      checksum += bytes[i];
    }

    if ( bytes[26] == checksum ) {
      Serial.println("Checksum 2 matches");
    } else {
      Serial.println("Checksum 2 does not match");
    }
  }
}

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
