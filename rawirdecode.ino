#include <Arduino.h>

bool decodeMitsubishiElectric(byte *bytes, int byteCount);
bool decodeFujitsu(byte *bytes, int byteCount);
bool decodeMitsubishiHeavy(byte *bytes, int byteCount);
bool decodeDaikin(byte *bytes, int byteCount);
bool decodeSharp(byte *bytes, int byteCount);
bool decodeCarrier(byte *bytes, int byteCount);
bool decodePanasonicCKP(byte *bytes, int byteCount);
bool decodeHyundai(byte *bytes, int pulseCount);
bool decodeGree(byte *bytes, int pulseCount);
bool decodeFuego(byte *bytes, int byteCount);
bool decodeToshiba(byte *bytes, int byteCount);
bool decodeNibe(byte *bytes, char* symbols, int bitCount);
bool decodeHitachi(byte *bytes, int byteCount);
bool decodeSamsung(byte *bytes, int byteCount);
bool decodeBallu(byte *bytes, int byteCount);
bool decodeAUX(byte *bytes, int byteCount);


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

// The thresholds for different symbols
uint16_t MARK_THRESHOLD_BIT_HEADER    = 0; // Value between BIT MARK and HEADER MARK
uint16_t SPACE_THRESHOLD_ZERO_ONE     = 0; // Value between ZERO SPACE and ONE SPACE
uint16_t SPACE_THRESHOLD_ONE_HEADER   = 0; // Value between ONE SPACE and HEADER SPACE
uint16_t SPACE_THRESHOLD_HEADER_PAUSE = 0; // Value between HEADER SPACE and PAUSE SPACE (Panasonic/Midea only)


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

// we will store up to 1024 symbols
char symbols[1024];  // decoded symbols
uint16_t currentpulse = 0; // index for pulses we're storing

uint8_t modelChoice = 0;

// Decoded bytes
byte byteCount = 0;
byte bytes[128];


void setup(void) {

  Serial.begin(9600);
  delay(1000);
  Serial.println(F("Select model to decode (this affects the IR signal timings detection):"));
  Serial.println(F("* '1' for Panasonic DKE>, Mitsubishi Electric, Fujitsu etc. codes"));
  Serial.println(F("* '2' for Panasonic CKP, Midea etc. codes"));
  Serial.println(F("* '3' for Mitsubishi Heavy etc. codes"));
  Serial.println(F("* '4' for Hyundai etc. codes"));
  Serial.println(F("* '5' for Samsung etc. codes"));
  Serial.println(F("* '9' for entering the bit sequence on the serial monitor (instead of the IR receiver)"));
  Serial.println();
  Serial.print(F("Enter choice: "));

  while (modelChoice == 0) {
    int selection = Serial.read();

    if ( selection != -1 ) {
      Serial.print((char)selection);

      switch ((char)selection) {
        case '1':
          modelChoice = 1;
          break;
        case '2':
          modelChoice = 2;
          break;
        case '3':
          modelChoice = 3;
          break;
        case '4':
          modelChoice = 4;
          break;
        case '5':
          modelChoice = 5;
          break;
        case '9':
          modelChoice = 9;
          break;
          }
    }
  }

  Serial.print(F("\n\nReady to decode IR for choice '"));
  Serial.print(modelChoice);
  Serial.println(F("'\n\n"));

  if (modelChoice == 1) {
    MARK_THRESHOLD_BIT_HEADER    = 2000;
    SPACE_THRESHOLD_ZERO_ONE     =  800;
    SPACE_THRESHOLD_ONE_HEADER   = 1500;
    SPACE_THRESHOLD_HEADER_PAUSE = 8000;
  } else if (modelChoice == 2) {
    MARK_THRESHOLD_BIT_HEADER    = 2000;
    SPACE_THRESHOLD_ZERO_ONE     = 1800;
    SPACE_THRESHOLD_ONE_HEADER   = 3200;
    SPACE_THRESHOLD_HEADER_PAUSE = 8000;
  } else if (modelChoice == 3) {
    MARK_THRESHOLD_BIT_HEADER    = 2000;
    SPACE_THRESHOLD_ZERO_ONE     =  800;
    SPACE_THRESHOLD_ONE_HEADER   = 1400;
    SPACE_THRESHOLD_HEADER_PAUSE = 8000;
  } else if (modelChoice == 4) {
    MARK_THRESHOLD_BIT_HEADER    = 2000;
    SPACE_THRESHOLD_ZERO_ONE     =  800;
    SPACE_THRESHOLD_ONE_HEADER   = 2400;
    SPACE_THRESHOLD_HEADER_PAUSE = 8000;
  } else if (modelChoice == 5) {
    MARK_THRESHOLD_BIT_HEADER    = 2000;
    SPACE_THRESHOLD_ZERO_ONE     =  800;
    SPACE_THRESHOLD_ONE_HEADER   = 2400;
    SPACE_THRESHOLD_HEADER_PAUSE = 10000;
  }
}

