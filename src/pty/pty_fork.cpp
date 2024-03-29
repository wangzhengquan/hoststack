/* pty_fork.c

   Implements ptyFork(), a function that creates a child process connected to
   the parent (i.e., the calling process) via a pseudoterminal (pty). The child
   is placed in a new session, with the pty slave as its controlling terminal,
   and its standard input, output, and error connected to the pty slave.

   In the parent, 'masterFd' is used to return the file descriptor for the
   pty master.

   If 'slaveName' is non-NULL, then it is used to return the name of the pty
   slave. If 'slaveName' is not NULL,  then 'snLen' should be set to indicate
   the size of the buffer pointed to by 'slaveName'.

   If 'slaveTermios' and 'slaveWS' are non-NULL, then they are used respectively
   to set the terminal attributes and window size of the pty slave.

   Returns:
        in child: 0
        in parent: PID of child or -1 on error
*/
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "pty_master_open.h"
#include "pty_fork.h"                   /* Declares ptyFork() */
#include "usg_common.h"
#include "tty_functions.h"

#define MAX_SNAME 1000                  /* Maximum size for pty slave name */

/* 定义一个给 clone 用的栈，栈大小1M */
#define STACK_SIZE (1024 * 1024)

pid_t
ptyFork(int *masterFd, char *slaveName, size_t snLen,
        const struct termios *slaveTermios, const struct winsize *slaveWS)
{
  int mfd, slaveFd, savedErrno;
  pid_t childPid;
  char slname[MAX_SNAME];

  mfd = ptyMasterOpen(slname, MAX_SNAME);
  if (mfd == -1)
    return -1;

  if (slaveName != NULL)              /* Return slave name to caller */
  {
    if (strlen(slname) < snLen)
    {
      strncpy(slaveName, slname, snLen);

    }
    else                            /* 'slaveName' was too small */
    {
      close(mfd);
      errno = EOVERFLOW;
      return -1;
    }
  }

  childPid = fork();

  if (childPid == -1)                 /* fork() failed */
  {
    savedErrno = errno;             /* close() might change 'errno' */
    close(mfd);                     /* Don't leak file descriptors */
    errno = savedErrno;
    return -1;
  }

  if (childPid != 0)                  /* Parent */
  {
    *masterFd = mfd;                /* Only parent gets master fd */
    return childPid;                /* Like parent of fork() */
  }

  /* Child falls through to here */

  if (setsid() == -1)                 /* Start a new session */
    err_exit(errno, "ptyFork:setsid");

  close(mfd);                         /* Not needed in child */

  slaveFd = open(slname, O_RDWR);     /* Becomes controlling tty */
  if (slaveFd == -1)
    err_exit(errno, "ptyFork:open-slave");

#ifdef TIOCSCTTY                        /* Acquire controlling tty on BSD */
  if (ioctl(slaveFd, TIOCSCTTY, 0) == -1)
    err_exit(errno, "ptyFork:ioctl-TIOCSCTTY");
#endif

  if (slaveTermios != NULL)           /* Set slave tty attributes */
    if (tcsetattr(slaveFd, TCSANOW, slaveTermios) == -1)
      err_exit(errno, "ptyFork:tcsetattr");

  if (slaveWS != NULL)                /* Set slave tty window size */
    if (ioctl(slaveFd, TIOCSWINSZ, slaveWS) == -1)
      err_exit(errno, "ptyFork:ioctl-TIOCSWINSZ");

  /* Duplicate pty slave to be child's stdin, stdout, and stderr */

  if (dup2(slaveFd, STDIN_FILENO) != STDIN_FILENO)
    err_exit(errno, "ptyFork:dup2-STDIN_FILENO");
  if (dup2(slaveFd, STDOUT_FILENO) != STDOUT_FILENO)
    err_exit(errno, "ptyFork:dup2-STDOUT_FILENO");
  if (dup2(slaveFd, STDERR_FILENO) != STDERR_FILENO)
    err_exit(errno, "ptyFork:dup2-STDERR_FILENO");

  if (slaveFd > STDERR_FILENO)        /* Safety check */
    close(slaveFd);                 /* No longer need this fd */

  return 0;                           /* Like child of fork() */
}

