#ifndef __CONTAINER_LS_CLI_H
#define __CONTAINER_LS_CLI_H

#include "usg_common.h"
#include "hoststack_config.h"

class ContainerLsCli {

public:
	static void usage();
	
	static void handleCommand (int argc,   char *argv[]);
};
#endif
