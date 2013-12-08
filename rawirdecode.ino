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
#define IRpin_PIN      PIND
#define IRpin          2

// the maximum pulse we'll listen for - 65 milliseconds is a long time
#define MAXPULSE 65000

// what our timing resolution should be, larger is better
// as its more 'precise' - but too large and you wont get
// accurate timing
uint16_t RESOLUTION=20;


// The thresholds for different symbols
// These should work with Panasonic (from DKE on), Fujitsu and Mitsubishi
// Anyway, adjust these to get reliable readings

#define MARK_THRESHOLD_BIT_HEADER    2000 // Value between BIT MARK and HEADER MARK
#define SPACE_THRESHOLD_ZERO_ONE     800  // Value between ZERO SPACE and ONE SPACE
#define SPACE_THRESHOLD_ONE_HEADER   1400 // Value between ONE SPACE and HEADER SPACE
#define SPACE_THRESHOLD_HEADER_PAUSE 8000 // Value between HEADER SPACE and PAUSE SPACE (Panasonic only)

// This set works on the Panasonic CKP
/*
#define MARK_THRESHOLD_BIT_HEADER    2000 // Value between BIT MARK and HEADER MARK
#define SPACE_THRESHOLD_ZERO_ONE     1800 // Value between ZERO SPACE and ONE SPACE
#define SPACE_THRESHOLD_ONE_HEADER   3200 // Value between ONE SPACE and HEADER SPACE
#define SPACE_THRESHOLD_HEADER_PAUSE 8000 // Value between HEADER SPACE and PAUSE SPACE (Panasonic only)
*/

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

  memset(symbols, 0, sizeof(symbols)); // Wipe the symbols

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


  // Decode the string of symbols to HEX digits
  for (int i = 0; i < currentpulse; i++)
  {
    if (symbols[i] == '0' || symbols[i] == '1')
    {
      currentByte >>= 1;
      bitCount++;

      if (symbols[i] == '1') {
        currentByte |= 0x80;
      }

      if (bitCount == 8) {
        if (currentByte < 0x10) {
          Serial.print("0");
        }
        Serial.print(currentByte,HEX);
        Serial.print(",");
        bitCount = 0;
      }
    }
  }

  Serial.println();
}
