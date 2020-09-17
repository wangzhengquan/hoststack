
#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <sys/syscall.h>

#include "container_manager.h"
#include "container.h"
#include "container_cli.h"
#include "container_attach_cli.h"

// static char container_stack[STACK_SIZE];
void ContainerCli::usage()
{
  fprintf(stderr, "Usage: docker container COMMAND\n\n");
  fprintf(stderr, "Manage containers.\n\n");
  fprintf(stderr, "Commands:\n\n");
  #define fpe(str) fprintf(stderr, "  %s", str);
  fpe("run         Run a command in a new container\n");
  fpe("stop        Stop one or more running containers\n");
  fpe("ls          List containers\n");
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
  
printf("action=%s\n", action);
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
  else if (strcmp(action, "--help") == 0)
  {
    usage();
  } else {
  	usage();
  	exit(1);
  }
}




