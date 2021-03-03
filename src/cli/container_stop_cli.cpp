#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <sys/syscall.h>

#include "container_service.h"
#include "container_info.h"
#include "container_stop_cli.h"

void ContainerStopCli::usage()
{
  fprintf(stderr, "Usage:	docker container stop  CONTAINER [CONTAINER...]\n\n");
  fprintf(stderr, "Stop one or more running containers\n\n");
  // fprintf(stderr, "Options:\n\n");
  // #define fpe(str) fprintf(stderr, "  %s", str);
  // fpe("-d, --detach                         Start container in background and print container ID\n");
  // fpe("\n");
}


void ContainerStopCli::handleCommand(int argc, char *argv[]) {

  if (argc < 2) {
    usage();
    return;
  }

  if(argc == 2 && strcmp(argv[1], "--help") == 0) {
    usage();
    return;
  }

  for(int i = 1; i < argc; i++) {
    ContainerService::stop(argv[i]);
  }
  
}
