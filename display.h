/**
 * Functions for interacting with the display hardware.
 */

#ifndef DISPLAY_H
#define DISPLAY_H

/**
 * Setup SPI hardware, send initial start-up commands to the display, come out of power down with
 * all registers blanked and intensity set to full.
 */
void display_begin(void);


/**
 * Shift a frame buffer image onto the display, and set the display intensity.
 */
void display_buf(const char *buf, int global_intensity);


#endif
