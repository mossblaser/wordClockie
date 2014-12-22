#include "tween.h"


////////////////////////////////////////////////////////////////////////////////
// Internal state
////////////////////////////////////////////////////////////////////////////////

static struct {
	// The source/destination frames
	const char *from;
	const char *to;
	
	// The animation to use
	tween_animation_t animation;
	
	// The duration to animate for
	int duration;
	
	// The number of frames elapsed
	int elapsed;
} state;


////////////////////////////////////////////////////////////////////////////////
// Tweening functions (internal)
////////////////////////////////////////////////////////////////////////////////

bool tween_next_cut(char const **buf, int *global_intensity) {
	if (state.elapsed == 0) {
		*buf = state.to;
		*global_intensity = 0xF;
		state.elapsed = 1;
		return true;
	} else {
		return false;
	}
}


bool tween_next_fade_from_black(char const **buf, int *global_intensity) {
	state.elapsed++;
	
	if (state.elapsed <= state.duration) {
		*buf = state.to;
		*global_intensity = (state.elapsed * 0xF) / state.duration;
		return true;
	} else {
		// Prevent overflow
		state.elapsed--;
		return false;
	}
}


bool tween_next_fade_to_black(char const **buf, int *global_intensity) {
	state.elapsed++;
	if (state.elapsed < state.duration) {
		*buf = state.from;
		*global_intensity = 0xF - ((state.elapsed * 0xF) / state.duration);
		return true;
	} else if (state.elapsed == state.duration) {
		*buf = state.to;
		*global_intensity = 0xF;
		return true;
	} else {
		// Prevent overflow
		state.elapsed--;
		return false;
	}
}


bool tween_next_fade_through_black(char const **buf, int *global_intensity) {
	state.elapsed++;
	if (state.elapsed < state.duration/2) {
		*buf = state.from;
		*global_intensity = 0xF - ((state.elapsed * 0xF) / (state.duration/2));
		return true;
	} else if (state.elapsed <= state.duration) {
		*buf = state.to;
		*global_intensity = ((state.elapsed - (state.duration/2)) * 0xF)
		                    / (state.duration - (state.duration/2));
		return true;
	} else {
		// Prevent overflow
		state.elapsed--;
		return false;
	}
}


bool tween_next_fade(char const **buf, int *global_intensity) {
	state.elapsed++;
	if (state.elapsed < state.duration) {
		int duty = state.elapsed % TWEEN_FADE_DUTYCYCLE;
		int phase = (state.elapsed * TWEEN_FADE_DUTYCYCLE) / state.duration;
		if (duty > phase)
			*buf = state.from;
		else
			*buf = state.to;
		
		*global_intensity = 0xF;
		return true;
	} else if (state.elapsed == state.duration) {
		*buf = state.to;
		*global_intensity = 0xF;
		return true;
	} else {
		// Prevent overflow
		state.elapsed--;
		return false;
	}
}



////////////////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////////////////

void tween_start( const char *from
                , const char *to
                , tween_animation_t animation
                , int duration
                ) {
	state.from      = from;
	state.to        = to;
	state.animation = animation;
	state.duration  = duration;
	
	state.elapsed = 0;
}


bool tween_next(char const **buf, int *global_intensity) {
	switch (state.animation) {
		case TWEEN_CUT:                return tween_next_cut(buf, global_intensity);
		case TWEEN_FADE_FROM_BLACK:    return tween_next_fade_from_black(buf, global_intensity);
		case TWEEN_FADE_TO_BLACK:      return tween_next_fade_to_black(buf, global_intensity);
		case TWEEN_FADE_THROUGH_BLACK: return tween_next_fade_through_black(buf, global_intensity);
		case TWEEN_FADE:               return tween_next_fade(buf, global_intensity);
		
		default:
			return false;
	}
}
