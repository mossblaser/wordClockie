#include <Arduino.h>
#include <SPI.h>

#include "word_clock.h"
#include "display.h"


////////////////////////////////////////////////////////////////////////////////
// Register addresses and values
////////////////////////////////////////////////////////////////////////////////

#define REG_NOP          0x0
#define REG_ROW(r)       ((r) + 1)
#define REG_DECODE_MODE  0x9
#define REG_INTENSITY    0xA
#define REG_SCAN_LIMIT   0xB
#define REG_SHUTDOWN     0xC
#define REG_TEST         0xF

#define NO_DECODE 0x00

#define SCAN_LIMIT(num_rows) ((num_rows) - 1)

#define SHUTDOWN 0x00
#define NORMAL   0x01

#define NO_TEST 0x00
#define TEST    0x01


////////////////////////////////////////////////////////////////////////////////
// Internal utility functions
////////////////////////////////////////////////////////////////////////////////

/**
 * Write the same value to the same register in all the displays.
 */
void write_all_reg(int reg, int value) {
	digitalWrite(nEN_PIN, LOW);
	for (int i = 0; i < (DISPLAYS_X*DISPLAYS_Y); i++) {
		SPI.transfer(reg);
		SPI.transfer(value);
	}
	digitalWrite(nEN_PIN, HIGH);
}


////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////

void display_begin(void) {
	// Set up nenable/load pin
	pinMode(nEN_PIN, OUTPUT);
	digitalWrite(nEN_PIN, HIGH);
	
	// Setup SPI
	SPI.begin();
	SPI.setDataMode(SPI_MODE0);
	SPI.setBitOrder(MSBFIRST);
	SPI.setClockDivider(SPI_CLOCK_DIV16);
	
	
	// Configure the displays (remain in shutdown mode during this process)
	write_all_reg(REG_SHUTDOWN, SHUTDOWN);
	write_all_reg(REG_TEST, NO_TEST);
	write_all_reg(REG_DECODE_MODE, NO_DECODE);
	write_all_reg(REG_INTENSITY, 0x0F);
	write_all_reg(REG_SCAN_LIMIT, SCAN_LIMIT(DISPLAY_HEIGHT));
	for (int row = 0; row < DISPLAY_HEIGHT; row++)
		write_all_reg(REG_ROW(row), 0x00);
	write_all_reg(REG_SHUTDOWN, NORMAL);
}


void display_buf(const char *buf, int global_intensity) {
	// Set display intensity
	write_all_reg(REG_INTENSITY, global_intensity);
	
	// Load frame from buffer
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
				SPI.transfer(REG_ROW(row));
				SPI.transfer(row_pixels);
			}
		}
		digitalWrite(nEN_PIN, HIGH);
	}
}
