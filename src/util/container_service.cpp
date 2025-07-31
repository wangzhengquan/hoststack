#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "pty_exec_util.h"
#include "logger_factory.h"
#include "sem_util.h"
#include "container_dao.h"
#include "container_fs.h"
#include "container_service.h"
#include "path_assembler.h"

int synchSem;

static const char *containerId = NULL;


// static void exitHandler(void)
// {
 
//   if(containerId != NULL) {
//     ContainerFs::umount_container(containerId);
//     ContainerDao::change_status_to_stop(containerId);
//     // LoggerFactory::getRunLogger().debug("exitHandler containerId=%s", containerId); 
//   } else {
//     // LoggerFactory::getRunLogger().debug("exitHandler containerId=NULL"); 
//   }
 
// }

// static void sigTermHandler(int sig) {
//   LoggerFactory::getRunLogger().debug("sigTermHandler %s", strsignal(sig));
//   exit(0);
// }
 
static void sigHupHandler(int sig) {
   std::cout << "sigHupHandler \n" << std::endl;

}

static void sigQuitHandler(int sig) {
  std::cout << "sigQuitHandler \n" << std::endl;
}



/*
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.
 */
static void sigchld_handler(int sig)
{
  // printf("%ld interrupt by SIGCHLD(%d)\n", time(NULL), sig);

  int olderrno = errno;
  pid_t pid;
  int status;
  while ( (pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0)
  {
    if (WIFEXITED(status))
    {
      printf("(%d) normally exit by signal %d\r\n", pid, WEXITSTATUS(status));
      ContainerFs::umount_container(containerId);
      ContainerDao::change_status_to_stop(containerId);
      exit(0);
    }
    else if (WIFSIGNALED(status))
    {
      // printf(" (%d) terminated by signal %d\n", pid, WTERMSIG(status));
    }
    else if (WIFSTOPPED(status))
    {
      // printf(" (%d) stopted by signal %d\n",  pid, WSTOPSIG(status));
    }
    else if (WIFCONTINUED(status))
    {
    }
  }

  if (pid == -1 && errno != ECHILD)
  {
    err_msg(errno, "sigchld_handler error");
  }
  errno = olderrno;
}
 


void ContainerService::start(container_start_option_t & startOpt,  std::function<void(int)> startSuccess)
{
  pid_t childPid;
  synchSem = SemUtil::get(IPC_PRIVATE, 1);

  childPid = fork();

  if (childPid == -1)                 /* fork() failed */
  {
    err_exit(errno, "ContainerService::start fork");
    return;
  }

  if (childPid == 0)                  
  {
    // signal for "kill 容器进程"
    containerId = startOpt.containerId;
    // Signal(SIGTERM, sigTermHandler);
    // Signal(SIGHUP, sigHupHandler);
    // Signal(SIGQUIT, sigQuitHandler);

    // if (atexit(exitHandler) != 0)
    //   err_msg(errno, "container_run_main >> atexit");

    Signal(SIGCHLD, sigchld_handler);

    pty_exe_opt_t ptyopt = {};
    ptyopt.synchSem = synchSem;
    ptyopt.containerId = startOpt.containerId;
    ptyopt.cmd = startOpt.cmd;
    ptyopt.detach = startOpt.detach;
    ptyopt.volume_list = startOpt.volume_list;
    ptyopt.ttyAttr = startOpt.ttyAttr;
    ptyopt.ttyWs = startOpt.ttyWs;
    
    pty_run_container(ptyopt, [&](pid_t cpid) {
      startSuccess(cpid);
    });
    
    return ;                
  }

  
  // LoggerFactory::getRunLogger().info("Container pid[%d]!\n", containerPid);

 
  SemUtil::zero(synchSem);
  if( !startOpt.detach ) {
    pty_exe_opt_t execOpt = {};
    execOpt.containerId = startOpt.containerId;
    pty_client( execOpt) ;
  } else {
    printf("%s\n", startOpt.containerId);
  }
  
}

void ContainerService::stop(const std::string & name) {
  ContainerInfo info = ContainerDao::get_container_by_id_or_name(name);
  if(info.id.empty() || info.status != CONTAINER_RUNNING) {
    printf( "No container identify by %s, or it's not a container in running.", name.c_str());
    return;
  }
  LoggerFactory::getRunLogger().info("Stopping container=%s, pid=%d\n",  name.c_str(), info.pid);
  if(kill(info.pid, SIGTERM) != 0) {
    err_msg(errno, "SIGTERM Stop container %s failed.pid = %d", name.c_str(), info.pid);
    return;
  }

  sleep(3);
  if(kill(info.pid, SIGKILL) != 0) {
    //err_msg(errno, "SIGKILL Stop container %s failed.", name.c_str());
  }
  //sleep(1);
}