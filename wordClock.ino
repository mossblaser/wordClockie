#include <Time.h>
#include <DS1307RTC.h>
#include <SPI.h>
#include <Wire.h>

#define DISPLAYS_X 3
#define DISPLAYS_Y 2
#define DISPLAY_WIDTH 5
#define DISPLAY_HEIGHT 7

#define WIDTH  (DISPLAYS_X * DISPLAY_WIDTH)
#define HEIGHT (DISPLAYS_Y * DISPLAY_HEIGHT)


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

void write_all_reg(int reg, int value) {
	digitalWrite(nEN_PIN, LOW);
	for (int i = 0; i < (DISPLAYS_X*DISPLAYS_Y); i++) {
		SPI.transfer(reg);
		SPI.transfer(value);
	}
	digitalWrite(nEN_PIN, HIGH);
}

static const char WORDS[HEIGHT][WIDTH] = {
	{'F','O','R','I','T','S','C','U','B','E','T','H','A','N','D'},
	{'F','O','R','T','W','E','N','T','Y','T','H','I','R','T','Y'},
	{'O','N','E','L','E','V','E','N','I','N','E','T','E','E','N'},
	{'T','H','R','E','E','I','G','H','T','E','E','N','T','W','O'},
	{'S','E','V','E','N','T','E','E','N','T','W','E','L','V','E'},
	{'T','H','I','R','T','E','E','N','Q','U','A','R','T','E','R'},
	{'F','O','U','R','T','E','E','N','S','I','X','T','E','E','N'},
	{'F','I','F','T','E','E','N','H','A','L','F','I','V','E','J'},
	{'T','E','N','D','E','C','A','D','E','S','P','A','S','T','O'},
	{'T','H','R','E','E','L','E','V','E','N','I','N','E','D','H'},
	{'O','N','E','I','G','H','T','W','O','T','W','E','L','V','E'},
	{'W','E','E','K','S','E','V','E','N','D','A','Y','S','I','X'},
	{'M','O','N','T','H','S','Y','E','A','R','S','F','O','U','R'},
	{'F','I','V','E','T','E','N','E','V','E','R','A','M','P','M'}
};

unsigned char buf[HEIGHT][WIDTH];


bool string_to_buf(const char *str) {
	// Last char of current word being placed
	const char *cur_word = str + strlen(str) - 1;
	// Next char to be placed
	const char *cur_char = cur_word;
	
	bool skip_next = false;
	for (int y = HEIGHT-1; y >= 0; y--) {
		for (int x = WIDTH-1; x >= 0; x--) {
			if (skip_next) {
				skip_next = false;
				continue;
			}
			
			if (WORDS[y][x] == *cur_char) {
				// Character found!
				buf[y][x] = 1;
				// Stop if this is the end of the string
				if (cur_char == str) {
					// Completed the string, clear rest of buffer
					//if (x != 0) {
					//	x--;
					//} else {
					//	x = WIDTH-1;
					//	y--;
					//}
					//for (int y = HEIGHT-1; y >= 0; y--)
					//	for (int x = WIDTH-1; x >= 0; x--)
					//		buf[y][x] = 0;
					return true;
				} 
				
				// Continue if not the end of the string
				cur_char--;
				if (*cur_char == ' ') {
					// Completed the word
					// Skip the next character if not at the start of a line (where we
					// need not leave a space)
					if (x != 0)
						skip_next = true;
					cur_char--;
					cur_word = cur_char;
					// Sanity check: should not start sequence with a space nor feature
					// multiple spaces in a row.
					if (cur_char == str || *cur_char == ' ')
						return false;
				}
			} else if (cur_char != cur_word) {
				// The word was not matched after at least one character, unwind all characters in the word
				// Move back to previous char
				if (x+1 < WIDTH) {
					x++;
				} else {
					y++;
					x=0;
				}
				// Clear all characters
				for (; y < HEIGHT; y++) {
					for (; x < WIDTH; x++) {
						cur_char++;
						buf[y][x] = 0;
						if (cur_char == cur_word)
							break;
					}
					if (cur_char == cur_word)
						break;
				}
				// Skip over the original misleading character
				skip_next == true;
			} else {
				// The first character of the current word was not matched
				buf[y][x] = 0;
			}
		}
	}
	
	// If we got here, the whole string wasn't found
	return false;
}

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

void reinitialise(void) {
	// Leave test mode
	write_all_reg(0x0F, 0x00);
	
	// Don't decode as code-B (seven-seg)
	write_all_reg(0x09, 0x00);
	
	// Set intensity to full
	write_all_reg(0x0A, 0x0F);
	
	// Limit scan to the 7 rows
	write_all_reg(0x0B, 0x06);
	
	// Come out of power-down
	write_all_reg(0x0C, 0x01);
}

void flip(void) {
	for (int row = 0; row < DISPLAY_HEIGHT; row++) {
		digitalWrite(nEN_PIN, LOW);
		for (int display_y = 0; display_y < DISPLAYS_Y; display_y++) {
			for (int display_x = 0; display_x < DISPLAYS_X; display_x++) {
				unsigned char row_pixels = 0;
				for (int col = 0; col < DISPLAY_WIDTH; col++) {
					row_pixels <<= 1;
					row_pixels |= buf[(display_y*DISPLAY_HEIGHT) + row]
					                 [(display_x*DISPLAY_WIDTH) + col];
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
			buf[y][x] = 0;
}

void loop() {
	static int count = 0;
	string_to_buf("CUBETHAN FOR ONE YEAR");
	
	reinitialise();
	flip();
	
	//digitalClockDisplay();
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
