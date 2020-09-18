#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <termios.h>
#if ! defined(__hpux)
/* HP-UX 11 doesn't have this header file */
#include <sys/select.h>
#endif
#include <sys/un.h>
#include <sys/socket.h>
#include "pty_fork.h"           /* Declaration of ptyFork() */
#include "tty_functions.h"      /* Declaration of ttySetRaw() */
#include "usg_common.h"
#include "pty_exec_util.h"
#include "container_manager.h"
#include "logger_factory.h"
#include "socket_io.h"
#include "sem_util.h"
#include "path_assembler.h"

#define BUF_SIZE 4096
#define MAX_SNAME 1000

typedef struct { /* Represents a pool of connected descriptors */ //line:conc:echoservers:beginpool
  int maxfd;        /* Largest descriptor in read_set */
  fd_set read_set;  /* Set of all active descriptors */
  fd_set ready_set; /* Subset of descriptors ready for reading  */
  int nready;       /* Number of ready descriptors from select */
  int maxi;         /* Highwater index into client array */
  int clientfd[FD_SETSIZE];    /* Set of active descriptors */
  int masterfd;
} pool; //line:conc:echoservers:endpool

void init_pool(int listenfd, int masterfd, pool *p);
void add_client(int connfd, pool *p);
void check_clients_and_master(pool *p);

struct termios ttyOrig;
struct termios  g_termios_raw ;

pty_exe_opt_t garg;

static std::string containerId;


static void ttyReset(void)
{
  if (tcsetattr(STDIN_FILENO, TCSANOW, &ttyOrig) == -1)
    err_msg(errno, "pty_exec_util ttyReset");
}

