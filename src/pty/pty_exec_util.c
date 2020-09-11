#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <termios.h>
#if ! defined(__hpux)
/* HP-UX 11 doesn't have this header file */
#include <sys/select.h>
#endif
#include "pty_fork.h"           /* Declaration of ptyFork() */
#include "tty_functions.h"      /* Declaration of ttySetRaw() */
#include "usg_common.h"
#include "pty_exec_util.h"


#define BUF_SIZE 4096
#define MAX_SNAME 1000

struct request {                /* Request (client --> server) */
  char *clientFifo;                 /* fifo of client */
  char buf[BUF_SIZE];
  int bufSize;
};

struct termios ttyOrig;

static void             /* Reset terminal mode on program exit */
ttyReset(void)
{
  if (tcsetattr(STDIN_FILENO, TCSANOW, &ttyOrig) == -1)
    err_exit(errno, "pty_exec_util ttyReset tcsetattr");
}

int pty_exec(pty_exe_opt_t arg)
{
  char slaveName[MAX_SNAME];
  int masterFd, logFd;
 
  struct winsize ws;
  fd_set inFds;
  char buf[BUF_SIZE];
  ssize_t numRead;
  pid_t childPid;

  /* Retrieve the attributes of terminal on which we are started */

  if (tcgetattr(STDIN_FILENO, &ttyOrig) == -1)
    err_exit(errno, "tcgetattr");
  if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0)
    err_exit(errno, "ioctl-TIOCGWINSZ");

  /* Create a child process, with parent and child connected via a
     pty pair. The child is connected to the pty slave and its terminal
     attributes are set to be the same as those retrieved above. */

  childPid = ptyFork(&masterFd, slaveName, MAX_SNAME, &ttyOrig, &ws);
  if (childPid == -1)
    err_exit(errno, "ptyFork");

  if (childPid == 0)          /* Child: execute a shell on pty slave */
  {
    /* chroot 隔离目录 */
    if ( chdir(arg.rootfs) != 0 || chroot("./") != 0 )
    {
      err_exit(errno, "chdir/chroot:%s", arg.rootfs);
    }


    if (system("touch /usr/lib/tmp") != 0)
    {
      err_exit(errno, "touch /usr/lib/tmp");
    }

    execvp(arg.cmd[0], arg.cmd);
    err_exit(errno, "execvp: %s\n", arg.cmd[0]);
  }


  /* Parent: relay data between terminal and pty master */
  if(arg.detach) {
    logFd = open( arg.logfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (logFd == -1)
        err_exit(errno, "pty_exec open typescript: %s",  arg.logfile);
  }
 

  /* Place terminal in raw mode so that we can pass all terminal
   input to the pseudoterminal master untouched */

  ttySetRaw(STDIN_FILENO, &ttyOrig);

  if (atexit(ttyReset) != 0)
    err_exit(errno, "atexit");

  /* Loop monitoring terminal and pty master for input. If the
     terminal is ready for input, then read some bytes and write
     them to the pty master. If the pty master is ready for input,
     then read some bytes and write them to the terminal. */

  for (;;)
  {
    FD_ZERO(&inFds);
    FD_SET(STDIN_FILENO, &inFds);
    FD_SET(masterFd, &inFds);

    if (select(masterFd + 1, &inFds, NULL, NULL, NULL) == -1)
      err_exit(errno, "select");

    
    if (FD_ISSET(STDIN_FILENO, &inFds))     /* stdin --> pty */
    {
      if(!arg.detach) {
        numRead = read(STDIN_FILENO, buf, BUF_SIZE);
        if (numRead <= 0)
          exit(EXIT_SUCCESS);

        if (write(masterFd, buf, numRead) != numRead)
          err_exit(errno, "partial/failed write (masterFd)");
      }
    }

    if (FD_ISSET(masterFd, &inFds))        /* pty --> stdout+file */
    {
      numRead = read(masterFd, buf, BUF_SIZE);
      if (numRead <= 0)
        exit(EXIT_SUCCESS);
      if(arg.detach) {
        if (write(logFd, buf, numRead) != numRead)
          err_exit(errno, "partial/failed write (logFd)");
      } else {
        if (write(STDOUT_FILENO, buf, numRead) != numRead)
          err_exit(errno, "partial/failed write (STDOUT_FILENO)");
      }
     
      
    }
  }
}



