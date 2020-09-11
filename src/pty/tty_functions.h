/* tty_functions.h

   Header file for tty_functions.c.
*/
#ifndef TTY_FUNCTIONS_H
#define TTY_FUNCTIONS_H

#include <termios.h>

int ttySetCbreak(int fd, struct termios *prevTermios);

int ttySetRaw(int fd, struct termios *prevTermios);

#endif
