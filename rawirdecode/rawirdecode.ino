/* Raw IR decoder sketch!
 https://github.com/adafruit/Raw-IR-decoder-for-Arduino/blob/master/rawirdecode/rawirdecodestruct.ino
 This sketch/program uses the Arduno and a PNA4602 to 
 decode IR received. This can be used to make a IR receiver
 (by looking for a particular code)
 or transmitter (by pulsing an IR LED at ~38KHz for the
 durations detected )
 
 This code is blocking, it more or less polls the status of the IR detector 
 pin and then records state changes.
 
 Code is public domain, check out www.ladyada.net and adafruit.com
 for more tutorials! 
 
 

 /*
Some Updates by http://www.instructables.com/member/russ_hensel/
     add #define for a bunch of user options see section below with
           #defines
     add ability to easily change ir pin -- seems not working
     break into more subroutines, more or less for convenience
     added check for buffer overrun
     added/revised output formats of the ir data 
     added/revised some comments 
     Tested and compiles under Window 7, Arduino1.6.0, UNO
         Uses  3.7    K bytes of program
               1.2    K bytes of dynamic memory
               
There are other programs for IR decoding, perhaps most notably
http://www.righto.com/2009/08/multi-protocol-infrared-remote-library.html
That code is interrupt driven using timers and may in principal be capable
of being more accurate, but its normal resolution is 50 usec, so it may
in practice be better or not.

What the code does have is a sophisticated method of interpreting the results
which allows the representation of the code in just a byte or so ( if the
protocol is successfully identified ).  It also has nice transmit routines.
However, for raw discovery of the protocol this code may be more useful.
               
 */
 
 

// We need to use the 'raw' pin reading methods
// because timing is very important here and the digitalRead()
// procedure is slower!
//uint8_t IRpin = 2;
// Digital pin #2 is the same as Pin D2 see
// http://arduino.cc/en/Hacking/PinMapping168 for the 'raw' pin mapping


// ============================= easy modification using #define 's ================================

// redefinition: keep updating to be sure what version you are running 
#define VERSION     "2015 03 07.1"

// redefinition: in many cases no reason not to go faster, use a standard rate
// including 19200 .......
//#define  BAUDRATE  9600
//#define  BAUDRATE  19200
#define  BAUDRATE  38400

// seems to be defined as 7 somewhere, involved in reading pin 
// redefinition: ??
#define IRpin_PIN      PIND

// pin used for ir, digital, ....
// redefinition: I have not had luck redefining this one, but seems it should work 
#define IRpin          2           

// the maximum pulse we'll listen for - 65 milliseconds is a long time
// redefinition: increase to wait longer but may hit a limit at 64 binary k, check
#define MAXPULSE 65000

// what our timing resolution is, larger is better
// as its more 'precise' - but too large and you won't get accurate timing
// redefinition: unsure what is best, there is a software delay each time this expires, adds to error
// but you can expect an error of about 1/2 this value in each measurement
// may want to experiment.
#define RESOLUTION 20 

// Max number of measurements we can store 
// redefinition: more is better, but we can run out of memory -- look and see what your ir uses
// I found one control that put out 120 pulses no sweat.  beware of holding transmitter key down
// we do have a post measument overrun check, watch for it.
#define MAX_MEASUREMENTS  150

// used to turn on or off a given print format
// redefinition:  set to true or false to control print out of given format
#define PRINT_AS_PY_ARRAY    true
#define PRINT_AS_CODE        false
#define PRINT_TIMES_COLUMN   false
#define PRINT_TIMES          false
#define PRINT_AS_CPP_ARRAY   true


// used in formatting python code output, see subroutine below
// redefinition: if you want more or fewer values per line
#define PY_VAL_PER_LINE      10

// redefinition: used to seperate output sections, change if you want something different
#define SEPERATOR   "---------------------------"

#define CODE_COMMENT  "ir signal recieved had following Low High microsecond values"

// ============================= end #define section ================================


// these next variables will be global 
// we will store up to MAX_MEASUREMENTS pulse pairs (100 is -a lot- but why not use the memory you have ) -- note we check for overrun
uint16_t   pulses[MAX_MEASUREMENTS][2];  // pair is high and low pulse 
uint8_t    currentpulse = 0; // index for pulses we're storing

