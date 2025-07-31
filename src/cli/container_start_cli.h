#ifndef __CONTAINER_START_CLI_H
#define __CONTAINER_START_CLI_H

#include "usg_common.h"
#include "hoststack_config.h"
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>

struct container_start_arg_t
{
  bool detach;
  const char * containerName; // container identify
} ;


class ContainerStartCli {

public:
	static void usage();
	
	static void handleCommand (int argc,  char *argv[]);

	static void startContainer(container_start_arg_t & mopt, struct termios *ttyAttr,  struct winsize *ttyWs);
};
#endif
