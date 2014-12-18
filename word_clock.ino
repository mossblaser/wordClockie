/**
 * Firmware for a word clock for a special Cube wife.
 *
 * Hardware consists of an array of MAX7219/MAX7221 driven LED arrays, a
 * realtime clock and an LDR and tilt switches for I/O.
 */

#include <Time.h>
#include <DS1307RTC.h>
#include <SPI.h>
#include <Wire.h>

#include "word_clock.h"
#include "words.h"
#include "tween.h"


////////////////////////////////////////////////////////////////////////////////
// Pin Definitions
////////////////////////////////////////////////////////////////////////////////

// Load/nEnable pin for display
static const int nEN_PIN = 9;

// LDR power pins
static const int LDR_P_PIN = A0;
static const int LDR_N_PIN = A1;

// LDR sense pin (analogue pin)
static const int LDR_PIN = 2;

// Tilt switch pins
static const int TILT_LEFT_PIN = 2;
static const int TILT_RIGHT_PIN = 4;


////////////////////////////////////////////////////////////////////////////////
// Display control routines
////////////////////////////////////////////////////////////////////////////////

void write_all_reg(int reg, int value) {
	digitalWrite(nEN_PIN, LOW);
	for (int i = 0; i < (DISPLAYS_X*DISPLAYS_Y); i++) {
		SPI.transfer(reg);
		SPI.transfer(value);
	}
	digitalWrite(nEN_PIN, HIGH);
}


char buf[HEIGHT * WIDTH];


void setup_display(void) {
	// Power-down
	write_all_reg(0x0C, 0x00);
	
	// Leave test mode
	write_all_reg(0x0F, 0x00);
	
	// Don't decode as code-B (seven-seg)
	write_all_reg(0x09, 0x00);
	
	// Set intensity to full
	write_all_reg(0x0A, 0x0F);
	
	// Limit scan to the 7 rows
	write_all_reg(0x0B, 0x06);
	
	// Pre-blank the display
	for (int row = 0; row < DISPLAY_HEIGHT; row++)
		write_all_reg(row + 1, 0x00);
	
	// Come out of power-down
	write_all_reg(0x0C, 0x01);
}

void reinitialise(int intensity) {
	// Leave test mode
	write_all_reg(0x0F, 0x00);
	
	// Don't decode as code-B (seven-seg)
	write_all_reg(0x09, 0x00);
	
	// Set intensity to full
	write_all_reg(0x0A, intensity);
	
	// Limit scan to the 7 rows
	write_all_reg(0x0B, 0x06);
	
	// Come out of power-down
	write_all_reg(0x0C, 0x01);
}

void display(const char *buf) {
	for (int row = 0; row < DISPLAY_HEIGHT; row++) {
		digitalWrite(nEN_PIN, LOW);
		for (int display_y = 0; display_y < DISPLAYS_Y; display_y++) {
			for (int display_x = 0; display_x < DISPLAYS_X; display_x++) {
				unsigned char row_pixels = 0;
				for (int col = 0; col < DISPLAY_WIDTH; col++) {
					row_pixels <<= 1;
					row_pixels |= buf[ ((display_y*DISPLAY_HEIGHT) + row)*WIDTH
					                 + ((display_x*DISPLAY_WIDTH) + col)
					                 ];
				}
				// Pad out non-existing pixels
				row_pixels <<= 8 - DISPLAY_WIDTH;
				
				// Send to display
				SPI.transfer(row + 1);
				SPI.transfer(row_pixels);
			}
		}
		digitalWrite(nEN_PIN, HIGH);
	}
}

void setup() {
	Serial.begin(115200);
	
	SPI.begin();
	SPI.setClockDivider(SPI_CLOCK_DIV16);
	SPI.setDataMode(SPI_MODE0);
	SPI.setBitOrder(MSBFIRST);
	
	setSyncProvider(RTC.get);
	
	if(timeStatus()!= timeSet) 
		Serial.println("Unable to sync with the RTC");
	else
		Serial.println("RTC has set the system time");
	
	pinMode(nEN_PIN, OUTPUT);
	digitalWrite(nEN_PIN, HIGH);
	
	pinMode(2, INPUT); digitalWrite(2, HIGH);
	pinMode(4, INPUT); digitalWrite(4, HIGH);
	
	pinMode(A0, OUTPUT); digitalWrite(A0, HIGH);
	pinMode(A1, OUTPUT); digitalWrite(A1, LOW);
	
	for (int x = 0; x < WIDTH; x++)
		for (int y = 0; y < HEIGHT; y++)
			buf[y*WIDTH + x] = 0;
}

void loop() {
	static int mins = 0;
	static int hours = 0;
	
	// Pair of frame buffers to contain current time and previous time
	static char buf_a[HEIGHT * WIDTH];
	static char buf_b[HEIGHT * WIDTH];
	
	static char *prev_buf = buf_a;
	static char *cur_buf  = buf_b;
	
	// Swap buffers
	if (prev_buf == buf_a) {
		prev_buf = buf_b;
		cur_buf  = buf_a;
	} else {
		prev_buf = buf_a;
		cur_buf  = buf_b;
	}
	
	// Render the time into the current buffer
	char str[100] = {0};
	words_append_time(str, hours, mins);
	get_word_mask(cur_buf, str);
	
	// Animate the transition
	int intensity;
	const char *buf;
	tween_start(prev_buf, cur_buf, TWEEN_FADE, 300);
	while (tween_next(&buf, &intensity)) {
		reinitialise(intensity);
		display(buf);
	}
	
	if (++mins > 59)
		mins = 0;
	if (++hours > 23)
		hours = 0;
	
	delay(1000);
}


void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
