# IR decoder

Decodes long IR codes, for example from air conditioner / heat pump devices.

Shows the timings, the symbols, and also the decoded signal for certain air conditioners.

Required hardware:
- Arduino (any compatible will do, but Arduino Uno or Nano is easiest for prototyping)
- Infrared receiver, for example VS1838 will do fine, see also https://arduino-info.wikispaces.com/IR-RemoteControl
- Breadboard, wiring
- IR remote control from the aircon/heatpump you plan to decode

## Instructions

* Connect an IR receiver into the Arduino
* Start the sketch, and enter 1, 2, 3, 4 or 5 into the 'Serial Monitor', to select which timings to use
   * Try out the alternatives until you get sensible output
   * The signal should always start with 'Hh', and within the signal there should only be a couple of 'Hh' pairs (if any)
   * 'H' and 'h' should be there only in pairs 'Hh'
   * 'H' stands for 'header mark' and 'h' for 'header space'
* Point your IR remote to the IR receiver and send the code
   * If the symbols are known, then the decoder shows its meaning on the serial monitor
   * If the symbols are unknown, then you can help by writing a decoder for the unknown remote
   
-> Mode '9' can be used to decode known signals, in that case you can send the symbols from the terminal, like entering this:

   Hh001101011010111100000111001001010100000000000111000000001111111011010100000001000111001011

![Schema](arduino_irreceiver.png)
