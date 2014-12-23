/**
 * Implements a several popular 2D cellular automata step functions on a buffer.
 */

#ifndef AUTOMATA_H
#define AUTOMATA_H

/**
 * Simple XOR cellular automata rule as found on our wedding invitations.
 */
void automata_xor(char *to_buf, const char *from_buf);

/**
 * The game of life.
 */
void automata_life(char *to_buf, const char *from_buf);

#endif
