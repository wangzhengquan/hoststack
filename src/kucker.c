#include "usg_common.h"
#include "kucker_config.h"
#include "container_manager.h"
#include "container_cli.h"
#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>

static void usage()
{
  printf("usage: param error\n");
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
    ContainerCli::handle_command(argc - 1, argv + 1);
  }
  else if (strcmp(target, "run") == 0)
  {
    ContainerCli::handle_run_command(argc - 1, argv + 1);
  }
  else if (strcmp(target, "ps") == 0)
  {
    ContainerCli::handle_ls_command(argc - 1, argv + 1);
  }
  else if ( strcmp(target, "start") == 0)
  {

  } else {
    usage();
  }
  return 0;
}
