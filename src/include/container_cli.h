#ifndef __CONTAINER_CLI_H
#define __CONTAINER_CLI_H

#include "usg_common.h"
#include "kucker_config.h"
class ContainerCli {
public:
	static void handle_command(int argc, char *argv[]);
	static void handle_run_command (int argc, char *argv[]);
	static void handle_ls_command(int argc, char *argv[]);
};
#endif
