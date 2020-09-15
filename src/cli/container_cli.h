#ifndef __CONTAINER_CLI_H
#define __CONTAINER_CLI_H

#include "usg_common.h"
#include "kucker_config.h"

#include "container_ls_cli.h"
#include "container_run_cli.h"
#include "container_stop_cli.h"
#include "container_start_cli.h"
#include "container_exec_cli.h"
#include "container_attach_cli.h"

class ContainerCli {

public:
	static void usage();
	
	static void handle_command(int argc, char *argv[]);
	// static void handle_run_command (int argc, char *argv[]);
	// static void handle_ls_command(int argc, char *argv[]);
};



#endif
