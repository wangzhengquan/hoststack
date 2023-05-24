#include <sys/mount.h>
#include <getopt.h>
#include <sys/syscall.h>

#include "container_dao.h"
#include "container_info.h"
#include "container_attach_cli.h"
#include "pty_exec_util.h"

void ContainerAttachCli::usage()
{
  fprintf(stderr, "Usage: kucker container attach CONTAINER\n\n");
  fprintf(stderr, "Attach local standard input, output, and error streams to a running container.\n\n");
  // fprintf(stderr, "Commands:\n\n");
  // #define fpe(str) fprintf(stderr, "  %s", str);
  // fpe("run         Run a command in a new container\n");
  // fpe("stop        Stop one or more running containers\n");
  // fpe("ls          List containers\n");
  // fpe("\n");
  // fprintf(stderr, "Run 'kucker container COMMAND --help' for more information on a command.\n");
}


void ContainerAttachCli::handleCommand(int argc,  char *argv[]) {

  if (argc < 2) {
    usage();
    return;
  }

  if(argc == 2 && strcmp(argv[1], "--help") == 0) {
    usage();
    return;
  }


  char *containerName = argv[1];
  ContainerInfo info = ContainerDao::get_container_by_id_or_name(containerName);
  if(info.status == CONTAINER_STOPED) {
    printf("Conatiner %s is not in running status!\n", info.name.c_str());
    return ;
  }

  pty_exe_opt_t execOpt = {};
  execOpt.detach = false;
  execOpt.containerId = info.id.c_str();
  pty_client( execOpt) ;
   
  
}
