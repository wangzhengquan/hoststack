#ifndef FORK_PTY_H
#define FORK_PTY_H

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <functional>

pid_t ptyFork(int *masterFd, char *slaveName, size_t snLen,
        const struct termios *slaveTermios, const struct winsize *slaveWS);

int ptyClone(const struct termios *slaveTermios, const struct winsize *slaveWS, int *mfd, std::function<int(void *)>  fn);

#endif
