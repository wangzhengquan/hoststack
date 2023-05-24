#ifndef __CONTAINER_RM_CLI_H
#define __CONTAINER_RM_CLI_H

#include "usg_common.h"

class ContainerRMCli {

public:
	static void usage();
	
	static void handleCommand (int argc,  char *argv[]);
};
#endif