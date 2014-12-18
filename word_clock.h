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

#endif
