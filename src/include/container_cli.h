#ifndef __CONTAINER_CLI_H
#define __CONTAINER_CLI_H

#include "usg_common.h"
#include "kucker_config.h"
class ContainerCli {
public:
	static void exe_run_commond (int argc, char *argv[]);
	static void exe_ps_commond(int argc, char *argv[]);
};
#endif
