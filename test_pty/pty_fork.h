
/* Header file for Listing 64-2 */

/* pty_fork.h

   Header file for pty_fork.c.
*/
#ifndef FORK_PTY_H
#define FORK_PTY_H

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>

pid_t ptyFork(int *masterFd, char *slaveName, size_t snLen,
        const struct termios *slaveTermios, const struct winsize *slaveWS);

#endif
