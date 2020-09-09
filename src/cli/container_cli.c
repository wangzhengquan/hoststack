
#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <sys/syscall.h>

#include "container_manager.h"
#include "container.h"
#include "container_cli.h"

// static char container_stack[STACK_SIZE];
void ContainerCli::usage()
{
  fprintf(stderr, "Usage: docker container COMMAND\n\n");
  fprintf(stderr, "Manage containers.\n\n");
  fprintf(stderr, "Commands:\n\n");
  #define fpe(str) fprintf(stderr, "  %s", str);
  fpe("run         Run a command in a new container\n");
  fpe("ls          List containers\n");
  fpe("\n");
  fprintf(stderr, "Run 'kucker container COMMAND --help' for more information on a command.\n");
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
  } else if (strcmp(action, "--help") == 0)
  {
    usage();
  } else {
  	usage();
  	exit(1);
  }
}




