#include "usg_common.h"
#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include "path_assembler.h"
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
  fpe("attach      Attach local standard input, output, and error streams to a running container\n");
  fpe("rm          Remove one or more containers\n");
  fpe("stop        Stop one or more running containers\n");
  fpe("exec        Run a command in a running container\n");
  fpe("start       Start one or more stopped containers\n");
  fpe("\n");
  fprintf(stderr, "Run 'kucker COMMAND --help' for more information on a command.\n");
}

static void initContainerDir() {
  const char *unionfs = PathAssembler::getUnionFS(NULL);
  char line[1024];
  
  sprintf(line, "test -d %s/containers || sudo mkdir -p %s/containers", kucker_repo, kucker_repo);
  if (system(line) != 0)
  {
    perror(line);
  }

  sprintf(line, "test -d %s || sudo mkdir -p %s", unionfs, unionfs);
  if (system(line) != 0)
  {
    perror(line);
  }

  sprintf(line, "test -d %s/diff || sudo mkdir -p %s/diff", unionfs, unionfs);
  if (system(line) != 0)
  {
    perror(line);
  }
  sprintf(line, "test -d %s/layers || sudo mkdir -p %s/layers", unionfs, unionfs);
  if (system(line) != 0)
  {
    perror(line);
  }

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
  initContainerDir();
 
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
  else if (strcmp(target, "rm") == 0) {
    ContainerRMCli::handleCommand(argc - 1, argv + 1);
  } 
  else if ( strcmp(target, "--help") == 0)
  {
    usage();

  } else {
    usage();
  }
  return 0;
}