struct PtyRunArg
{
  int mfd;
  char * slname;
  const struct termios *slaveTermios;
  const struct winsize *slaveWS;
  std::function<int(void *)> fn;
  void *fnArg;
};

int _ptyCloneRun(void *_arg)
{
  int slaveFd;
  /* Child falls through to here */
  PtyRunArg &arg = *((PtyRunArg *)_arg);
  if (setsid() == -1)                 /* Start a new session */
    err_exit(errno, "ptyFork:setsid");

  close(arg.mfd);                         /* Not needed in child */

  slaveFd = open(arg.slname, O_RDWR);     /* Becomes controlling tty */
  if (slaveFd == -1)
  {
    err_msg(errno, "ptyFork:open-slave");
    return -1;
  }

#ifdef TIOCSCTTY                        /* Acquire controlling tty on BSD */
  if (ioctl(slaveFd, TIOCSCTTY, 0) == -1)
  {
    err_msg(errno, "ptyFork:ioctl-TIOCSCTTY");
    return -1;
  }
#endif

  if (arg.slaveTermios != NULL)           /* Set slave tty attributes */
    if (tcsetattr(slaveFd, TCSANOW, arg.slaveTermios) == -1)
    {
      err_msg(errno, "ptyFork:tcsetattr");
      return -1;
    }

  if (arg.slaveWS != NULL)                /* Set slave tty window size */
    if (ioctl(slaveFd, TIOCSWINSZ, arg.slaveWS) == -1)
    {
      err_msg(errno, "ptyFork:ioctl-TIOCSWINSZ");
      return -1;
    }

  /* Duplicate pty slave to be child's stdin, stdout, and stderr */

  if (dup2(slaveFd, STDIN_FILENO) != STDIN_FILENO)
  {
    err_msg(errno, "ptyFork:dup2-STDIN_FILENO");
    return -1;
  }
  if (dup2(slaveFd, STDOUT_FILENO) != STDOUT_FILENO)
  {
    err_msg(errno, "ptyFork:dup2-STDOUT_FILENO");
    return -1;
  }
  if (dup2(slaveFd, STDERR_FILENO) != STDERR_FILENO)
  {
    err_msg(errno, "ptyFork:dup2-STDERR_FILENO");
    return -1;
  }

  if (slaveFd > STDERR_FILENO)        /* Safety check */
    close(slaveFd);                 /* No longer need this fd */

  return arg.fn(NULL);
}



int ptyClone(const struct termios *slaveTermios, const struct winsize *slaveWS, int *mfd, std::function<int(void *)>  fn)
{
  int masterFd, savedErrno;
  int childPid;
  char slaveName[MAX_SNAME];

  masterFd = ptyMasterOpen(slaveName, MAX_SNAME);
  if (masterFd == -1)
    return -1;
  /* Create a child process, with parent and child connected via a
     pty pair. The child is connected to the pty slave and its terminal
     attributes are set to be the same as those retrieved above. */

// childPid = ptyFork(&masterFd, slaveName, MAX_SNAME, &ttyOrig, &ws);
  PtyRunArg ptyRunArg = {};
  ptyRunArg.mfd = masterFd;
  ptyRunArg.slname = slaveName;
  ptyRunArg.slaveTermios =  slaveTermios;
  ptyRunArg.slaveWS = slaveWS;
  ptyRunArg.fn = fn;
  // ptyRunArg.fnArg = arg;

  void *stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
  if (stack == MAP_FAILED)
    err_exit(0, "handle_run_command mmap:");

  childPid = clone(_ptyCloneRun, (char *)stack + STACK_SIZE,
                            CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, &ptyRunArg);
 
  // childPid = clone(_ptyCloneRun, child_stack, flags, &ptyRunArg);
 

  
  if (childPid == -1)                 /* fork() failed */
  {
    savedErrno = errno;             /* close() might change 'errno' */
    close(masterFd);                     /* Don't leak file descriptors */
    errno = savedErrno;
    return -1;
  }
  *mfd = masterFd;
  return childPid;

 
}
