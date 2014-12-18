#include "word_clock.h"
#include "automata.h"


// Is the given coordinate within the buffer's extents?
#define EXISTS(x,y) ((x) >= 0 && (x) < WIDTH && (y) >= 0 && (y) < HEIGHT)

// Return the value of the given coordinate or zero if the coordinate is not in
// range of the buffer.
#define ZERO_IF_NOT_EXIST(buf, x, y) ((EXISTS((x),(y))) ? (buf)[(y)*WIDTH + (x)] : 0)

void automata_xor(char *to_buf, const char *from_buf) {
	for (int x = 0; x < WIDTH; x++) {
		for (int y = 0; y < HEIGHT; y++) {
			to_buf[y*WIDTH + x] = from_buf[y*WIDTH + x]
			                    ^ ZERO_IF_NOT_EXIST(from_buf, x+1,y+0)
			                    ^ ZERO_IF_NOT_EXIST(from_buf, x-1,y+0)
			                    ^ ZERO_IF_NOT_EXIST(from_buf, x+0,y+1)
			                    ^ ZERO_IF_NOT_EXIST(from_buf, x+0,y-1)
			                    ;
		}
	}
}

void automata_life(char *to_buf, const char *from_buf) {
	for (int x = 0; x < WIDTH; x++) {
		for (int y = 0; y < HEIGHT; y++) {
			int live_neighbours = ZERO_IF_NOT_EXIST(from_buf, x+1,y+0)
			                    + ZERO_IF_NOT_EXIST(from_buf, x-1,y+0)
			                    + ZERO_IF_NOT_EXIST(from_buf, x+0,y+1)
			                    + ZERO_IF_NOT_EXIST(from_buf, x+0,y-1)
			                    + ZERO_IF_NOT_EXIST(from_buf, x+1,y+1)
			                    + ZERO_IF_NOT_EXIST(from_buf, x-1,y+1)
			                    + ZERO_IF_NOT_EXIST(from_buf, x+1,y+1)
			                    + ZERO_IF_NOT_EXIST(from_buf, x+1,y-1)
			                    ;
			
			if (from_buf[y*WIDTH + x]) {
				to_buf[y*WIDTH + x] = live_neighbours >= 2 && live_neighbours <= 3;
			} else {
				to_buf[y*WIDTH + x] = live_neighbours == 3;
			}
		}
	}
}
