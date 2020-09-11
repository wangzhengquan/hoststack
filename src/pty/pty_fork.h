#ifndef FORK_PTY_H
#define FORK_PTY_H

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>

pid_t ptyFork(int *masterFd, char *slaveName, size_t snLen,
        const struct termios *slaveTermios, const struct winsize *slaveWS);

int ptyClone(int (*fn)(void *), void *child_stack, int flags, void *arg, 
  int *mfd, const struct termios *slaveTermios, const struct winsize *slaveWS);

#endif
