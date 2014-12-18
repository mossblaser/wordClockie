/**
 * Functions for generating bitmaps representing the time using the letter mask
 * infront of the display.
 */

#ifndef WORDS_H
#define WORDS_H

#include "word_clock.h"

/**
 * Get the pixel mask required to display a specified string based on the mask
 * defined in words.cpp.
 *
 * Internally the string is processed from right-to-left (and from the
 * bottom-right to top-left of the mask). This means that when displaying the
 * time, the hours are displayed in consistent locations.
 *
 * @param buf A pointer to a buffer of size [HEIGHT][WIDTH] which will be loaded
 *            with 0 where no character is present and 1 where a character is
 *            present.
 *
 * @param str A string to be displayed.
 *
 * @returns True if the string was successfuly displayed or false if the string
 *          could not be rendered using the characters available.
 */
bool get_word_mask(char *buf, const char *str);


/**
 * Append a string onto to another string.
 */
void words_append(char *str, const char *appendage);

/**
 * Append a number in the range 1 to 39 inclusive written as words (e.g. "one").
 */
void words_append_number(char *str, int number);

/**
 * Append the time as strings such as "twenty four to seven pm" and "quarter
 * past two am".
 *
 * @param hours Number of hours on 24-hour clock (i.e. 0-23)
 * @param minutes Number of minutes (i.e. 0-59)
 */
void words_append_time(char *str, int hours, int minutes);

#endif
