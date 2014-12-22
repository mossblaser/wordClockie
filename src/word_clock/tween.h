/**
 * Library for generating animations between different frame buffers.
 */

#ifndef TWEEN_H
#define TWEEN_H

#include "word_clock.h"

/**
 * Different tween animations available.
 */
typedef enum {
	// Immeate transition with no animation. Always lasts one frame. Does not
	// need a starting frame.
	TWEEN_CUT,
	
	// Fade in from black. Doesn't need a starting frame.
	TWEEN_FADE_FROM_BLACK,
	
	// Fade out to black and afterward cut immediately to the ending frame.
	TWEEN_FADE_TO_BLACK,
	
	// Fade from one frame to another via black.
	TWEEN_FADE_THROUGH_BLACK,
	
	// Fade from one frame to another.
	TWEEN_FADE,
} tween_animation_t;


// Number of frames over which to perform PWM during the fade tween (also
// specifies the number of intensity levels that will be achieved).
#define TWEEN_FADE_DUTYCYCLE 8


/**
 * Initialise the tween logic to begin a new tween between the given pair of
 * frames (assumed to be at full intensity).
 *
 * @param from The frame to start the tween from (which must remain constant
 *             throughout the tween) of size [HEIGHT][WIDTH].
 * @param to The frame to end the tween on (which must remain constant
 *           throughout the tween) of size [HEIGHT][WIDTH].
 * @param animation The animation to use.
 * @param duration The desired number of frames to animate for.
 */
void tween_start( const char *from
                , const char *to
                , tween_animation_t animation
                , int duration
                );

/**
 * To be called repeatedly after tween_start has been called. This function
 * should be executed once per frame until it returns false at which point the
 * tween has been completed.
 *
 * @param buf A double pointer to the buffer of size [HEIGHT][WIDTH] which will
 *            be displayed.
 *
 * @param global_intensity An integer which will have an intensity value in the
 *                         range 0x0-0xF which should be applied to all lit pixels
 *                         on the display. Here, 0x0 is minimum intensity and
 *                         0xF is maximum intensity. 0x0 may not be completely
 *                         off.
 *
 * @param returns true if a frame was produced and false otherwise. If false,
 *                the buffer and global_intensity values may be invalid.
 */
bool tween_next(const char **buf, int *global_intensity);


#endif