static void redirectStdOut() {
  int inFd, outFd;
  char stdoutfile[1024];
  //sprintf(stdoutfile, "%s/containers/%s/stdout.%ld.log",kucker_repo,  garg.containerId, time(0));
  sprintf(stdoutfile, "/dev/null");
  // printf("stdoutfile = %s\n", stdoutfile);

  outFd = open(stdoutfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
  if (outFd == -1)
    err_exit(errno, "pty_exec open log: %s", stdoutfile);
  if( dup2(outFd, STDOUT_FILENO) == -1) {
    err_exit(errno, "redirectStdOut >> dup2 STDOUT_FILENO");
  } else {
    close(outFd);
  }

  char stdinfile[1024];
  sprintf(stdinfile, "%s/containers/%s/stdin.log",kucker_repo,  garg.containerId);
  if (mkfifo(stdinfile, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST)
      err_exit(errno, "mkfifo %s", stdinfile);
 
  //inFd = open("/dev/null", O_RDONLY);
  inFd = open(stdinfile, O_RDONLY);
  if (inFd == -1)
      err_exit(errno, "open %s", stdinfile);
  if(dup2(inFd, STDIN_FILENO) == -1) {
     err_exit(errno, "redirectStdOut >> dup2 STDIN_FILENO");
  } else {
    close(inFd);
  }
}

int pty_exec(pty_exe_opt_t arg)
{

  char slaveName[MAX_SNAME];
  int masterFd;
 
  struct winsize ws;
  fd_set ready_set;
  char buf[BUF_SIZE];
  ssize_t n;
  pid_t childPid;
  const char *rootfs = PathAssembler::getRootFS(arg.containerId, NULL);
  garg = arg;

  if (signal(SIGTTIN, SIG_IGN) == SIG_ERR)    err_msg(errno, "pty_exec >> SIGTTIN");
  if (signal(SIGTTOU, SIG_IGN) == SIG_ERR)    err_msg(errno, "pty_exec >> SIGTTOU");
  /* Retrieve the attributes of terminal on which we are started */
  if (tcgetattr(STDIN_FILENO, &ttyOrig) == -1)
    err_msg(errno, "tcgetattr");
  if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0)
    err_msg(errno, "ioctl-TIOCGWINSZ");

  // if (setpgid(0, 0) == -1) {
  //   err_msg(errno, "pty_exec:setsid");
  //   LoggerFactory::getDebugLogger().debug("error pty_exec_util.setsid");
  // }

  /* Create a child process, with parent and child connected via a
     pty pair. The child is connected to the pty slave and its terminal
     attributes are set to be the same as those retrieved above. */

  childPid = ptyFork(&masterFd, slaveName, MAX_SNAME, &ttyOrig, &ws);
  if (childPid == -1)
    err_msg(errno, "ptyFork");

  if (childPid == 0)          /* Child: execute a shell on pty slave */
  {
    /* chroot 隔离目录 */
    if ( chdir(rootfs) != 0 || chroot("./") != 0 )
    {
      err_msg(errno, "chdir/chroot:%s", rootfs);
    }


    if (system("touch /usr/lib/tmp") != 0)
    {
      err_msg(errno, "touch /usr/lib/tmp");
    }
    execvp(arg.cmd[0], arg.cmd);
    err_msg(errno, "execvp: %s\n", arg.cmd[0]);
  }


  /* Parent: relay data between terminal and pty master */
  if(! arg.detach) {
    /* Place terminal in raw mode so that we can pass all terminal
     input to the pseudoterminal master untouched */
    ttySetRaw(STDIN_FILENO, &ttyOrig);
    if (atexit(ttyReset) != 0)
      err_msg(errno, "atexit");
  } else {
    redirectStdOut();
  }
 

  /* Loop monitoring terminal and pty master for input. If the
     terminal is ready for input, then read some bytes and write
     them to the pty master. If the pty master is ready for input,
     then read some bytes and write them to the terminal. */
  // sigset_t selectBlockSet;
  // sigemptyset(&selectBlockSet);
  // sigaddset(&selectBlockSet, SIGHUP);
  for (;;)
  {
    FD_ZERO(&ready_set);
    FD_SET(STDIN_FILENO, &ready_set);
    FD_SET(masterFd, &ready_set);

    if (select(masterFd + 1, &ready_set, NULL, NULL, NULL) == -1) {
      err_msg(errno, "pty_exec select");
      if(errno == EINTR) {
        continue;
      } 
    }

    
    if (FD_ISSET(STDIN_FILENO, &ready_set))     /* stdin --> pty */
    {
     
      if (( n = read(STDIN_FILENO, buf, BUF_SIZE)) <= 0) {
        if(errno != EINTR)  {
          exit(EXIT_SUCCESS);
        }
      }

      if (write(masterFd, buf, n) != n)
        err_exit(errno, "partial/failed write (masterFd)");
    }

    if (FD_ISSET(masterFd, &ready_set))        /* pty --> stdout+file */
    {
      if ( (n = read(masterFd, buf, BUF_SIZE)) <= 0) {
        if(errno != EINTR)  {
          exit(EXIT_SUCCESS);
        }
      }
      if (write(STDOUT_FILENO, buf, n) != n)
          err_exit(errno, "partial/failed write (STDOUT_FILENO)");
      
    }
  }
}


int pty_proxy_exec(pty_exe_opt_t arg)
{
  char slaveName[MAX_SNAME];
  int masterFd;
  struct winsize ws;
  pid_t childPid;
  const char *rootfs = PathAssembler::getRootFS(arg.containerId, NULL);
  /* Retrieve the attributes of terminal on which we are started */

  if (tcgetattr(STDIN_FILENO, &ttyOrig) == -1)
    err_msg(errno, "tcgetattr");
  if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0)
    err_msg(errno, "ioctl-TIOCGWINSZ");

  /* Create a child process, with parent and child connected via a
     pty pair. The child is connected to the pty slave and its terminal
     attributes are set to be the same as those retrieved above. */

  childPid = ptyFork(&masterFd, slaveName, MAX_SNAME, &ttyOrig, &ws);
  if (childPid == -1)
    err_msg(errno, "ptyFork");

  /* =============== Child: execute a shell on pty slave */
  if (childPid == 0)          
  {
    /* chroot 隔离目录 */
    if ( chdir(rootfs) != 0 || chroot("./") != 0 )
    {
      err_msg(errno, "chdir/chroot:%s", rootfs);
    }


    if (system("touch /usr/lib/tmp") != 0)
    {
      err_msg(errno, "touch /usr/lib/tmp");
    }

    execvp(arg.cmd[0], arg.cmd);
    err_msg(errno, "execvp: %s\n", arg.cmd[0]);
  }
  /*==============exec end==============*/

 
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)    err_msg(errno, "signal");

  int     listenfd , connfd;
  struct sockaddr_un  listen_addr, clientaddr ;
  static pool pool;
  socklen_t     clientlen ;
 

  /* Create well-known FIFO, and open it for reading */
  listenfd = socket( AF_UNIX , SOCK_STREAM , 0 ) ;
  if( listenfd == -1 )
  {
    err_exit( errno, "pty_proxy_exec >> socket"  );
    return -1;
  }
  
  memset( &listen_addr , 0 , sizeof(struct sockaddr_un) );
  listen_addr.sun_family = AF_UNIX ;
  snprintf(listen_addr.sun_path , sizeof(listen_addr.sun_path)-1 , 
    "%s/containers/%s/kucker.socket", kucker_repo, arg.containerId );


  if( access( listen_addr.sun_path , F_OK ) == 0 )
  {
    unlink( listen_addr.sun_path );
  }
  if( bind( listenfd , (struct sockaddr *) &listen_addr , sizeof(struct sockaddr_un) ) == -1 )
  {
    err_exit(errno, "pty_proxy_exec >> bind %s", listen_addr.sun_path );
    return -1;
  }
  
  if( listen(listenfd , 1024 ) == -1 )
  {
    err_exit(errno, "pty_proxy_exec >> listen failed");
    return -1;
  }

  init_pool(listenfd, masterFd, &pool); 
  // 通知client
  SemUtil::set(arg.synchSem, 0);

  // sleep(2);
  // printf("2 Container pid=%d, pgrp=%d, tcgetpgrp=%d, getsid=%d, isatty=%d\n",
  //  getpid(), getpgrp(), tcgetpgrp(STDIN_FILENO), getsid(0), isatty(STDIN_FILENO));

  while (1) {
    /* Wait for listening/connected descriptor(s) to become ready */
    pool.ready_set = pool.read_set;
    if( (pool.nready = select(pool.maxfd + 1, &pool.ready_set, NULL, NULL, NULL)) == -1) {
      err_msg(errno, "server_proxy select");
      if(errno == EINTR) {
        continue;
      }
    }

    /* If listening descriptor ready, add new client to pool */
    if (FD_ISSET(listenfd, &pool.ready_set))   //line:conc:echoservers:listenfdready
    {
      clientlen = sizeof(struct sockaddr_un);

      if( (connfd = accept(listenfd,  (struct sockaddr *)&clientaddr, &clientlen)) != -1 ) {
        add_client(connfd, &pool);
      }
      
    }


    /* Echo a text line from each ready connected descriptor */
    check_clients_and_master(&pool); 
  }

  err_msg(0, "===========server exit================\n");

}


