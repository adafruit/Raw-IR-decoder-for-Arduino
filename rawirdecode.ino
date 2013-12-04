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
#define RESOLUTION 20

// we will store up to 100 pulse pairs (this is -a lot-)
uint16_t pulses[400][2];  // pair is high and low pulse
uint16_t currentpulse = 0; // index for pulses we're storing

boolean nodata = true; // No name for this sample yet
char samplename[50];   // Holds the name of the sample

void setup(void) {
  Serial.begin(9600);
  delay(1000);
  Serial.println(F("Ready to decode IR!"));
  Serial.println(F("Enter the name of each sample and press 'Send' before sending an IR signal. The data is in JSON format\n\n"));

  Serial.println(F(""));
  Serial.println(F("{"));

}

void loop(void) {
  char incoming = 0;
  char sampleposition = 0;

  memset(samplename, 0, 50); // Wipe the name of the sample

  // Receive the name of the sample
  while (nodata) {
    while (nodata || Serial.available() ) {
      if (Serial.available())
      {
        //read the incoming character
        char incoming = Serial.read();
        samplename[sampleposition++] = incoming;
        nodata=false;
        delay(50);
      }
    }
  }

  // Print out the first part of the JSON array row
  Serial.print(F("   \""));
  Serial.print(samplename);
  Serial.print(F("\": [ "));
  Serial.flush();

  currentpulse=0;
  receivepulses();
  printpulses();
}

void receivepulses(void) {
  uint16_t highpulse, lowpulse;  // temporary storage timing

  while (true)
  {
     highpulse = lowpulse = 0; // start out with no pulse length

     //  while (digitalRead(IRpin)) { // this is too slow!
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
    // we didn't time out so lets stash the reading
    pulses[currentpulse][0] = highpulse;

    // same as above
    while (! (IRpin_PIN & _BV(IRpin))) {
       // pin is still LOW
       lowpulse++;
       delayMicroseconds(RESOLUTION);
       if ((lowpulse >= MAXPULSE)  && (currentpulse != 0)) {
         return;
       }
    }
    pulses[currentpulse][1] = lowpulse;

    // we read one high-low pulse successfully, continue!
    currentpulse++;
  }
}

void printpulses(void) {
  // print it in JSON format

  for (uint8_t i = 0; i < currentpulse-1; i++) {
    Serial.print(pulses[i][1] * RESOLUTION, DEC);
    Serial.print(F(", "));
    Serial.print(pulses[i+1][0] * RESOLUTION, DEC);
    Serial.print(F(", "));
  }
  Serial.print(pulses[currentpulse-1][1] * RESOLUTION, DEC);
  Serial.println(F(", 0 ],"));

  nodata = true;
}

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
