#ifndef __CONTAINER_LS_CLI_H
#define __CONTAINER_LS_CLI_H

#include "usg_common.h"
#include "kucker_config.h"
class ContainerLsCli {

public:
	static void usage();
	
	static void handle_command (int argc, char *argv[]);
};
#endif