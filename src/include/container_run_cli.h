#ifndef __CONTAINER_RUN_CLI_H
#define __CONTAINER_RUN_CLI_H

#include "usg_common.h"
#include "kucker_config.h"

class ContainerRunCli {

public:
	static void usage();
	
	static void handle_command (int argc, char *argv[]);
};
#endif