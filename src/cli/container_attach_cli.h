#ifndef __CONTAINER_ATTACH_CLI_H
#define __CONTAINER_ATTACH_CLI_H

#include "usg_common.h"
#include "kucker_config.h"


class ContainerAttachCli {

public:
	static void usage();
	
	static void handle_command (int argc, char *argv[]);
};

#endif