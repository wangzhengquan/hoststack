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
/* $end echoserversmain */
void init_pool(int listenfd, int masterfd, pool *p);
void add_client(int connfd, int masterfd, pool *p);
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
 
  int outFd = open( garg.logfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
  if (outFd == -1)
      err_msg(errno, "pty_exec open log: %s", garg.logfile);
  dup2(outFd, STDOUT_FILENO);
  close(outFd);

  int inFd = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
  if (inFd == -1)
      err_msg(errno, "pty_exec open /dev/null");
  dup2(inFd, STDIN_FILENO);
  close(inFd);
}


static void sigStopHandler(int sig) {
  LoggerFactory::getDebugLogger().debug("pty_exec_util.sigStopHandler");
  ContainerManager::umount_container(containerId);
  ContainerManager::change_status_to_stop(containerId);
  // ContainerManager::stop(containerId);
}

static void sigHupHandler(int sig) {
  // std::cout << "sigHupHandler " << std::endl;
  LoggerFactory::getDebugLogger().debug("pty_exec_util.sigHupHandler logfile=%s", garg.logfile);
  // redirectStdOut();
  // ttyReset();
}

static void sigQuitHandler(int sig) {
  std::cout << "sigQuitHandler \n" << std::endl;
}

int pty_exec(pty_exe_opt_t arg)
{

  char slaveName[MAX_SNAME];
  int masterFd;
 
  struct winsize ws;
  fd_set ready_set;
  char buf[BUF_SIZE];
  ssize_t numRead;
  pid_t childPid;

  garg = arg;
  /* Retrieve the attributes of terminal on which we are started */

  if (tcgetattr(STDIN_FILENO, &ttyOrig) == -1)
    err_msg(errno, "tcgetattr");
  if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0)
    err_msg(errno, "ioctl-TIOCGWINSZ");

  // if (setpgid(0, 0) == -1) {
  //   err_msg(errno, "pty_exec:setsid");
  //   LoggerFactory::getDebugLogger().debug("error pty_exec_util.setsid");
  // }

  // signal for "kill 容器进程"
  containerId = arg.containerId;
  Signal(SIGTERM, sigStopHandler);
  Signal(SIGHUP, sigHupHandler);
  // Signal(SIGQUIT, sigQuitHandler);

  /* Create a child process, with parent and child connected via a
     pty pair. The child is connected to the pty slave and its terminal
     attributes are set to be the same as those retrieved above. */

  childPid = ptyFork(&masterFd, slaveName, MAX_SNAME, &ttyOrig, &ws);
  if (childPid == -1)
    err_msg(errno, "ptyFork");

  if (childPid == 0)          /* Child: execute a shell on pty slave */
  {
    /* chroot 隔离目录 */
    if ( chdir(arg.rootfs) != 0 || chroot("./") != 0 )
    {
      err_msg(errno, "chdir/chroot:%s", arg.rootfs);
    }


    if (system("touch /usr/lib/tmp") != 0)
    {
      err_msg(errno, "touch /usr/lib/tmp");
    }

    execvp(arg.cmd[0], arg.cmd);
    err_msg(errno, "execvp: %s\n", arg.cmd[0]);
  }


  /* Parent: relay data between terminal and pty master */
  if(arg.detach) {
    redirectStdOut();
  }
 
  if(!arg.detach) {
     /* Place terminal in raw mode so that we can pass all terminal
     input to the pseudoterminal master untouched */

    ttySetRaw(STDIN_FILENO, &ttyOrig);

    if (atexit(ttyReset) != 0)
      err_msg(errno, "atexit");
  }
 

  /* Loop monitoring terminal and pty master for input. If the
     terminal is ready for input, then read some bytes and write
     them to the pty master. If the pty master is ready for input,
     then read some bytes and write them to the terminal. */
  sigset_t selectBlockSet;
  sigemptyset(&selectBlockSet);
  sigaddset(&selectBlockSet, SIGHUP);
  for (;;)
  {
    FD_ZERO(&ready_set);
    FD_SET(STDIN_FILENO, &ready_set);
    FD_SET(masterFd, &ready_set);


    if (pselect(masterFd + 1, &ready_set, NULL, NULL, NULL, &selectBlockSet) == -1) {
      
      if(errno == EINTR) {
        err_msg(errno, "pty_exec select EINTR");
        continue;
      } else {
        err_msg(errno, "pty_exec select");
      }
    }

    
    if (FD_ISSET(STDIN_FILENO, &ready_set))     /* stdin --> pty */
    {
      numRead = read(STDIN_FILENO, buf, BUF_SIZE);
      if (numRead <= 0) {
       // exit(EXIT_SUCCESS);
      }

      if (write(masterFd, buf, numRead) != numRead)
        err_msg(errno, "partial/failed write (masterFd)");
    }

    if (FD_ISSET(masterFd, &ready_set))        /* pty --> stdout+file */
    {
      numRead = read(masterFd, buf, BUF_SIZE);
      if (numRead <= 0) {
        exit(EXIT_SUCCESS);
      }
      if (write(STDOUT_FILENO, buf, numRead) != numRead)
          err_msg(errno, "partial/failed write (STDOUT_FILENO)");
     
      
    }
  }
}





