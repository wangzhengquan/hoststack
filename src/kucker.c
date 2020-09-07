#include "usg_common.h"
#include "kucker_config.h"
#include "container_manager.h"
#include "container_cli.h"

#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>




 

int pipefd[2];
 
void usage()
{
  printf("usage:\n");
}





int main(int argc, char *argv[])
{


  char *action;
  if (argc < 2)
  {
    usage();
    return 1;
  }
  else
  {
    action = argv[1];
  }
 

  if (strcmp(action, "run") == 0)
  {
    ContainerCli::exe_run_commond(argc - 1, argv + 1);
  }
  else if (strcmp(action, "ps") == 0)
  {
    ContainerCli::exe_ps_commond(argc - 1, argv + 1);
  }
  else if ( strcmp(action, "start") == 0)
  {

  }


  return 0;
}
