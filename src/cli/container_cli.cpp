
#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <sys/syscall.h>

#include "container_info.h"
#include "container_cli.h"
#include "container_attach_cli.h"

// static char container_stack[STACK_SIZE];
void ContainerCli::usage()
{
  fprintf(stderr, "Usage: kucker container COMMAND\n\n");
  fprintf(stderr, "Manage containers.\n\n");
  fprintf(stderr, "Commands:\n\n");
  #define fpe(str) fprintf(stderr, "  %s", str);
  fpe("run         Run a command in a new container\n");
  fpe("stop        Stop one or more running containers\n");
  fpe("ls          List containers\n");
  fpe("attach      Attach local standard input, output, and error streams to a running container\n");
  fpe("start       Start one or more stopped containers\n");
  fpe("exec        Run a command in a running container\n");
  fpe("rm          Remove one or more containers\n");
  fpe("\n");
  fprintf(stderr, "Run 'kucker container COMMAND --help' for more information on a command.\n");
}

void ContainerCli::handleCommand(int argc, char *argv[]) {
	char *action;
  if (argc < 2)
  {
    usage();
    exit(1);
  }
  else
  {
    action = argv[1];
  }
  
	if (strcmp(action, "run") == 0) {
    ContainerRunCli::handleCommand(argc - 1, argv + 1);
  }
  else if (strcmp(action, "ls") == 0) {
    ContainerLsCli::handleCommand(argc - 1, argv + 1);
  } 
  else if (strcmp(action, "stop") == 0) {
    ContainerStopCli::handleCommand(argc - 1, argv + 1);
  }  
  else if (strcmp(action, "start") == 0) {
    ContainerStartCli::handleCommand(argc - 1, argv + 1);
  } 
  else if (strcmp(action, "exec") == 0) {
    ContainerExecCli::handleCommand(argc - 1, argv + 1);
  }
  else if (strcmp(action, "attach") == 0) {
    ContainerAttachCli::handleCommand(argc - 1, argv + 1);
  } 
  else if (strcmp(action, "rm") == 0) {
    ContainerRMCli::handleCommand(argc - 1, argv + 1);
  } 
  else if (strcmp(action, "--help") == 0)
  {
    usage();
  } else {
  	usage();
  	exit(1);
  }
}




