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
#define BUF_SIZE 4096
#define MAX_SNAME 1000

struct request {                /* Request (client --> server) */
  char *clientFifo;                 /* fifo of client */
  char buf[BUF_SIZE];
  int bufSize;
};

struct termios ttyOrig;

pty_exe_opt_t garg;

static std::string containerName;


static void             /* Reset terminal mode on program exit */
ttyReset(void)
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
  ContainerManager::umount_container(containerName);
  ContainerManager::change_status_to_stop(containerName);
  // ContainerManager::stop(containerName);
}

static void sigHupHandler(int sig) {
  // std::cout << "sigHupHandler " << std::endl;
  LoggerFactory::getDebugLogger().debug("pty_exec_util.sigHupHandler logfile=%s", garg.logfile);
  //redirectStdOut();
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
  containerName = arg.containerName;
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
    // close(logFd);
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
    err_msg(0, "for\n");
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
      if(!arg.detach) {
        numRead = read(STDIN_FILENO, buf, BUF_SIZE);
        if (numRead <= 0) {
          exit(EXIT_SUCCESS);
        }

        if (write(masterFd, buf, numRead) != numRead)
          err_msg(errno, "partial/failed write (masterFd)");
      }
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



typedef struct { /* Represents a pool of connected descriptors */ //line:conc:echoservers:beginpool
  int maxfd;        /* Largest descriptor in read_set */
  fd_set read_set;  /* Set of all active descriptors */
  fd_set ready_set; /* Subset of descriptors ready for reading  */
  int nready;       /* Number of ready descriptors from select */
  int maxi;         /* Highwater index into client array */
  int clientfd[FD_SETSIZE];    /* Set of active descriptors */
  rio_t clientrio[FD_SETSIZE]; /* Set of active read buffers */
  rio_t masterrio;
  int masterfd;
} pool; //line:conc:echoservers:endpool
/* $end echoserversmain */
void init_pool(int listenfd, pool *p);
void add_client(int connfd, pool *p);
void check_clients(pool *p);




/* $begin init_pool */
void init_pool(int listenfd, int masterfd, pool *p)
{
  /* Initially, there are no connected descriptors */
  int i;
  p->maxi = -1;                   //line:conc:echoservers:beginempty
  for (i = 0; i < FD_SETSIZE; i++)
    p->clientfd[i] = -1;        //line:conc:echoservers:endempty

  /* Initially, listenfd is only member of select read set */
  p->maxfd = MAX(listenfd, masterfd);            //line:conc:echoservers:begininit
  FD_ZERO(&p->read_set);
  FD_SET(listenfd, &p->read_set);  
  FD_SET(masterfd, &p->read_set);

  p->masterfd = masterfd;
  Rio_readinitb(&p->masterrio, masterfd); 
}
/* $end init_pool */

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
      Rio_readinitb(&p->clientrio[i], connfd); 

      /* Add the descriptor to descriptor set */
      FD_SET(connfd, &p->read_set); //line:conc:echoservers:addconnfd

      /* Update max descriptor and pool highwater mark */
      if (connfd > p->maxfd) //line:conc:echoservers:beginmaxfd
        p->maxfd = connfd; //line:conc:echoservers:endmaxfd
      if (i > p->maxi)       //line:conc:echoservers:beginmaxi
        p->maxi = i;       //line:conc:echoservers:endmaxi
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
  char buf[MAXLINE];
  rio_t rio;

  for (i = 0; (i <= p->maxi) && (p->nready > 0); i++)
  {
    connfd = p->clientfd[i];
    rio = p->clientrio[i];

    /* If the descriptor is ready, echo a text line from it */
    if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set)))
    {
      p->nready--;
      if ((n = Rio_readn(connfd, buf, MAXLINE)) <= 0) {
        close(connfd); 
        FD_CLR(connfd, &p->read_set);
        p->clientfd[i] = -1;
        
      } else {
        printf("Server received %d  bytes on fd %d\n", n, connfd);
        Rio_writen(p->masterfd, buf, n);
      }
    }

    if (FD_ISSET(p->masterfd,  &p->ready_set))         
    {
      p->nready--;
      if ((n = Rio_readn(p->masterfd, buf, MAXLINE)) <= 0) {
        err_msg(0, "read mater <= 0");
        exit(EXIT_SUCCESS);
      } else {
        
        for (int j = 0; j <= p->maxi; j++) {
          Rio_writen(p->clientfd[i], buf, n);
        }
  
      }
    }
  }


  // for (;;)
  // {
    

  //   if (select(maxFd + 1, &ready_set, NULL, NULL, NULL) == -1)
  //     err_msg(errno, "select");

  //   // 收到client请求
  //   if (FD_ISSET(serverFd, &ready_set))     /* stdin --> pty */
  //   {
  //       numRead = read(serverFd, &req, sizeof(struct request));
  //       if (numRead != sizeof(struct request))
  //         exit(EXIT_SUCCESS);

  //       if (write(masterFd, req.buf, req.bufSize) != req.bufSize)
  //         err_msg(errno, "partial/failed write (masterFd)");
  //   }

  //   if (FD_ISSET(masterFd, &ready_set))         pty --> stdout+file 
  //   {
  //     numRead = read(masterFd, buf, BUF_SIZE);
  //     if (numRead <= 0)
  //       exit(EXIT_SUCCESS);
  //     clientFd = open(req.clientFifo, O_WRONLY);
  //     if (clientFd == -1) {           /* Open failed, give up on client */
  //         err_msg(errno, "open %s", arg.clientFifo);
  //         continue;
  //     }
  //     if (write(clientFd, buf, numRead) != numRead)
  //         err_msg(errno, "partial/failed write (STDOUT_FILENO)");
  //     if (close(clientFd) == -1)
  //         err_msg(errno, "close");
  //   }
  // }
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
  // if( access( listen_addr.sun_path , F_OK ) == 0 )
  // {
  //   unlink( listen_addr.sun_path );
  // }
  if( bind( listenfd , (struct sockaddr *) &listen_addr , sizeof(struct sockaddr_un) ) == -1 )
  {
    err_exit(errno, "*** ERROR : bind failed , errno[%d]\n"  );
    return -1;
  }
  
  nret = listen( listenfd , 1024 ) ;
  if( nret == -1 )
  {
    err_exit(errno, "*** ERROR : listen failed , errno[%d]\n"  );
    return -1;
  }

  init_pool(listenfd, masterFd, &pool); 

  while (1) {
    /* Wait for listening/connected descriptor(s) to become ready */
    pool.ready_set = pool.read_set;
    if( (pool.nready = select(pool.maxfd + 1, &pool.ready_set, NULL, NULL, NULL)) == -1) {
      err_msg(errno, "server_proxy select");
    }

    /* If listening descriptor ready, add new client to pool */
    if (FD_ISSET(listenfd, &pool.ready_set))   //line:conc:echoservers:listenfdready
    {

      clientlen = sizeof(struct sockaddr_un) ;;
      if( (connfd = accept(listenfd,  (struct sockaddr *)&clientaddr, &clientlen)) != -1 ) {
        add_client(connfd, &pool); //line:conc:echoservers:addclient
      }
      
    }

    /* Echo a text line from each ready connected descriptor */
    check_clients_and_master(&pool); 
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

  ttySetRaw(STDIN_FILENO, &ttyOrig);

  if (atexit(ttyReset) != 0)
    err_msg(errno, "atexit");

  clientfd = socket( AF_UNIX , SOCK_STREAM , 0 ) ;
  if( clientfd == -1 )
  {
    err_exit(errno, "*** ERROR : socket failed , errno[%d]\n"  );
    return -1;
  }
  memset( &addr , 0 , sizeof(struct sockaddr_un) );
  addr.sun_family = AF_UNIX ;
  snprintf( addr.sun_path , sizeof(addr.sun_path)-1 , "%s/containers/%s/kucker.socket", kucker_repo, arg.containerId );
  if(connect( clientfd , (struct sockaddr *) & addr , sizeof(struct sockaddr_un) ) == -1) {
    err_exit(errno, "pty_client connect");
  }
  
  
  


  if(arg.detach) {
    logFd = open( arg.logfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (logFd == -1)
        err_msg(errno, "open typescript");
  }

  FD_ZERO(&read_set);
  FD_SET(STDIN_FILENO, &read_set);
  FD_SET(clientfd, &read_set);

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
      if(!arg.detach) {
       
        if (( numRead = read(STDIN_FILENO, buf, BUF_SIZE)) <= 0)
          exit(EXIT_SUCCESS);
        
        if (write(clientfd, buf, numRead) !=  numRead)
          err_msg(errno, "partial/failed write (masterFd)");
      }
    }

    if (FD_ISSET(clientfd, &ready_set))        /* pty --> stdout+file */
    {
      if ((numRead = read(clientfd, buf, BUF_SIZE)) <= 0)
        exit(EXIT_SUCCESS);
      if(arg.detach) {
        if (write(logFd, buf, numRead) != numRead)
          err_msg(errno, "partial/failed write (logFd)");
      } else {
        if (write(STDOUT_FILENO, buf, numRead) != numRead)
          err_msg(errno, "partial/failed write (STDOUT_FILENO)");
      }
     
      
    }
  }

}
