#include "usg_common.h"
#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>

#include "kucker_config.h"
#include "container_cli.h"

static void usage()
{
  fprintf(stderr, "Usage: kucker [OPTIONS] COMMAND\n\n");
  fprintf(stderr, "A self-sufficient runtime for containers\n\n");
  fprintf(stderr, "Commands:\n\n");
  #define fpe(str) fprintf(stderr, "  %s", str);
  fpe("run         Run a command in a new container\n");
  fpe("ps          List containers\n");
  fpe("\n");
  fprintf(stderr, "Run 'kucker COMMAND --help' for more information on a command.\n");
}



int main(int argc, char *argv[])
{
  char *target;
  if (argc < 2)
  {
    usage();
    return 1;
  }
  else
  {
    target = argv[1];
  }
 
  if (strcmp(target, "container") == 0){
    ContainerCli::handleCommand(argc - 1, argv + 1);
  }
  else if (strcmp(target, "run") == 0)
  {
    ContainerRunCli::handleCommand(argc - 1, argv + 1);
  }
  else if (strcmp(target, "stop") == 0)
  {
    ContainerStopCli::handleCommand(argc - 1, argv + 1);
  }
  else if (strcmp(target, "start") == 0)
  {
    ContainerStartCli::handleCommand(argc - 1, argv + 1);
  }
  else if (strcmp(target, "ps") == 0)
  {
    ContainerLsCli::handleCommand(argc - 1, argv + 1);
  }
  else if (strcmp(target, "attach") == 0) {
    ContainerAttachCli::handleCommand(argc - 1, argv + 1);
  } 
  else if (strcmp(target, "exec") == 0)
  {
    ContainerExecCli::handleCommand(argc - 1, argv + 1);
  }
  else if ( strcmp(target, "--help") == 0)
  {
    usage();

  } else {
    usage();
  }
  return 0;
}
