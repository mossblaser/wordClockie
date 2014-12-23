/**
 * Text rendering functions for producing scrolling on-screen messages.
 */

#ifndef TEXT_H
#define TEXT_H

/**
 * Specify the string to display, this string must remain valid for the full
 * duration of the animation (i.e. until text_next returns false).
 */
void text_start(const char *str);


/**
 * Shift in the next column of character pixels.
 *
 * @param buf The display buffer to render the text into. The contents will be
 *            pushed off the left of the buffer as the text scrolls into view.
 * @returns true if the string is still being displayed, false otherwise. The
 *          last frame will be all blank and undefined when this function
 *          returns false.
 */
bool text_next(char *buf);

#endif
