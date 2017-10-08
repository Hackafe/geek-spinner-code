
/*

Not all Attiny cores support the tone() function. Try this one
https://github.com/SpenceKonde/ATTinyCore

// ATMEL ATTINY84 / ARDUINO
//
//                           +-\/-+
//                     VCC  1|    |14  GND
//             (D  0)  PB0  2|    |13  AREF (D 10)
//             (D  1)  PB1  3|    |12  PA1  (D  9) 
//                     PB3  4|    |11  PA2  (D  8) 
//  PWM  INT0  (D  2)  PB2  5|    |10  PA3  (D  7) 
//  PWM        (D  3)  PA7  6|    |9   PA4  (D  6) 
//  PWM        (D  4)  PA6  7|    |8   PA5  (D  5)        PWM

*/

#include <EEPROMex.h>
#include <avr/pgmspace.h>
#include "font.h"
#include "textAndShapes.h"

const int charHeight = 8;
const int charWidth = 5;
int rows= 8;		        // Total LED's in a row
int LEDS[] = {10, 9, 8, 7, 6, 5, 3, 4};

bool STATES[] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};

char charArray[6];              // holds characters to display
unsigned long lastTimeUs;                // time (us) magnet sensed
unsigned long revTimeUs;                 // time (us) of last full rotation
unsigned long dwellTimeUs;               // time (us) between LED changes (based on rotation speed)
volatile unsigned long revolutions = 0;  // track number of revolutions since last start
long totalRevolutions;          // track total number of revolutions (stored in EEPROM)
bool spinning = true;          // track reset of time & counts
unsigned long startTimeUs;               // time (us) current spinning started
  
int mode;                       // current operating mode, stored in EEPROM
int modes = 8;                  // number of modes available
  // 0 -> text "Hello World!"
  // 1 -> RPM
  // 2 -> time in seconds
  // 3 -> spin count
  // 4 -> spin count (total)
  // 5 -> "lilly pad" pattern
  // 6 -> shape 1 (heart)
  // 7 -> shape 2 (smile)
  

byte ledPin = 0; 
byte magPin = 0; // Hall effect sensor, 
byte magPullUp = 0; // Pin A0 / D0
byte touchPin = 1; // Push button A1 / D1
byte touchPullUp = 1;

volatile boolean rotationFlag = false;  // modified by ISR


void setup(){
  // setup inputs
  pinMode(touchPin, INPUT);
  digitalWrite(touchPullUp, HIGH);
  pinMode(magPin, INPUT_PULLUP);
  digitalWrite(magPullUp, HIGH);
  
  // setup other LEDs
  for(int LED=0; LED<(sizeof(LEDS)/sizeof(int)); LED++){
    pinMode(LEDS[LED], OUTPUT);
    digitalWrite(LEDS[LED], STATES[LED]);
  }

  // Interupt setting
  GIMSK  = bit (PCIE0);  // Pin change interrupt enable
  PCMSK0 = bit (PCINT0); // Enable interupt on PCINT0 (D10)
  sei();                 // enables interrupts

  // get saved mode from eeprom
  mode = EEPROM.read(0);
  if (mode >= modes) {
    mode = 0; // may be very large first time
  }
  
  // get saved revolution total from eeprom
  totalRevolutions = EEPROMReadlong(1);
  if (totalRevolutions < 0 || totalRevolutions > 999999UL){ // will be -1 first time.
    totalRevolutions = 0;
    EEPROMWritelong(1, 0);
  }

  // show we are alive and what mode is active
  blipLEDs();

  lastTimeUs = micros();
}


void loop(){
  int sigFigs = 6;               // number of significant figures to deplay
  unsigned long curTimeUs;
  int seconds;
  unsigned int rpm;
  
  checkButton();
  if (((micros() - lastTimeUs) > 1000000UL)){ // less than 1 rev / sec
    if (spinning){
      spinning = false;
      totalRevolutions = totalRevolutions + revolutions;
      EEPROMWritelong(1, totalRevolutions);
      revolutions = 0;
      clearArray();
      blipLEDs();
    }
    else {
      //digitalWrite(LEDS[mode], LOW); 
    }
  }

  if (mode == 5){  // lilly pad pattern, show regardles of magnet.
    for(int LED=0; LED<(sizeof(LEDS)/sizeof(int)); LED++){
       digitalWrite(LEDS[LED], HIGH); 
       delay(1); 
       digitalWrite(LEDS[LED], LOW); 
    } 
    for(int LED=(sizeof(LEDS)/sizeof(int))-1; LED >= 0; LED--){
       digitalWrite(LEDS[LED], HIGH); 
       delay(1); 
       digitalWrite(LEDS[LED], LOW); 
    }     
  }

  else if (rotationFlag){ // we are spinning!
    rotationFlag = false;
    if (!spinning){
      spinning = true;
      startTimeUs = micros(); 
    }
    curTimeUs = micros();
    revTimeUs = curTimeUs - lastTimeUs;
    dwellTimeUs = revTimeUs * 3UL / 360UL;   // 3 degrees
    seconds = (curTimeUs - startTimeUs) / 1000000UL;
    rpm = 60000 * 1000 / revTimeUs;
    lastTimeUs = curTimeUs;
    clearArray();    
    if (mode == 0){
      strcpy (charArray, text);
      //sprintf(charArray, "%lu", dwellTimeUs);
    }
    else if (mode == 1){
      sprintf(charArray, "%d", rpm);
      sigFigs = 2;
    }
    else if (mode == 2){
      sprintf(charArray, "%d", seconds);
    }   
    else if (mode == 3){   
      sprintf(charArray, "%lu", revolutions);
    }
    else if (mode == 4){   
      sprintf(charArray, "%lu", totalRevolutions + revolutions);
    }
    else if (mode == 6){ // shape 1 (heart)
      for(int k=0; k< sizeof(shape_1);k++){
        if (rotationFlag){
          break;
        }
        char b = pgm_read_byte_near(&(shape_1[k]));
        for (int j=0; j<charHeight; j++) {
          digitalWrite(LEDS[j], bitRead(b, 7-j));
        }
        dwellTimeUs = revTimeUs * 4UL / 360UL;   // 5 degrees
        delayMicroseconds(dwellTimeUs);
      }
    }    
    else if (mode == 7){ // shape 2 (smile)
      for(int k=0; k< sizeof(shape_2);k++){
        if (rotationFlag){
          break;
        }
        char b = pgm_read_byte_near(&(shape_2[k]));
        for (int j=0; j<charHeight; j++) {
          digitalWrite(LEDS[j], bitRead(b, 7-j));
        }
        dwellTimeUs = revTimeUs * 4UL / 360UL;   // 5 degrees
        delayMicroseconds(dwellTimeUs);
      }
    }

    // Text in top half
    if (mode < 5) { 
      int digits = 0;    
      for(int k=0; k< sizeof(charArray);k++){
        char c = charArray[k];
        if (rotationFlag){
          break;
        }
        if(c){
          if (digits < sigFigs){
            printLetter(c);
            //digits += 1;
          }
          else{
            printLetter('0');
          }
          digits += 1;
        }
      }
      
      // Handle display in lower section
      clearArray();
      if(1 && (revTimeUs < 200000)){
        char * ptr = (char *) pgm_read_word (&string_table[mode]);
        //char buffer [6]; // must be large enough!
        strcpy_P (charArray, ptr);
  
        // wait for it . . .
        while((micros() < (lastTimeUs + revTimeUs / 2)) && !rotationFlag){};
        
        // show it
        for (int k=sizeof(charArray)-1; k>=0; k--){
          if (rotationFlag){
            break;
          }
          printLetterLower(charArray[k]);;
        }
      }
    }
  }  
}