/* $begin init_pool */
void init_pool(int listenfd, int masterfd, pool *p)
{
  /* Initially, there are no connected descriptors */
  int i;
  p->maxi = -1;                   //line:conc:echoservers:beginempty
  for (i = 0; i < FD_SETSIZE; i++)
    p->clientfd[i] = -1;        //line:conc:echoservers:endempty

  /* Initially, listenfd is only member of select read set */
  p->maxfd = listenfd;            //line:conc:echoservers:begininit
  FD_ZERO(&p->read_set);
  FD_SET(listenfd, &p->read_set);

  FD_SET(masterfd, &p->read_set);
  p->masterfd = masterfd;
}


/* $begin add_client */
void add_client(int connfd, pool *p)
{
  int i;
  p->nready--;
  for (i = 0; i < FD_SETSIZE; i++)  /* Find an available slot */
    if (p->clientfd[i] < 0)
    {
      /* Add connected descriptor to the pool */
      p->clientfd[i] = connfd;

      /* Add the descriptor to descriptor set */
      FD_SET(connfd, &p->read_set);

      /* Update max descriptor and pool highwater mark */
      if (connfd > p->maxfd) 
        p->maxfd = connfd; 
      if (i > p->maxi)     
        p->maxi = i;      
      break;
    }
 
  if (i == FD_SETSIZE) /* Couldn't find an empty slot */
    err_msg(0, "add_client error: Too many clients");
}
/* $end add_client */

