#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sched.h>

#include "pty_exec_util.h"
#include "logger_factory.h"
#include "sem_util.h"
#include "container_dao.h"
#include "container_fs.h"
#include "container_service.h"
#include "path_assembler.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> // For directory traversal
#include <ctype.h>  // For isdigit()
#include <unistd.h> // For getpid(), fork(), sleep()
#include <sys/types.h>
#include <sys/wait.h> // For wait()

int synchSem;

const char *containerId = NULL;
int containerIdSem = SemUtil::get(IPC_PRIVATE, 1);
pid_t containerPid;
 
static void exitHandler(void)
{

  // LoggerFactory::getRunLogger().debug("----exitHandler");
  // std::optional<ContainerInfo> container = ContainerDao::get_container_by_id_or_name(containerId);
  sleep(2);
  if(containerId != NULL) {
    LoggerFactory::getRunLogger().debug("exitHandler containerId=%s", containerId); 
    ContainerFs::umount_container(containerId);
    ContainerDao::change_status_to_stop(containerId);
    
  }
 
}

static void sigTermHandler(int sig) {
  LoggerFactory::getRunLogger().debug("sigTermHandler %s", strsignal(sig));
}
 
static void sigHupHandler(int sig) {
  LoggerFactory::getRunLogger().debug("sigHupHandler %s", strsignal(sig));

}

static void sigQuitHandler(int sig) {
  LoggerFactory::getRunLogger().debug("sigQuitHandler %s", strsignal(sig));
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
      // TODO: 在`./hoststack exec`的进程未结束的情况下, 容器exit后未执行该sigchld_handler函数。
      LoggerFactory::getRunLogger().debug("pid(%d) normally exit by signal %d\r\n", pid, WEXITSTATUS(status));
      if (containerId != NULL){
        ContainerFs::umount_container(containerId);
        ContainerDao::change_status_to_stop(containerId);
        containerId = NULL;
      }
      
    }
    else if (WIFSIGNALED(status))
    {
      LoggerFactory::getRunLogger().debug("pid(%d) terminated by signal %d\r\n", pid, WTERMSIG(status));
    }
    else if (WIFSTOPPED(status))
    {
      LoggerFactory::getRunLogger().debug("pid(%d) stopted by signal %d\r\n",  pid, WSTOPSIG(status));
    }
    else if (WIFCONTINUED(status))
    {
      LoggerFactory::getRunLogger().debug("pid(%d) continued by signal %d\r\n",  pid, sig);
    } else {
      LoggerFactory::getRunLogger().debug("pid(%d) signal %d\r\n",  pid, sig);
    }
  }

  if (pid == -1 && errno != ECHILD)
  {
    err_msg(errno, "sigchld_handler error");
  }
  errno = olderrno;
}
 


void ContainerService::start(container_start_option_t & opt,  std::function<void(int)> startSuccess)
{
  synchSem = SemUtil::get(IPC_PRIVATE, 1);

  pid_t childPid = fork();

  if (childPid == -1)                 /* fork() failed */
  {
    err_exit(errno, "fork");
    return;
  }

  if (childPid == 0)                  
  {
    // signal for "kill 容器进程"
    containerId = opt.containerId;
    // if (atexit(exitHandler) != 0)
    //   err_msg(errno, "container_run_main >> atexit");
    // Signal(SIGTERM, sigTermHandler);
    // Signal(SIGHUP, sigHupHandler);
    // Signal(SIGQUIT, sigQuitHandler);

    // Signal(SIGCHLD, sigchld_handler);

    pty_exe_opt_t ptyopt = {};
    ptyopt.synchSem = synchSem;
    ptyopt.containerId = opt.containerId;
    ptyopt.cmd = opt.cmd;
    ptyopt.detach = opt.detach;
    ptyopt.volume_list = opt.volume_list;
    ptyopt.ttyAttr = opt.ttyAttr;
    ptyopt.ttyWs = opt.ttyWs;
    
    pty_run_container(ptyopt, [&](pid_t pid){
      containerPid = pid;
      startSuccess(pid);
    });
    
    return ;                
  }

  SemUtil::zero(synchSem);
  if( !opt.detach ) {
    pty_exe_opt_t execOpt = {};
    execOpt.containerId = opt.containerId;
    pty_client( execOpt) ;
  } else {
    printf("%s\n", opt.containerId);
  }
  
}

void ContainerService::stop(const std::string & name) {
  auto info = ContainerDao::get_container_by_id_or_name(name);
   
  if (!info) {
    fprintf(stderr, "Container %s not found!\n", name.c_str());
    return;
  }
  if(info->status != CONTAINER_RUNNING) {
    fprintf(stderr, "Conatiner %s is not in running status!\n", name.c_str());
    return ;
  }
  LoggerFactory::getRunLogger().info("Stopping container=%s, pid=%d\n",  name.c_str(), info->pid);
  if(kill(info->pid, SIGTERM) != 0) {
    err_msg(errno, "SIGTERM Stop container %s failed.pid = %d", name.c_str(), info->pid);
    return;
  }
  err_msg(0, "stoping %s", name.c_str());
  sleep(3);
  if(kill(info->pid, SIGKILL) != 0) {
    err_msg(errno, "SIGKILL Stop container %s failed.", name.c_str());
  }
  //sleep(1);
}


void ContainerService::exec(container_exec_option_t & opt) {
  
  char nspath[1024];
  const char *namespaces[] = {"pid", "mnt", NULL};
  int i = 0, fd;
  if(opt.detach) {
    if(daemon(0, 1) != 0) {
      err_exit(errno, "conatiner_exec_cli >> daemon");
    }
  }
  
  // setpgid(0, 0);
  while(namespaces[i] != NULL) {
    sprintf(nspath, "/proc/%d/ns/%s",  opt.containerPid, namespaces[i]);
    if( (fd = open(nspath, O_RDONLY)) == -1 ) {
        err_exit(errno, "ContainerExecCli >> open %s", nspath);
    }
    if(setns(fd, 0) == -1) {
        err_exit(errno, "ContainerExecCli >> setns:");
    }
    close(fd);
    i++;
  }
  pty_exe_opt_t ptyopt = {};
  ptyopt.containerId = opt.containerId;
  ptyopt.cmd = opt.cmd;
  ptyopt.detach = opt.detach;
  ptyopt.ttyAttr = opt.ttyAttr;
  ptyopt.ttyWs = opt.ttyWs;
// printf("=====ptyopt.containerId=%s\n", ptyopt.containerId);
  // containerId = opt.containerId;
  Signal(SIGCHLD, sigchld_handler);
  pty_exec(ptyopt);

}