int pty_proxy_exec(pty_exe_opt_t arg)
{
  char slaveName[MAX_SNAME];
  int masterFd;
  struct winsize ws;
 
  pid_t childPid;
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

  if (childPid == 0)          /* Child: execute a shell on pty slave */
  {
    /* chroot 隔离目录 */
    if ( chdir(arg.rootfs) != 0 || chroot("./") != 0 )
    {
      err_msg(errno, "chdir/chroot:%s", arg.rootfs);
    }


    if (system("touch /usr/lib/tmp") != 0)
    {
      err_msg(errno, "touch /usr/lib/tmp");
    }

    execvp(arg.cmd[0], arg.cmd);
    err_msg(errno, "execvp: %s\n", arg.cmd[0]);
  }

 
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)    err_msg(errno, "signal");

  int     listenfd , connfd;
  struct sockaddr_un  listen_addr, clientaddr ;
  int nret;
  static pool pool;
  socklen_t     clientlen ;
 

  /* Create well-known FIFO, and open it for reading */
  listenfd = socket( AF_UNIX , SOCK_STREAM , 0 ) ;
  if( listenfd == -1 )
  {
    err_exit( errno, "*** ERROR : socket failed , errno[%d]\n"  );
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
    err_exit(errno, "*** ERROR : bind failed , errno[%d]\n"  );
    return -1;
  }
  
  if( listen( listenfd , 1024 ) == -1 )
  {
    err_exit(errno, "*** ERROR : listen failed , errno[%d]\n"  );
    return -1;
  }

  init_pool(listenfd, masterFd, &pool); 
  // 通知client
  SemUtil::set(arg.synchSem, 0);
  while (1) {
    /* Wait for listening/connected descriptor(s) to become ready */
    pool.ready_set = pool.read_set;
    if( (pool.nready = select(pool.maxfd + 1, &pool.ready_set, NULL, NULL, NULL)) == -1) {
      err_msg(errno, "server_proxy select");
    }

    /* If listening descriptor ready, add new client to pool */
    if (FD_ISSET(listenfd, &pool.ready_set))   //line:conc:echoservers:listenfdready
    {

      clientlen = sizeof(struct sockaddr_un);

      if( (connfd = accept(listenfd,  (struct sockaddr *)&clientaddr, &clientlen)) != -1 ) {
        add_client(connfd, masterFd, &pool);
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
  // FD_SET(masterfd, &p->read_set);

  p->masterfd = -1;
}


/* $begin add_client */
void add_client(int connfd, int masterfd, pool *p)
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

  if(p->masterfd == -1) {
    p->masterfd = masterfd;
    FD_SET(masterfd, &p->read_set);
  }
 
  if (i == FD_SETSIZE) /* Couldn't find an empty slot */
    err_msg(0, "add_client error: Too many clients");
}
/* $end add_client */

/* $begin check_clients */
void check_clients_and_master(pool *p)
{
  int i, connfd, n;
  char buf[MAXLINE];

  for (i = 0; (i <= p->maxi) && (p->nready > 0); i++)
  {
    connfd = p->clientfd[i];

    /* If the descriptor is ready, echo a text line from it */
    if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set)))
    {
      p->nready--;
// err_msg(0, "============%d==========\n", 1);
      if ((n = read(connfd, buf, MAXLINE)) <= 0) {
// err_msg(0, "============%d===before=======\n", 11);
        close(connfd); 
        FD_CLR(connfd, &p->read_set);
        p->clientfd[i] = -1;
// err_msg(0, "============%d====after=====\n", 11);
        
      } else {
// err_msg(0, "============%d===before=======\n", 12);
        // printf("Server received %d  bytes on fd %d\n", n, connfd);
        if(rio_writen(p->masterfd, buf, n) < 0) {
           exit(EXIT_SUCCESS);
        }
// err_msg(0, "============%d====after======\n", 12);
      }
    }
  }

  if (p->masterfd != -1 && FD_ISSET(p->masterfd,  &p->ready_set))         
  {
    p->nready--;
// err_msg(0, "============%d==========\n", 2);
    if ((n = read(p->masterfd, buf, MAXLINE)) <= 0) {
// err_msg(0, "============%d=====before=====\n", 21);
      // close(p->masterfd); 
      // FD_CLR(p->masterfd, &p->read_set);
      // p->masterfd = -1;
      exit(EXIT_SUCCESS);
// err_msg(0, "============%d=====after=====\n", 21);
    } else {
// err_msg(0, "============%d====before======\n", 22);        
      for (int j = 0; j <= p->maxi; j++) {
        if(p->clientfd[j] > 0) {
          Rio_writen(p->clientfd[j], buf, n);
        }
      }
// err_msg(0, "============%d====after======\n", 22);    

    }
  }

}



