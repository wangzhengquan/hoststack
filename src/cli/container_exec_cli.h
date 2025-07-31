#ifndef __CONTAINER_EXEC_CLI_H
#define __CONTAINER_EXEC_CLI_H

#include "usg_common.h"

class ContainerExecCli {

public:
	static void usage();
	
	static void handleCommand (int argc,  char *argv[]);
};
#endif