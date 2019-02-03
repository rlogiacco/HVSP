// AVR High-voltage Serial Fuse Reprogrammer
// Adapted from code and design by Paul Willoughby 03/20/2010

// Fuse Calc:
//   http://www.engbedded.com/fusecalc/

#include "Firmware.h"
#define  RST     13    // HVRESET driver pin
#define  SCI     12    // Clock Input
#define  SDO     11    // Data Output
#define  SII     10    // Instruction Input
#define  SDI      9    // Data Input
#define  VCC      8    // VCC

#define  HFUSE  0x747C
#define  LFUSE  0x646C
#define  EFUSE  0x666E

// Define ATTiny series signatures
#define  ATTINY13   0x9007  // L: 0x6A, H: 0xFF             8 pin
#define  ATTINY24   0x910B  // L: 0x62, H: 0xDF, E: 0xFF   14 pin
#define  ATTINY25   0x9108  // L: 0x62, H: 0xDF, E: 0xFF    8 pin
#define  ATTINY44   0x9207  // L: 0x62, H: 0xDF, E: 0xFFF  14 pin
#define  ATTINY45   0x9206  // L: 0x62, H: 0xDF, E: 0xFF    8 pin
#define  ATTINY84   0x930C  // L: 0x62, H: 0xDF, E: 0xFFF  14 pin
#define  ATTINY85   0x930B  // L: 0x62, H: 0xDF, E: 0xFF    8 pin

void setup() {
	pinMode(VCC, OUTPUT);
	pinMode(RST, OUTPUT);
	pinMode(SDI, OUTPUT);
	pinMode(SII, OUTPUT);
	pinMode(SCI, OUTPUT);
	pinMode(SDO, OUTPUT);     // Configured as input when in programming mode
	digitalWrite(RST, HIGH);  // Turns off HVRESET
	Serial.begin(19200);
	Serial.println("Hit any key to start");
}

void loop() {
	if (Serial.available() > 0) {
		wdt_enable(WDTO_8S);
		Serial.println("### Watchdog enabled: if the chip is not responding the program will reset.");
		Serial.read();
		Serial.print("Chip reset initiated");
		pinMode(SDO, OUTPUT);     // SDO to output
		digitalWrite(SDI, LOW);
		digitalWrite(SII, LOW);
		digitalWrite(SDO, LOW);
		digitalWrite(RST, HIGH);  // 12V Off
		digitalWrite(VCC, HIGH);  // VCC On
		delayMicroseconds(20);
		digitalWrite(RST, LOW);   // 12V On
		delayMicroseconds(10);
		pinMode(SDO, INPUT);      // SDO to input
		delayMicroseconds(300);
		unsigned int sig = readSignature();
		Serial.print("Signature is: ");
		Serial.println(sig, HEX);
		Serial.print("Reading chip fuses... ");
		readFuses();
		if (sig == ATTINY13) {
			writeFuse(LFUSE, 0x6A);
			writeFuse(HFUSE, 0xFF);
		} else if (sig == ATTINY24 || sig == ATTINY44 || sig == ATTINY84
				|| sig == ATTINY25 || sig == ATTINY45 || sig == ATTINY85) {
			writeFuse(LFUSE, 0x62);
			writeFuse(HFUSE, 0xDF);
			writeFuse(EFUSE, 0xFF);
		}
		readFuses();
		Serial.print("Writing chip fuses... ");
		digitalWrite(SCI, LOW);
		digitalWrite(VCC, LOW);    // Vcc Off
		digitalWrite(RST, HIGH);   // 12v Off
		Serial.println("DONE!");
		wdt_reset();
	}
}

byte shiftOut(byte val1, byte val2) {
	int inBits = 0;
	// wait until SDO goes high
	while (!digitalRead(SDO));
	unsigned int dout = (unsigned int) val1 << 2;
	unsigned int iout = (unsigned int) val2 << 2;
	for (int ii = 10; ii >= 0; ii--) {
		digitalWrite(SDI, !!(dout & (1 << ii)));
		digitalWrite(SII, !!(iout & (1 << ii)));
		inBits <<= 1;
		inBits |= digitalRead(SDO);
		digitalWrite(SCI, HIGH);
		digitalWrite(SCI, LOW);
	}
	return inBits >> 2;
}

void writeFuse(unsigned int fuse, byte val) {
	shiftOut(0x40, 0x4C);
	shiftOut(val, 0x2C);
	shiftOut(0x00, (byte) (fuse >> 8));
	shiftOut(0x00, (byte) fuse);
}

void readFuses() {
	byte val;
	shiftOut(0x04, 0x4C);  // LFuse
	shiftOut(0x00, 0x68);
	val = shiftOut(0x00, 0x6C);
	Serial.print("LFuse: ");
	Serial.print(val, HEX);
	shiftOut(0x04, 0x4C);  // HFuse
	shiftOut(0x00, 0x7A);
	val = shiftOut(0x00, 0x7E);
	Serial.print(", HFuse: ");
	Serial.print(val, HEX);
	shiftOut(0x04, 0x4C);  // EFuse
	shiftOut(0x00, 0x6A);
	val = shiftOut(0x00, 0x6E);
	Serial.print(", EFuse: ");
	Serial.println(val, HEX);
}

unsigned int readSignature() {
	Serial.println("Reading chip signature...");
	unsigned int sig = 0;
	byte val;
	for (int ii = 1; ii < 3; ii++) {
		shiftOut(0x08, 0x4C);
		shiftOut(ii, 0x0C);
		shiftOut(0x00, 0x68);
		val = shiftOut(0x00, 0x6C);
		sig = (sig << 8) + val;
	}
	return sig;
}
