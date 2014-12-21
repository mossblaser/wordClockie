/**
 * Firmware for a word clock for a special Cube wife.
 *
 * Hardware consists of an array of MAX7219/MAX7221 driven LED arrays, a
 * realtime clock and an LDR and tilt switches for I/O.
 */

#ifndef WORD_CLOCK_H
#define WORD_CLOCK_H

////////////////////////////////////////////////////////////////////////////////
// Display size
////////////////////////////////////////////////////////////////////////////////

// Displays are attached in a scan path such that the last display in the scan
// path is the top-left display and the first in the path is the bottom-right
// display:
//
//           X axis
//           ----->
//         | +-----------+ +-----------+ +-----------+
//  Y axis | | o o o o o | | o o o o o | | o o o o o |
//         | | o o o o o | | o o o o o | | o o o o o |
//         V | o o o o o | | o o o o o | | o o o o o |
//           | o o o-o-o--<--o-o-o-o-o--<--o-o-o-o-o--<-+
//           | o o o o o | | o o o o o | | o o o o o |  |
//           | o o o o o | | o o o o o | | o o o o o |  |
//           | o o o o o | | o o o o o | | o o o o o |  |
//           +----+------+ +-----------+ +-----------+  |
//        +---------------------->----------------------+
//        |  +-----------+ +-----------+ +-----------+
//        |  | o o o o o | | o o o o o | | o o o o o |
//        |  | o o o o o | | o o o o o | | o o o o o |
//        |  | o o o o o | | o o o o o | | o o o o o |
//        +----o-o-o-o-o--<--o-o-o-o-o--<--o-o-o-o-o--<-- Scan path in
//           | o o o o o | | o o o o o | | o o o o o |
//           | o o o o o | | o o o o o | | o o o o o |
//           | o o o o o | | o o o o o | | o o o o o |
//           +-----------+ +-----------+ +-----------+


// Number of displays on each axis
#define DISPLAYS_X 3
#define DISPLAYS_Y 2

// The size of each display
#define DISPLAY_WIDTH 5
#define DISPLAY_HEIGHT 7

// The resulting size of the pixel array
#define WIDTH  ((DISPLAYS_X) * (DISPLAY_WIDTH))
#define HEIGHT ((DISPLAYS_Y) * (DISPLAY_HEIGHT))


////////////////////////////////////////////////////////////////////////////////
// UI Animation Timing
////////////////////////////////////////////////////////////////////////////////

// Fames for fade-in of initial message
#define RESET_MESSAGE_TWEEN_FRAMES 1000

// Time (ms) to display the initial message
#define RESET_MESSAGE_TIMEOUT_MSEC 5000

// Frames for animation when starting to display the clock
#define CLOCK_TWEEN_FRAMES 1000

// Frames for animation when changing the clock time
#define CLOCK_UPDATE_TWEEN_FRAMES 200

// Frames for fading out animation of previous frame before displaying a
// scrolling message
#define SCROLL_MESSAGE_TWEEN_FRAMES 1000

// Time (ms) between frames of a scrolling message
#define SCROLLING_MESSAGE_FRAME_MSEC 100

// Frames for animation when changing to showing the duration of our marriage
// (or changing this message).
#define MARRIAGE_DURATION_TWEEN_FRAMES 500

// Time (ms) between parts of the description of the time we've been married
#define MARRIAGE_DURATION_PAUSE_MSEC 3000

// Frames for animation when changing to showing the unicom sync status
#define UNICOM_START_TWEEN_FRAMES 500

// Period (ms) of blinking pattern during unisync lock phase
#define UNICOM_LOCKED_BLINK_PHASE_MSEC 300

// Time (ms) between frames of the saddening face
#define UNICOM_ERROR_FRAME_MSEC 60

// Time (ms) after the sad/happy face before the clock returns
#define UNICOM_FINAL_TIMEOUT_MSEC 1000

////////////////////////////////////////////////////////////////////////////////
// Significant dates
////////////////////////////////////////////////////////////////////////////////

#define ANIVERSARY_YEAR 2014
#define ANIVERSARY_MONTH 1
#define ANIVERSARY_DAY 4

#define MET_YEAR 2010
#define MET_MONTH 11
#define MET_DAY 14

#define CUBE_BIRTHDAY_YEAR 1989
#define CUBE_BIRTHDAY_MONTH 10
#define CUBE_BIRTHDAY_DAY 30

#define THAN_BIRTHDAY_YEAR 1990
#define THAN_BIRTHDAY_MONTH 12
#define THAN_BIRTHDAY_DAY 11

#define CHRISTMAS_MONTH 12
#define CHRISTMAS_DAY 25

#define NEW_YEAR_MONTH 1
#define NEW_YEAR_DAY 1

#define NEW_YEAR_EVE_MONTH 12
#define NEW_YEAR_EVE_DAY 31

////////////////////////////////////////////////////////////////////////////////
// Pin Definitions
////////////////////////////////////////////////////////////////////////////////

#ifdef ARDUINO
	#import "Arduino.h"
#else
	#define A0 14
	#define A1 15
#endif

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


#endif