// ----------------------------------------------------------------

void printHelp( void ) {
  // just a little intro for the program, not really help
  Serial.println( "" ); 
  Serial.println( SEPERATOR );
  Serial.print( "Program: rawirladyada  version = " );
  Serial.println( VERSION );
  Serial.println( PIND );
  Serial.println( SEPERATOR );
  Serial.println( "" );
  return;
}

// ----------------------------------------------------------------  
// print a little summary of the capture,
// check for buffer overrun.
void printSummary(void) {
  Serial.println( "" ); 
  Serial.println( SEPERATOR );
  // Serial.println( "" );
  Serial.print("Total High Low Pulses = ");
  Serial.println( currentpulse, DEC );
  Serial.print( "Sampling period ( usec ) = ");
  Serial.println( RESOLUTION, DEC ); 
  
  if ( currentpulse > MAX_MEASUREMENTS ) {
      Serial.print( "Error, pulses exceeded max array size = " );
      Serial.println( MAX_MEASUREMENTS, DEC );
  }
  return; 
} 

// ----------------------------------------------------------------  
// print as Python code that can be coppied into a Python program
// to defined the time delay values of the ir signal
void printAsPyArray( void ){
  uint8_t i_col;        // index to column
  uint8_t i_col_max;    // index to column maximum
  
  // formatting defines
  #define PY_LINE_CONTINUE   "  \x5c"
  #define PY_LINE_INDENT     "       "
  
  i_col_max  = PY_VAL_PER_LINE;
  // print it in as a python array definition
  Serial.println( "" ); 
  Serial.println( SEPERATOR );
  Serial.print("# " );
  Serial.println( CODE_COMMENT );
  Serial.print("ir_signal  = [ " );
  
  i_col = 0;
  for ( uint8_t i = 0; i < currentpulse-1; i++) {
      
      Serial.print(pulses[i][1]   * RESOLUTION , DEC);
      Serial.print(", ");
  
      Serial.print(pulses[i+1][0] * RESOLUTION , DEC);
      Serial.print(", ");
      
      i_col ++;
      if ( i_col >= i_col_max ) {
          Serial.println( PY_LINE_CONTINUE );
          Serial.print( PY_LINE_INDENT );
          i_col = 0;
      }
  }
  
  Serial.println(  PY_LINE_CONTINUE );   //   \x5c  is a backslash 
  Serial.print( PY_LINE_INDENT );
  Serial.print(pulses[currentpulse-1][1] * RESOLUTION, DEC);
  Serial.print( " ]" );
  Serial.println( "" );
}

// ----------------------------------------------------------------  
// print as C++ code that can be coppied into a C++ program
// to defined the time delay values of the ir signal
void printAsCppArray( void ){
  uint8_t i_col;        // index to column
  uint8_t i_col_max;    // index to column maximum
  
  // formatting defines
  #define CPP_LINE_CONTINUE   ""
  #define CPP_LINE_INDENT     "       "
  
  i_col_max  = PY_VAL_PER_LINE;
  // print it in as a python array definition
  Serial.println( "" ); 
  Serial.println( SEPERATOR );
  Serial.print( "//" );
  Serial.println( CODE_COMMENT );
  Serial.print( "int IRsignal[] = {");
  i_col = 0;
  for ( uint8_t i = 0; i < currentpulse-1; i++) {
      
      Serial.print(pulses[i][1]   * RESOLUTION , DEC);
      Serial.print(", ");
  
      Serial.print(pulses[i+1][0] * RESOLUTION , DEC);
      Serial.print(", ");
      
      i_col ++;
      if ( i_col >= i_col_max ) {
          Serial.println( CPP_LINE_CONTINUE );
          Serial.print( CPP_LINE_INDENT );
          i_col = 0;
      }
  }
  
  Serial.println( CPP_LINE_CONTINUE );
  Serial.print( CPP_LINE_INDENT );
  Serial.print(pulses[currentpulse-1][1] * RESOLUTION, DEC);
  Serial.println( " };" );
}
  
