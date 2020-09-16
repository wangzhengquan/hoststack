#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "pty_exec_util.h"
#include "logger_factory.h"
#include "sem_util.h"
#include "container_manager.h"
#include "container_service.h"

int synchSem;

static const char *containerId;

static void sigStopHandler(int sig) {
  LoggerFactory::getDebugLogger().debug("pty_exec_util.sigStopHandler");
  ContainerManager::umount_container(containerId);
  ContainerManager::change_status_to_stop(containerId);
  // ContainerManager::stop(containerId);
}

static void sigHupHandler(int sig) {
  // std::cout << "sigHupHandler " << std::endl;
 // LoggerFactory::getDebugLogger().debug("pty_exec_util.sigHupHandler logfile=%s", garg.logfile);
  // redirectStdOut();
  // ttyReset();
}

static void sigQuitHandler(int sig) {
  std::cout << "sigQuitHandler \n" << std::endl;
}

static int container_run_main(void* arg)
{

  printf("in container...\n ");
  container_start_option_t startOpt = *((container_start_option_t *)arg);

  // signal for "kill 容器进程"
  containerId = startOpt.containerId;
  // Signal(SIGTERM, sigStopHandler);
  // Signal(SIGHUP, sigHupHandler);
  // Signal(SIGQUIT, sigQuitHandler);

  ContainerManager::mount_container(startOpt.containerId);
  // 容器卷
  if (startOpt.volume != NULL )
  {
    ContainerManager::mount_volume(startOpt.containerId,  startOpt.volume );
  }


  char rootfs[1024];
  char logfile[1024];

  pty_exe_opt_t ptyopt = {};
  ptyopt.synchSem = synchSem;
  ptyopt.containerId = startOpt.containerId;
  sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, startOpt.containerId);
  ptyopt.rootfs = rootfs;
  ptyopt.cmd = startOpt.cmd;
  ptyopt.detach = startOpt.detach;
  // stdout 重定向日志文件
  sprintf(logfile, "%s/containers/%s/stdout.%ld.log", kucker_repo, startOpt.containerId, time(0));
  ptyopt.logfile = logfile;
  printf("logfile = %s\n", ptyopt.logfile);
  
  pty_proxy_exec(ptyopt);
 // pty_exec(ptyopt);

  return 1;
}


void ContainerService::start(container_start_option_t & startOpt,  std::function<void(int)>  startSuccess)
{

  synchSem = SemUtil::get(IPC_PRIVATE, 1);

 
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
  printf("killing pid %d\n", info.pid);
  if(kill(info.pid, SIGTERM) != 0) {
    err_msg(errno, "SIGTERM Stop container %s failed.", name.c_str());
    return;
  }

  sleep(3);
  if(kill(info.pid, SIGKILL) != 0) {
    //err_msg(errno, "SIGKILL Stop container %s failed.", name.c_str());
  }
}