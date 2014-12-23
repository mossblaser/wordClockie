#include <avr/pgmspace.h>

#include "word_clock.h"
#include "face.h"

////////////////////////////////////////////////////////////////////////////////
// Face bitmaps
////////////////////////////////////////////////////////////////////////////////

#include "eyes_0_.png.h"
#include "eyes_0.png.h"
#include "eyes_1.png.h"
#include "eyes_n1.png.h"
#include "eyes_n2.png.h"
#include "mouth_0.png.h"
#include "mouth_10.png.h"
#include "mouth_11.png.h"
#include "mouth_12.png.h"
#include "mouth_13.png.h"
#include "mouth_14.png.h"
#include "mouth_15.png.h"
#include "mouth_1.png.h"
#include "mouth_2.png.h"
#include "mouth_3.png.h"
#include "mouth_4.png.h"
#include "mouth_5.png.h"
#include "mouth_6.png.h"
#include "mouth_7.png.h"
#include "mouth_8.png.h"
#include "mouth_9.png.h"
#include "mouth_n1.png.h"
#include "mouth_n2.png.h"
#include "mouth_n3.png.h"
#include "mouth_n4.png.h"
#include "nose.png.h"

#define MOUTH_MIN FACE_MIN
#define MOUTH_MAX FACE_MAX
static prog_uchar *MOUTHES[1+MOUTH_MAX-MOUTH_MIN] = {
	mouth_n4, mouth_n3, mouth_n2, mouth_n1,
	mouth_0, mouth_1, mouth_2, mouth_3, mouth_4,
	mouth_5, mouth_6, mouth_7, mouth_8, mouth_9,
	mouth_10, mouth_11, mouth_12, mouth_13, mouth_14,
	mouth_15,
};


#define EYE_MIN FACE_MIN
#define EYE_MAX FACE_MAX
static prog_uchar *EYES[1+EYE_MAX-EYE_MIN] = {
	eyes_n2, eyes_n2, eyes_n1, eyes_n1,
	eyes_0, eyes_0, eyes_0, eyes_0, eyes_0, eyes_0, eyes_0, eyes_0,
	eyes_1, eyes_1, eyes_1, eyes_1, eyes_1, eyes_1, eyes_1, eyes_1,
};

static prog_uchar *EYE_BLINK = eyes_0_;

static prog_uchar *NOSE = nose;


static void overlay_bitmap(char *buf, prog_uchar *bitmap) {
	const int width_bytes = (WIDTH+7)/8;
	
	for (int y = 0; y < HEIGHT; y++) {
		for (int x_byte = 0; x_byte < width_bytes; x_byte++) {
			unsigned char c = pgm_read_byte_near(bitmap + (y*width_bytes) + x_byte);
			for (int x_bit = 0; x_bit < 8; x_bit++) {
				int x = x_byte*8 + x_bit;
				buf[y*WIDTH + x] |= (c >> x_bit) & 1;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////

void face(char *buf, int happiness, bool blink) {
	// Blank the frame
	for (int i = 0; i < WIDTH*HEIGHT; i++)
		buf[i] = 0;
	
	// Build up the face
	overlay_bitmap(buf, NOSE);
	overlay_bitmap(buf, MOUTHES[happiness - MOUTH_MIN]);
	if (blink)
		overlay_bitmap(buf, EYE_BLINK);
	else
		overlay_bitmap(buf, EYES[happiness - EYE_MIN]);
}
