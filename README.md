# Brutal Timer

This project is based on [A2Pico](https://github.com/oliverschmidt/a2pico).

This firmware does not __emulate__ a [Brutal Timer](https://github.com/antoinevignau/source/tree/main/brutaltimer) card. However, it can be used in place of this card in certain scenarios.

## Differences

* The Brutal Timer card has three time sources by default:
  * The Apple II Phi signal (~1 MHz)
  * The Apple II 7M signal (~7 MHz)
  * A 20 MHz crystal

  In contrast, this firmware only has a single 1 MHz time source. This has two consequences::
  * The 1 MHz timer cannot be used to count Apple II cycles.
  * The 7 MHz and 20 MHz timers only have an actual resolution of 1 MHz.

* The Brutal Timer card offers the option of connecting an 8-digit 7-segment display.

  In contrast, this firmware implements a USB CDC-ACM device. When connected to a PC, a virtual serial port is established. When opening that port with a terminal program, the same 8 digits are displayed. However, in certain situations, the digits are not updated as they are on the Brutal Timer display. The A2Pico LED indicates wether a terminal program has opened the port.