int pty_proxy_exec(pty_exe_opt_t arg)
{
  char slaveName[MAX_SNAME];
  int masterFd;
  struct winsize ws;
 
  char buf[BUF_SIZE];
  ssize_t numRead;
  pid_t childPid;

  /* Retrieve the attributes of terminal on which we are started */

  if (tcgetattr(STDIN_FILENO, &ttyOrig) == -1)
    err_exit(errno, "tcgetattr");
  if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0)
    err_exit(errno, "ioctl-TIOCGWINSZ");

  /* Create a child process, with parent and child connected via a
     pty pair. The child is connected to the pty slave and its terminal
     attributes are set to be the same as those retrieved above. */

  childPid = ptyFork(&masterFd, slaveName, MAX_SNAME, &ttyOrig, &ws);
  if (childPid == -1)
    err_exit(errno, "ptyFork");

  if (childPid == 0)          /* Child: execute a shell on pty slave */
  {
    /* chroot 隔离目录 */
    if ( chdir(arg.rootfs) != 0 || chroot("./") != 0 )
    {
      err_exit(errno, "chdir/chroot:%s", arg.rootfs);
    }


    if (system("touch /usr/lib/tmp") != 0)
    {
      err_exit(errno, "touch /usr/lib/tmp");
    }

    execvp(arg.cmd[0], arg.cmd);
    err_exit(errno, "execvp: %s\n", arg.cmd[0]);
  }

  fd_set inFds;
  int serverFd, dummyFd, clientFd, maxFd = masterFd;
  struct request req;

  /* Create well-known FIFO, and open it for reading */

  umask(0);                           /* So we get the permissions we want */
  if (mkfifo(arg.serverFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST)
    err_exit(errno, "mkfifo %s", arg.serverFifo);
  serverFd = open(arg.serverFifo, O_RDONLY);
  if (serverFd == -1)
    err_exit(errno, "open %s", arg.serverFifo);
  maxFd = MAX(serverFd, maxFd);
  /* Open an extra write descriptor, so that we never see EOF */

  dummyFd = open(arg.serverFifo, O_WRONLY);
  if (dummyFd == -1)
      err_exit(errno, "open %s", arg.serverFifo);

  /* Let's find out about broken client pipe via failed write() */

  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)    err_exit(errno, "signal");


  for (;;)
  {
    FD_ZERO(&inFds);
    FD_SET(serverFd, &inFds);
    FD_SET(masterFd, &inFds);

    if (select(maxFd + 1, &inFds, NULL, NULL, NULL) == -1)
      err_exit(errno, "select");

    // 收到client请求
    if (FD_ISSET(serverFd, &inFds))     /* stdin --> pty */
    {
        numRead = read(serverFd, &req, sizeof(struct request));
        if (numRead != sizeof(struct request))
          exit(EXIT_SUCCESS);

        if (write(masterFd, req.buf, req.bufSize) != req.bufSize)
          err_exit(errno, "partial/failed write (masterFd)");
    }

    if (FD_ISSET(masterFd, &inFds))        /* pty --> stdout+file */
    {
      numRead = read(masterFd, buf, BUF_SIZE);
      if (numRead <= 0)
        exit(EXIT_SUCCESS);
      clientFd = open(req.clientFifo, O_WRONLY);
      if (clientFd == -1) {           /* Open failed, give up on client */
          err_exit(errno, "open %s", arg.clientFifo);
          continue;
      }
      if (write(clientFd, buf, numRead) != numRead)
          err_exit(errno, "partial/failed write (STDOUT_FILENO)");
      if (close(clientFd) == -1)
          err_msg(errno, "close");
    }
  }

}


int pty_client(pty_exe_opt_t arg) {
  fd_set inFds;
  int serverFd;
  int clientFd;
  int logFd;
  int maxFd = 2;
  char buf[BUF_SIZE];
  ssize_t numRead;
  struct request req;
  /* Place terminal in raw mode so that we can pass all terminal
  input to the pseudoterminal master untouched */

  ttySetRaw(STDIN_FILENO, &ttyOrig);

  if (atexit(ttyReset) != 0)
    err_exit(errno, "atexit");

  umask(0);
  serverFd = open(arg.serverFifo, O_WRONLY);
  if (serverFd == -1)
    err_exit(errno, "open %s", arg.serverFifo);
  maxFd = MAX(serverFd, maxFd);

  clientFd = open(arg.clientFifo, O_RDONLY);
  if (clientFd == -1)
        err_exit(errno, "open %s", arg.clientFifo);
  maxFd = MAX(clientFd, maxFd);


  if(arg.detach) {
    logFd = open( arg.logfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (logFd == -1)
        err_exit(errno, "open typescript");
  }

  /* Loop monitoring terminal and pty master for input. If the
     terminal is ready for input, then read some bytes and write
     them to the pty master. If the pty master is ready for input,
     then read some bytes and write them to the terminal. */
  for (;;)
  {
    FD_ZERO(&inFds);
    FD_SET(STDIN_FILENO, &inFds);
    FD_SET(serverFd, &inFds);

    if (select(maxFd + 1, &inFds, NULL, NULL, NULL) == -1)
      err_exit(errno, "select");

    
    if (FD_ISSET(STDIN_FILENO, &inFds))     /* stdin --> pty */
    {
      if(!arg.detach) {
        numRead = read(STDIN_FILENO, req.buf, BUF_SIZE);
        if (numRead <= 0)
          exit(EXIT_SUCCESS);
        req.clientFifo = arg.clientFifo;
        req.bufSize = numRead;
        if (write(serverFd, &req, sizeof(request)) !=  sizeof(request))
          err_exit(errno, "partial/failed write (masterFd)");
      }
    }

    if (FD_ISSET(serverFd, &inFds))        /* pty --> stdout+file */
    {
      numRead = read(serverFd, buf, BUF_SIZE);
      if (numRead <= 0)
        exit(EXIT_SUCCESS);
      if(arg.detach) {
        if (write(logFd, buf, numRead) != numRead)
          err_exit(errno, "partial/failed write (logFd)");
      } else {
        if (write(STDOUT_FILENO, buf, numRead) != numRead)
          err_exit(errno, "partial/failed write (STDOUT_FILENO)");
      }
     
      
    }
  }

}
