/**
 * Functions for 
 */

#include <string.h>

#include "words.h"
#include "word_clock.h"

////////////////////////////////////////////////////////////////////////////////
// Character mask
////////////////////////////////////////////////////////////////////////////////

#undef ARDUINO
#ifdef ARDUINO
	#include <avr/pgmspace.h>
	typedef prog_char mask_t;
	#define WORDS_PROGMEM PROGMEM
	#define GET_MASK_CHAR(x,y) ((char)(pgm_read_byte_near(WORDS + ((y)*WIDTH) + (x))))
#else
	typedef char mask_t;
	#define WORDS_PROGMEM
	#define GET_MASK_CHAR(x,y) ((char)(WORDS[y][x]))
	#define F
#endif
#define ARDUINO

// Lookup from pixel coordinate to character on the display.
static const mask_t WORDS[HEIGHT][WIDTH] WORDS_PROGMEM = {
	{'f','o','r','i','t','s','c','u','b','e','t','h','a','n','d'},
	{'f','o','r','t','w','e','n','t','y','t','h','i','r','t','y'},
	{'o','n','e','l','e','v','e','n','i','n','e','t','e','e','n'},
	{'t','h','r','e','e','i','g','h','t','e','e','n','t','w','o'},
	{'s','e','v','e','n','t','e','e','n','t','w','e','l','v','e'},
	{'t','h','i','r','t','e','e','n','q','u','a','r','t','e','r'},
	{'f','o','u','r','t','e','e','n','s','i','x','t','e','e','n'},
	{'f','i','f','t','e','e','n','h','a','l','f','i','v','e','j'},
	{'t','e','n','d','e','c','a','d','e','s','p','a','s','t','o'},
	{'t','h','r','e','e','l','e','v','e','n','i','n','e','d','h'},
	{'o','n','e','i','g','h','t','w','o','t','w','e','l','v','e'},
	{'w','e','e','k','s','e','v','e','n','d','a','y','s','i','x'},
	{'m','o','n','t','h','s','y','e','a','r','s','f','o','u','r'},
	{'f','i','v','e','t','e','n','e','v','e','r','a','m','p','m'}
};


////////////////////////////////////////////////////////////////////////////////
// Letter mask functions
////////////////////////////////////////////////////////////////////////////////

/**
 * Set every element in the buffer to zero starting immediately following coordinate given and
 * working from right-to-left and bottom-to-top until the top left corner is
 * reached.
 */
static void clear_buffer_after(char buf[HEIGHT][WIDTH], int x, int y) {
	if (x != 0) {
		x--;
	} else {
		x = WIDTH-1;
		y--;
	}
	
	for (; y >= 0; y--) {
		for (; x >= 0; x--) {
			buf[y][x] = 0;
		}
		x = WIDTH-1;
	}
}


bool get_word_mask(char buf[HEIGHT][WIDTH], const char *str) {
	// Last char of current word being placed
	const char *cur_word = str + strlen(str) - 1;
	// Next char to be placed
	const char *cur_char = cur_word;
	
	// Scan backwards through the character array and string attempting to place
	// each word, back-tracking if a word cannot be placed.
	bool skip_next = false;
	for (int y = HEIGHT-1; y >= 0; y--) {
		for (int x = WIDTH-1; x >= 0; x--) {
			if (skip_next) {
				skip_next = false;
				buf[y][x] = 0;
				continue;
			}
			
			if (GET_MASK_CHAR(x,y) == *cur_char) {
				// Character found!
				buf[y][x] = 1;
				
				if (cur_char == str) {
					// The current character was the last
					clear_buffer_after(buf, x,y);
					return true;
				} else {
					// Not at the beginning yet, move onto next character.
					cur_char--;
				}
				
				if (*cur_char == ' ') {
					// Completed the a word, skip at least one character in the mask or
					// leave a line-break.
					if (x != 0)
						skip_next = true;
					
					// Advance to the next word
					if (cur_char != str && *cur_char == ' ')
						cur_char--;
					cur_word = cur_char;
					
					// Special case: the start of the string was whitespace which we've
					// now skipped over and thus we can now terminate successfuly.
					if (cur_char == str) {
						clear_buffer_after(buf, x,y);
						return true;
					}
				}
			} else if (cur_char != cur_word) {
				// The word was not matched after at least one character had matched
				// succssfully, unwind all characters in the word.
				if (x+1 < WIDTH) {
					x++;
				} else {
					y++;
					x=0;
				}
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
			} else {
				// The first character of the current word was not matched
				buf[y][x] = 0;
			}
		}
	}
	
	// If we got here, the whole string wasn't found
	return false;
}