ISR(PCINT0_vect){  // Magnet sensed
  if (!digitalRead(magPullUp)){
    rotationFlag = true;             // Increment volatile variables
    revolutions += 1;
  }
}


void dwellDelay(){ // avoid glitch on first rotation having erronious value 
  if (dwellTimeUs > 2000){
    dwellTimeUs = 2000;
  }
  if (dwellTimeUs < 100){
    dwellTimeUs = 100;
  }  
  delayMicroseconds(dwellTimeUs);
}


void printLetter(char ch){
// https://github.com/reger-men/Arduion-POV-clock/blob/master/clock.ino
  // make sure the character is within the alphabet bounds (defined by the font.h file)
  // if it's not, make it a blank character
  if (ch < 32 || ch > 126){
    ch = 32;
    }
  // subtract the space character (converts the ASCII number to the font index number)
  ch -= 32;
  // step through each byte of the character array
  for (int i=0; i<charWidth; i++) {
    char b = pgm_read_byte_near(&(font[ch][i]));
    
    for (int j=0; j<charHeight; j++) {
      digitalWrite(LEDS[j], bitRead(b, 7-j));
    }
    dwellDelay();
  }
  
  //clear the LEDs
  for (int i = 0; i < rows; i++)
    digitalWrite(LEDS[i] , LOW);
  dwellDelay();
}


void printLetterLower(char ch){
  // make sure the character is within the alphabet bounds (defined by the font.h file)
  // if it's not, make it a blank character
  if (ch < 32 || ch > 126){
    ch = 32;
    }
  // subtract the space character (converts the ASCII number to the font index number)
  ch -= 32;
  // step through each byte of the character array
  for (int i=charWidth-1; i>-1; i--) {
    char b = pgm_read_byte_near(&(font[ch][i]));
    for (int j=0; j<charHeight; j++) {
      digitalWrite(LEDS[j+1], bitRead(b,j));
    }
    dwellDelay();
  }
  //clear the LEDs
  for (int i = 0; i < rows; i++)
    digitalWrite(LEDS[i] , LOW);

  // space between letters
  dwellDelay();
}  


bool touched(){
  // returns true if touched, false if not.  Light LED until touch released
  bool touchVal = digitalRead(touchPullUp);
  if (!touchVal){
    while(!digitalRead(touchPullUp)){ // wait till touch release
      delay(10);
      digitalWrite(LEDS[mode], LOW);
    }
    //digitalWrite(LEDS[0], LOW);
    return (true);
  }
  else{
    return (false);
  }
}


void checkButton(){
  // check button for mode change and display current mode
  if (touched()){
    mode += 1;
    if (mode >= modes){
      mode = 0;
    }
    EEPROM.write(0, mode);
    blipLEDs();
  }
}


void blipLEDs(){
  // something to show we are alive
  for(int LED=0; LED<(sizeof(LEDS)/sizeof(int)); LED++){
    digitalWrite(LEDS[LED], HIGH); 
    delay(10); 
    digitalWrite(LEDS[LED], LOW); 
  } 
  for(int LED=sizeof(LEDS)/sizeof(int); LED>mode; LED--){
    digitalWrite(LEDS[LED], HIGH); 
    delay(10); 
    digitalWrite(LEDS[LED], LOW); 
  } 
  digitalWrite(LEDS[mode], HIGH);
}



void EEPROMWritelong(int address, long value)  {
  //This function will write a 4 byte (32bit) long to the eeprom at
  //the specified address to address + 3.
  //https://playground.arduino.cc/Code/EEPROMReadWriteLong

  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}


long EEPROMReadlong(long address){
  //Read the 4 bytes from the eeprom memory.
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);

  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}


void clearArray(){
  for(int i=0; i<sizeof(charArray); i++){      // clear array
    charArray[i] = 0;
  }
}