/* $begin check_clients */
void check_clients_and_master(pool *p)
{
  int i, connfd, n;
  char buf[BUF_SIZE];

  for (i = 0; (i <= p->maxi) && (p->nready > 0); i++)
  {
    connfd = p->clientfd[i];

    /* If the descriptor is ready, echo a text line from it */
    if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set)))
    {
      p->nready--;
      if ((n = read(connfd, buf, BUF_SIZE)) <= 0) {
        if(errno != EINTR) {
          close(connfd); 
          FD_CLR(connfd, &p->read_set);
          p->clientfd[i] = -1;
        }
       
      } else {
        if(rio_writen(p->masterfd, buf, n) <= 0) {
           exit(EXIT_SUCCESS);
        }
      }
    }
  }

  if (p->masterfd != -1 && FD_ISSET(p->masterfd,  &p->ready_set))         
  {
    p->nready--;
    if ((n = read(p->masterfd, buf, BUF_SIZE)) <= 0) {
// err_msg(0, "============%d=====before=====\n", 21);
      // close(p->masterfd); 
      // FD_CLR(p->masterfd, &p->read_set);
      // p->masterfd = -1;
      if(errno != EINTR) {
         exit(EXIT_SUCCESS);
      }
     
    } else {
      for (i = 0; i <= p->maxi; i++) {
        // === debug
        // if (rio_writen(STDOUT_FILENO, buf, n) != n) {
        //   err_msg(errno, "check_clients_and_master>>rio_writen STDOUT_FILENO");
        // }
        //====
        connfd = p->clientfd[i];
        if(connfd > 0) {
          if (rio_writen(connfd, buf, n) != n) {
            err_msg(errno, "client closed.check_clients_and_master>>rio_writen client");
            close(connfd); 
            FD_CLR(connfd, &p->read_set);
            p->clientfd[i] = -1;
            
          }
        }
      }
    }
  }

}



int pty_client(pty_exe_opt_t arg) {
  int clientfd;
  struct sockaddr_un  addr ;
  
  char buf[BUF_SIZE];
  ssize_t n;
  // struct request req;
  fd_set read_set, ready_set;

 

  clientfd = socket( AF_UNIX , SOCK_STREAM , 0) ;
  if( clientfd == -1 )
  {
    err_exit(errno, "pty_client socket failed\n"  );
    return -1;
  }
  memset( &addr, 0 , sizeof(struct sockaddr_un) );
  addr.sun_family = AF_UNIX ;
  snprintf( addr.sun_path , sizeof(addr.sun_path)-1 , "%s/containers/%s/kucker.socket", kucker_repo, arg.containerId );
  
  if(connect(clientfd , (struct sockaddr *) & addr , sizeof(struct sockaddr_un) ) == -1) {
    err_exit(errno, "pty_client connect");
  }
  
  
 /* Place terminal in raw mode so that we can pass all terminal
  input to the pseudoterminal master untouched */
  tcgetattr(STDIN_FILENO , &ttyOrig );
  memcpy( & g_termios_raw , &ttyOrig , sizeof(struct termios) );
  cfmakeraw( &g_termios_raw );
  tcsetattr( STDIN_FILENO , TCSANOW , &g_termios_raw ) ;

  if (atexit(ttyReset) != 0)
    err_msg(errno, "pty_client >> atexit");

  FD_ZERO(&read_set);
  FD_SET(STDIN_FILENO, &read_set);
  FD_SET(clientfd, &read_set);

  snprintf(buf, BUF_SIZE, "\n");
  if (write(clientfd, buf, strlen(buf)) !=  strlen(buf))
    err_msg(errno, "partial/failed write (masterFd)");
  /* Loop monitoring terminal and pty master for input. If the
     terminal is ready for input, then read some bytes and write
     them to the pty master. If the pty master is ready for input,
     then read some bytes and write them to the terminal. */
  while(1)
  {
    ready_set = read_set;
    if (select(clientfd + 1, &ready_set, NULL, NULL, NULL) == -1)
      err_msg(errno, "select");

    
    if (FD_ISSET(STDIN_FILENO, &ready_set))     /* stdin --> pty */
    {
      if (( n = read(STDIN_FILENO, buf, BUF_SIZE)) <= 0) {
        
        exit(EXIT_SUCCESS);
      }
        
      if (rio_writen(clientfd, buf, n) !=  n)
        err_exit(errno, "partial/failed write (masterFd)");
    }

    if (FD_ISSET(clientfd, &ready_set))        /* pty --> stdout+file */
    {
      if ((n = read(clientfd, buf, BUF_SIZE)) <= 0) {
        exit(EXIT_SUCCESS);
      }

      if (rio_writen(STDOUT_FILENO, buf, n) != n)
        err_exit(errno, "partial/failed write (STDOUT_FILENO)");
      
    }
  }

}