////////////////////////////////////////////////////////////////////////////////
// String generation functions
////////////////////////////////////////////////////////////////////////////////


void words_append(char *str, const char *appendage) {
	str = str + strlen(str);
	strcpy(str, appendage);
}

void words_append_number(char *str, int number) {
	// Fail if out of bounds
	if (number < 1 or number > 39)
		return;
	
	int tens  = number / 10;
	int units = number % 10;
	
	bool has_tens_prefix;
	switch (tens) {
		case 2: words_append(str, F("twenty")); has_tens_prefix = true; break;
		case 3: words_append(str, F("thirty")); has_tens_prefix = true; break;
		default: has_tens_prefix = false; break;
	}
	
	if (has_tens_prefix && units != 0)
		words_append(str, " ");
	
	if (number < 10 || number >= 20) {
		switch (units) {
			case 0: break;
			case 1: words_append(str, F("one")); break;
			case 2: words_append(str, F("two")); break;
			case 3: words_append(str, F("three")); break;
			case 4: words_append(str, F("four")); break;
			case 5: words_append(str, F("five")); break;
			case 6: words_append(str, F("six")); break;
			case 7: words_append(str, F("seven")); break;
			case 8: words_append(str, F("eight")); break;
			case 9: words_append(str, F("nine")); break;
		}
	} else {
		switch (number) {
			case 10: words_append(str, F("ten")); break;
			case 11: words_append(str, F("eleven")); break;
			case 12: words_append(str, F("twelve")); break;
			case 13: words_append(str, F("thirteen")); break;
			case 14: words_append(str, F("fourteen")); break;
			case 15: words_append(str, F("fifteen")); break;
			case 16: words_append(str, F("sixteen")); break;
			case 17: words_append(str, F("seventeen")); break;
			case 18: words_append(str, F("eighteen")); break;
			case 19: words_append(str, F("nineteen")); break;
		}
	}
}

void words_append_time(char *str, int hours, int minutes) {
	int relative_minutes;
	int relative_hours;
	bool past;
	
	// Get minutes relative to hours
	if (minutes <= 30) {
		relative_minutes = minutes;
		relative_hours = hours;
		past = true;
	} else {
		relative_minutes = 60 - minutes;
		relative_hours = (hours + 1) % 24;
		past = false;
	}
	
	// Convert hours to 12-hour time
	bool am = relative_hours >= 12;
	relative_hours %= 12;
	if (relative_hours == 0)
		relative_hours = 12;
	
	// Render minutes
	switch (relative_minutes) {
		default: case 0:
			// Nothing
			break;
		
		case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10:
		case 11: case 12: case 13: case 14: case 16: case 17: case 18: case 19: case 20:
		case 21: case 22: case 23: case 24: case 25: case 26: case 27: case 28: case 29:
			words_append_number(str, relative_minutes);
			break;
		
		case 15:
			words_append(str, F("quarter"));
			break;
		
		case 30:
			words_append(str, F("half"));
			break;
	}
	
	// Render past/to
	if (relative_minutes != 0) {
		if (past)
			words_append(str, F(" past "));
		else
			words_append(str, F(" to "));
	}
	
	// Render hours
	words_append_number(str, relative_hours);
	
	// Render AM/PM
	if (am)
		words_append(str, F(" am"));
	else
		words_append(str, F(" pm"));
}
