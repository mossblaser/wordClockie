/**
 * Simple library for generating images of faces in various moods.
 */

#ifndef FACE_H
#define FACE_H


/**
 * Minimum and maximum values for happiness of the face.
 */
#define FACE_MAX 15
#define FACE_MIN -4


/**
 * Load a face in the specified mood into the buffer.
 *
 * @param buf A buffer [HEIGHT][WIDTH].
 * @param happiness An integer in the range FACE_MIN to FACE_MAX inclusive where
 *                  FACE_MIN is very sad, 0 is neutral and FACE_MAX is very
 *                  happy.
 * @param blink If true, set the eyes to look like they're blinking, otherwise
 *              the eyes should be open.
 */
void face(char *buf, int happiness, bool blink);

#endif
