Geek spinner - POV spinner with ATTiny84
============

### Introduction 
Source code for the geek spinner board from the Hackafe geek spinner workshop.

### Prerequisites

* Install [ATTiny Core Support](https://github.com/SpenceKonde/ATTinyCore) 
* Install [EEPROMex Library](https://github.com/thijse/Arduino-EEPROMEx)

### Preparing for uploading

* Pick up a spare Arduino UNO from your desk
* Program it with the *Arduino ISP* sketch from the IDE menu *File->Examples->ArduinoISP->ArduinoISP*

### Compiling and Uploading

## Compiling 

* [Download ZIP archive](https://github.com/Hackafe/geek-spinner-code/archive/master.zip) or clone this repository.
* Open the *SMD_spinner.ino* file
* From the arduino IDE select the tools menu  *Tools->Board* and scroll down and find *ATtiny 24/44/84* 
* Then again click on Tools menu and select *ATtiny84* from the *Chip* option
* Click on *Tools* and then select *Clockwise* from the *Pin mapping* option
* Select the oscillator speed *8Mhz Internal* from  *Tools->Clock*
* Click on *Verify* to verify that the sketch compiles without errors

## Uploading

* Pick up the Arduino UNO you programmed earlier and wire it with the *ATtiny84* chip from the spinner from [this diagram](http://42bots.com/wp-content/uploads/2014/01/programming-attiny44-attiny84-with-arduino-uno.png) 

* From *Tools->Programmer* select *Arduino as ISP*
* From *Tools->Port* select the port of the Arduino UNO connected to your PC
* Hit the upload button from the Arduino IDE. The upload process to the ATtiny84 should take 10 seconds.

## Changing

* You can change the text and shapes of your spinner from the *textAndShapes.h* header file


### Notes

* You can use any Arduino board to program the ATtiny84, just google for *Arduino [UNO/MEGA] etc.. as a programmer*
* If you have any trouble open an issue in this repository

Acknowledgements
-----------------

Original design and code from [Makers Box hackerspace](http://www.makersbox.us/). Link to [Tindie Product](https://www.tindie.com/products/MakersBox/programmable-pov-fidget-spinner/)
