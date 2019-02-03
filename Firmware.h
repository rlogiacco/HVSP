// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _HVSP_H_
#define _HVSP_H_
#include "Arduino.h"
//add your includes for the project HVSP here
#include <avr/wdt.h>

//end of add your includes here


//add your function definitions for the project HVSP here
byte shiftOut(byte val1, byte val2);
void writeFuse(unsigned int fuse, byte val);
void readFuses();
unsigned int readSignature();



//Do not add code below this line
#endif /* _HVSP_H_ */
