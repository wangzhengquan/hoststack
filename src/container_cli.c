
#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <sys/syscall.h>

#include "container_manager.h"
#include "container.h"
#include "container_cli.h"
#include "container_ls_cli.h"
#include "container_run_cli.h"
// static char container_stack[STACK_SIZE];
void ContainerCli::usage()
{
  printf("usage: param error\n");
}

void ContainerCli::handle_command(int argc, char *argv[]) {
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

	if (strcmp(action, "run") == 0)
  {
    ContainerRunCli::handle_command(argc - 1, argv + 1);
  }
  else if (strcmp(action, "ls") == 0)
  {
    ContainerLsCli::handle_command(argc - 1, argv + 1);
  } else {
  	usage();
  	exit(1);
  }
}