// ----------------------------------------------------------------
// print as c++ arduino code using subroutines
void printAsCode( void ){

  Serial.println( "" ); 
  Serial.println( SEPERATOR );
  // Serial.println("int IRsignal[] = {");
  Serial.println("// ON, OFF ");
  for (uint8_t i = 0; i < currentpulse-1; i++) {
    //Serial.print("\t"); // tab
    Serial.print("pulseIR(");
    Serial.print(pulses[i][1] * RESOLUTION , DEC);
    Serial.print(");");
    Serial.println("");
    //Serial.print("\t");
    Serial.print("delayMicroseconds(");
    Serial.print(pulses[i+1][0] * RESOLUTION , DEC);
    Serial.println(");");

  }
  //Serial.print("\t"); // tab
  Serial.print("pulseIR(");
  Serial.print(pulses[currentpulse-1][1] * RESOLUTION, DEC);
  Serial.print(");");

}  
  
// ----------------------------------------------------------------
// print time all in a single column, may be most useful for
// pasting into a spreadsheet
void printTimesColumn(void) {
  Serial.println( "" ); 
  Serial.println( SEPERATOR );
  Serial.println("Alternate values High Low");
  for (uint8_t i = 0; i < currentpulse; i++) {
    Serial.println(pulses[i][0] * RESOLUTION, DEC);
    Serial.println(pulses[i][1] * RESOLUTION, DEC);
  }
}
  
// ----------------------------------------------------------------
// print the times, one high one low on each line with units 
void printTimes(void) {
  Serial.println( "" ); 
  Serial.println( SEPERATOR );
  Serial.println("\n\r\n\rReceived: \n\rOFF \tON");
  for (uint8_t i = 0; i < currentpulse; i++) {
    Serial.print(pulses[i][0] * RESOLUTION, DEC);
    Serial.print(" usec, ");
    Serial.print(pulses[i][1] * RESOLUTION, DEC);
    Serial.println(" usec");
  }
}

// ----------------------------------------------------------------
// print signal in selected formats see individual routines for
// a brief description 
// print all really prints all that have their #defines set to true
void printAll(void) {
    printSummary( );
    
    if ( PRINT_TIMES_COLUMN ) {    
         printTimesColumn( );
    }
    if ( PRINT_TIMES ) {  
        printTimes();
    }
    if ( PRINT_AS_CODE ) {
        printAsCode();
    }
    if ( PRINT_AS_PY_ARRAY ) {    
        printAsPyArray();
    }
    if ( PRINT_AS_CPP_ARRAY ) {      
        printAsCppArray();
    }
}

// ----------------------------------------------
// routine that measures and store the ir code values
// works via polling, no adjustment for time that code runs
// which is a systematic error, we could adjust ... need to look into it 
void getIr( void ) {

  uint16_t highpulse, lowpulse;  // temporary storage timing
  highpulse = lowpulse = 0;      // start out with no pulse length
  
  currentpulse  = 0;
  while ( true ) {
      highpulse     = 0;
      lowpulse      = 0;
      
      // we loop here after a high and low, then need to continue
      while (IRpin_PIN & (1 << IRpin)) {
           // pin is still HIGH
      
           // count off another few microseconds
           highpulse++;
           delayMicroseconds(RESOLUTION);
      
           // If the pulse is too long, we 'timed out' - either nothing
           // was received or the code is finished, so print what
           // we've grabbed so far, and then reset
           if ((highpulse >= MAXPULSE) && (currentpulse != 0)) {
             //printpulses();
             //currentpulse=0;
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
             // printpulses();
             // currentpulse=0;
             return;
           }
        }
        pulses[currentpulse][1] = lowpulse;

        // we read one high-low pulse successfully, continue!
        currentpulse++;
  }
}

// ----------------------------------------------
// setup, set serial speed, print help/welcome message
void setup(void) {
  Serial.begin( BAUDRATE );
  printHelp(); 
}

// ----------------------------------------------

void loop(void) {
    Serial.println("");
    Serial.println( SEPERATOR );
    Serial.println("Ready to decode IR2...");
    getIr();
    printAll();
}
// =========================== eof =============================





