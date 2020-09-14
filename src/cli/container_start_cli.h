#ifndef __CONTAINER_START_CLI_H
#define __CONTAINER_START_CLI_H

#include "usg_common.h"
#include "kucker_config.h"

class ContainerStartCli {

public:
	static void usage();
	
	static void handle_command (int argc, char *argv[]);
};
#endif