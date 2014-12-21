/**
 * Simple library for generating images of faces in various moods.
 */

#ifndef FACE_H
#define FACE_H

/**
 * Load a face in the specified mood into the buffer.
 *
 * @param buf A buffer [HEIGHT][WIDTH].
 * @param happiness An integer in the range -4 to 15 where -4 is very sad, 0 is
 *                  neutral and 15 is very happy.
 * @param blink If true, set the eyes to look like they're blinking, otherwise
 *              the eyes should be open.
 */
void face(char *buf, int happiness, bool blink);

#endif
