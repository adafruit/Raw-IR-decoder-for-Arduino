# Raw-IR-decoder-for-Arduino
Take raw IR signal from a remote receiver and print out pulse lengths
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
 
