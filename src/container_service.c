#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "pty_exec_util.h"
#include "logger_factory.h"
#include "sem_util.h"
#include "container_manager.h"
#include "container_service.h"
#include "path_assembler.h"

int synchSem;

static const char *containerId;


static void exitHandler(void)
{
  LoggerFactory::getDebugLogger().debug("exitHandler containerId=%s", containerId); 
  ContainerManager::umount_container(containerId);
  ContainerManager::change_status_to_stop(containerId);
}

static void sigTermHandler(int sig) {
  LoggerFactory::getDebugLogger().debug("sigTermHandler %s", strsignal(sig));
  exit(0);
}
 
// static void sigHupHandler(int sig) {
  
// }

// static void sigQuitHandler(int sig) {
//   std::cout << "sigQuitHandler \n" << std::endl;
// }

static int container_run_main(void* arg)
{

  //printf("in container...\n ");

  container_start_option_t startOpt = *((container_start_option_t *)arg);

  // signal for "kill 容器进程"
  containerId = startOpt.containerId;
  Signal(SIGTERM, sigTermHandler);
  // Signal(SIGHUP, sigHupHandler);
  // Signal(SIGQUIT, sigQuitHandler);

  if (atexit(exitHandler) != 0)
    err_msg(errno, "container_run_main >> atexit");

  ContainerManager::mount_container(startOpt.containerId);
  // 容器卷
  if (startOpt.volume != NULL )
  {
    ContainerManager::mount_volume(startOpt.containerId,  startOpt.volume );
  }


  pty_exe_opt_t ptyopt = {};
  ptyopt.synchSem = synchSem;
  ptyopt.containerId = startOpt.containerId;
  ptyopt.cmd = startOpt.cmd;
  ptyopt.detach = startOpt.detach;
  
  pty_proxy_exec(ptyopt);
 // pty_exec(ptyopt);

  return 1;
}


void ContainerService::start(container_start_option_t & startOpt,  std::function<void(int)>  startSuccess)
{

  synchSem = SemUtil::get(IPC_PRIVATE, 1);

  printf("Parent pid=%d, pgrp=%d, tcgetpgrp=%d, getsid=%d, isatty=%d\n",
   getpid(), getpgrp(), tcgetpgrp(STDIN_FILENO), getsid(0), isatty(STDIN_FILENO));
 
  // if (info.id.empty() || info.status == CONTAINER_RUNNING)
  //   continue;
  /* Create the child in new namespace(s) */
  void *stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
  if (stack == MAP_FAILED)
    err_exit(0, "handle_run_command mmap:");

  int containerPid = clone(container_run_main, (char *)stack + STACK_SIZE,
                            CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, &startOpt);

   
  printf("Parent pid [%5d] - Container pid[%5d]!\n", getpid(), containerPid);

  startSuccess(containerPid);
  // ------save end---------
  
  SemUtil::zero(synchSem);
  if( !startOpt.detach ) {
    pty_exe_opt_t execOpt = {};
    execOpt.containerId = startOpt.containerId;
    pty_client( execOpt) ;
  } else {
    printf("containerName=%s\n", startOpt.containerId);
  }
  
}

void ContainerService::stop(const std::string & name) {
  Container info = ContainerManager::get_container_by_id_or_name(name);
  if(info.id.empty() || info.status != CONTAINER_RUNNING) {
    err_msg(0, "No container identify by %s, or it's not a container in running.", name.c_str());
    return;
  }
  printf("Stopping container=%s, pid=%d\n",  name.c_str(), info.pid);
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