void loop(void) {

  memset(symbols, 0, sizeof(symbols));
  memset(bytes, 0, sizeof(bytes));

  currentpulse=0;
  byteCount=0;
  if (modelChoice != 9) {
    receivePulses();
  } else {
    while ((currentpulse = Serial.readBytesUntil('\n', symbols+1, sizeof(symbols)-1)) == 0) {}
    currentpulse++;
  }

  printPulses();
  decodeProtocols();
}

void receivePulses(void) {
  uint16_t highpulse, lowpulse;  // temporary storage timing

  // Initialize the averages every time
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
      } else if ( (currentpulse > 0 && symbols[currentpulse-1] == 'H') || highpulse > SPACE_THRESHOLD_ONE_HEADER ) {
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

void printPulses(void) {

  int bitCount = 0;
  byte currentByte = 0;

  Serial.print(F("\nNumber of symbols: "));
  Serial.println(currentpulse);

  // Print the symbols (0, 1, H, h, W)
  Serial.println(F("Symbols:"));
  Serial.println(symbols+1);

  // Print the decoded bytes
  Serial.println(F("Bytes:"));

  // Decode the string of bits to a byte array
  for (uint16_t i = 0; i < currentpulse; i++) {
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
      Serial.print(F("0"));
    }
    Serial.print(bytes[i],HEX);
    if ( i < byteCount - 1 ) {
      Serial.print(F(","));
    }
  }
  Serial.println();

  // Print the timing constants
  Serial.println(F("Timings (in us): "));
  Serial.print(F("PAUSE SPACE:  "));
  Serial.println(space_pause_avg);
  Serial.print(F("HEADER MARK:  "));
  Serial.println(mark_header_avg);
  Serial.print(F("HEADER SPACE: "));
  Serial.println(space_header_avg);
  Serial.print(F("BIT MARK:     "));
  Serial.println(mark_bit_avg);
  Serial.print(F("ZERO SPACE:   "));
  Serial.println(space_zero_avg);
  Serial.print(F("ONE SPACE:    "));
  Serial.println(space_one_avg);
}

void decodeProtocols()
{
  Serial.println(F("Decoding known protocols..."));

  if ( ! (decodeMitsubishiElectric(bytes, byteCount) ||
          decodeFujitsu(bytes, byteCount) ||
          decodeMitsubishiHeavy(bytes, byteCount) ||
          decodeSharp(bytes, byteCount) ||
          decodeDaikin(bytes, byteCount) ||
          decodeCarrier(bytes, byteCount) ||
          decodeCarrier(bytes, byteCount) ||
          decodePanasonicCKP(bytes, byteCount) ||
          decodeHyundai(bytes, currentpulse) ||
          decodeGree(bytes, currentpulse) ||
          decodeFuego(bytes, byteCount) ||
          decodeToshiba(bytes, byteCount) ||
          decodeNibe(bytes, symbols, currentpulse) ||
          decodeHitachi(bytes, byteCount) ||
          decodeSamsung(bytes, byteCount) ||
          decodeBallu(bytes, byteCount) ||
          decodeAUX(bytes, byteCount)))
  {
    Serial.println(F("Unknown protocol"));
  }
}
