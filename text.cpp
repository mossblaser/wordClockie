#include "word_clock.h"
#include "text.h"

////////////////////////////////////////////////////////////////////////////////
// Font selection
////////////////////////////////////////////////////////////////////////////////

// Fonts can be generated using:
//
//   python font_gen.py 14 16 "Myriad Pro" FONT_ > font.h
//                      |  |   |           |       |
//                      |  |   |           |       '--- Output filename
//                      |  |   |           '-- Prefix used in the code
//                      |  |   |               (must be FONT_)
//                      |  |   '-- Font name
//                      |  '-- Number of LEDs high (rounded to multiple of 8)
//                      '-- Height of font (0 - 40)

#include "font.h"

////////////////////////////////////////////////////////////////////////////////
// Internal state
////////////////////////////////////////////////////////////////////////////////

static struct {
	// The string being displayed
	const char *str;
	
	// The character currently being rendered
	const char *this_char;
	
	// The pixel column within the current character being displayed
	unsigned int this_col;
	
	// The pixel column within the next character being displayed
	unsigned int next_col;
	
	// Number of blank pixels printed at the end of the message
	unsigned int blank_pixels;
} state;


////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////

void text_start(const char *str) {
	state.str = str;
	state.this_char = str;
	state.this_col = 0;
	state.next_col = 0;
	state.blank_pixels = 0;
}


bool text_next(char *buf) {
	// Shift everything in the buffer one pixel to the left leaving a column of
	// zeros on the right-hand-side.
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH-1; x++) {
			buf[(y*WIDTH) + x] = buf[(y*WIDTH) + x + 1];
		}
		buf[(y*WIDTH) + WIDTH - 1] = 0;
	}
	
	// If we've reached the end of the string, generate blank pixels to fill the
	// remainder before finishing.
	if (*(state.this_char) == '\0')
		return state.blank_pixels++ < WIDTH;
	
	// Get the character index in the font (default to space if character is not
	// available)
	unsigned int this_index = pgm_read_byte_near(FONT_ASCII_TO_INDEX + state.this_char[0]);
	unsigned int next_index = pgm_read_byte_near(FONT_ASCII_TO_INDEX + state.this_char[1]);
	if (this_index == 0xFF) this_index = pgm_read_byte_near(FONT_ASCII_TO_INDEX + ' ');
	if (next_index == 0xFF) next_index = pgm_read_byte_near(FONT_ASCII_TO_INDEX + ' ');
	
	// Get the start address of the bitmap for this character
	unsigned int this_bitmap = pgm_read_word_near(FONT_GLYPH_BITMAPS_LOOKUP + this_index);
	unsigned int next_bitmap = pgm_read_word_near(FONT_GLYPH_BITMAPS_LOOKUP + next_index);
	
	unsigned int this_width = pgm_read_byte_near(FONT_GLYPH_WIDTH + this_index);
	unsigned int this_end   = pgm_read_byte_near(FONT_GLYPH_END   + this_index);
	unsigned int next_start = pgm_read_byte_near(FONT_GLYPH_START + next_index);
	unsigned int next_end   = pgm_read_byte_near(FONT_GLYPH_END   + next_index);
	
	// Display current the column
	for (int row_byte = 0; row_byte < FONT_HEIGHT/8; row_byte++) {
		// The current character's line
		unsigned char c = pgm_read_byte_near( FONT_GLYPH_BITMAPS
		                                    + this_bitmap
		                                    + (state.this_col * (FONT_HEIGHT/8))
		                                    + row_byte
		                                    );
		
		// Overlaid with the next character's overlapping columns
		if (state.this_col >= this_end - next_start && state.next_col < next_end)
			c |= pgm_read_byte_near( FONT_GLYPH_BITMAPS
			                       + next_bitmap
			                       + (state.next_col * (FONT_HEIGHT/8))
			                       + row_byte
			                       );
		
		// Render the pixels into the buffer
		for (int row_bit = 0; row_bit < 8; row_bit++) {
			// Get the pixel position (centering the font within the screen's height)
			int row_pixel = row_byte*8 + row_bit + (((int)HEIGHT - (int)FONT_HEIGHT)/2);
			if (row_pixel >= 0 && row_pixel < HEIGHT) {
				buf[(row_pixel*WIDTH) + WIDTH - 1] = (c >> (7-row_bit)) & 1;
			}
		}
	}
	
	// Advance through the columns
	if (state.this_col >= this_end - next_start)
		state.next_col++;
	state.this_col++;
	
	// Move to the next character as required (possibly skipping the next
	// character if it was already completely printed in the current pass).
	while (state.this_col >= this_width && *(state.this_char) != '\0') {
		state.this_char++;
		state.this_col = state.next_col;
		state.next_col = 0;
	}
	
	return true;
}
