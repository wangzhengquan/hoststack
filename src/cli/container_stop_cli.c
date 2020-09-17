#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <sys/syscall.h>

#include "container_service.h"
#include "container.h"
#include "container_stop_cli.h"

void ContainerStopCli::usage()
{
  printf("usage: param error\n");
}


void ContainerStopCli::handleCommand(int argc, char *argv[]) {

  if (argc < 2) {
    usage();
    return;
  }

  for(int i = 1; i < argc; i++) {
    ContainerService::stop(argv[i]);
  }
  
}