int pty_client(pty_exe_opt_t arg) {
  int clientfd;
  struct sockaddr_un  addr ;
  int logFd;
  
  char buf[BUF_SIZE];
  ssize_t numRead;
  // struct request req;
  fd_set read_set, ready_set;
  /* Place terminal in raw mode so that we can pass all terminal
  input to the pseudoterminal master untouched */

  tcgetattr( STDIN_FILENO , &ttyOrig );
  memcpy( & g_termios_raw , &ttyOrig , sizeof(struct termios) );
  cfmakeraw( &g_termios_raw );
  tcsetattr( STDIN_FILENO , TCSANOW , &g_termios_raw ) ;

  if (atexit(ttyReset) != 0)
    err_msg(errno, "atexit");

  clientfd = socket( AF_UNIX , SOCK_STREAM , 0) ;
  if( clientfd == -1 )
  {
    err_exit(errno, "*** ERROR : socket failed , errno[%d]\n"  );
    return -1;
  }
  memset( &addr, 0 , sizeof(struct sockaddr_un) );
  addr.sun_family = AF_UNIX ;
  snprintf( addr.sun_path , sizeof(addr.sun_path)-1 , "%s/containers/%s/kucker.socket", kucker_repo, arg.containerId );
  

  if(connect(clientfd , (struct sockaddr *) & addr , sizeof(struct sockaddr_un) ) == -1) {
    err_exit(errno, "pty_client connect");
  }
  
   

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
      if (( numRead = read(STDIN_FILENO, buf, BUF_SIZE)) <= 0) {
        
        exit(EXIT_SUCCESS);
      }
        
      if (rio_writen(clientfd, buf, numRead) !=  numRead)
        err_exit(errno, "partial/failed write (masterFd)");
    }

    if (FD_ISSET(clientfd, &ready_set))        /* pty --> stdout+file */
    {
      if ((numRead = read(clientfd, buf, BUF_SIZE)) <= 0) {
        exit(EXIT_SUCCESS);
      }

      if (rio_writen(STDOUT_FILENO, buf, numRead) != numRead)
        err_exit(errno, "partial/failed write (STDOUT_FILENO)");
      
    }
  }

}



