# IR decoder

Decodes long IR codes, for example from air conditioner / heat pump devices.

Shows the timings, the symbols, and also the decoded signal for certain air conditioners.


## Instructions

* Connect an IR receiver into the Arduino
* Start the sketch, and enter 1, 2 or 3 into the 'Serial Monitor', to select which timings to use
   * Try out the alternatives until you get sensible output
   * The signal should always start with 'Hh', and within the signal there should only be a couple of 'Hh' pairs (if any)
   * 'H' and 'h' should be there only in pairs 'Hh'
   * 'H' stands for 'header mark' and 'h' for 'header space'
* Point your IR remote to the IR receiver and send the code
* Mode '9' can be used to decode known signals, in that case you can send the symbols from the terminal, like entering this:

   Hh001101011010111100000111001001010100000000000111000000001111111011010100000001000111001011

![Schema](arduino_irreceiver.png)
