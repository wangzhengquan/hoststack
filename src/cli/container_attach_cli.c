#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <sys/syscall.h>

#include "container_manager.h"
#include "container.h"
#include "container_attach_cli.h"
#include "pty_exec_util.h"

void ContainerAttachCli::usage()
{
  printf("usage: param error\n");
}


void ContainerAttachCli::handleCommand(int argc, char *argv[]) {

  if (argc < 2) {
    usage();
    return;
  }

  char *containerName = argv[1];
  Container info = ContainerManager::get_container_by_id_or_name(containerName);

  pty_exe_opt_t execOpt = {};
  execOpt.detach = false;
  execOpt.containerId = info.id.c_str();
  pty_client( execOpt) ;
   
  
}
