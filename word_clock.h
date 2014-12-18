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

#define RESET_MESSAGE_TWEEN_FRAMES 1000
#define RESET_MESSAGE_TIMEOUT_MSEC 5000

#define CLOCK_TWEEN_FRAMES 1000
#define CLOCK_UPDATE_TWEEN_FRAMES 200

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
