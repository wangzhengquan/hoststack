#ifndef FORK_PTY_H
#define FORK_PTY_H

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>

pid_t ptyFork(int *masterFd, char *slaveName, size_t snLen,
        const struct termios *slaveTermios, const struct winsize *slaveWS);

pid_t ptyClone( const struct termios *slaveTermios,
    const struct winsize *slaveWS, 
    int *masterFd, 
    int (*fn)(void *), void *child_stack, int flags, void *arg)
#endif
