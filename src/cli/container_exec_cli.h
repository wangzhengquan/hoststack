#ifndef __CONTAINER_EXEC_CLI_H
#define __CONTAINER_EXEC_CLI_H

#include "usg_common.h"
#include "container_info.h"

class ContainerExecCli {

struct container_exec_arg_t {
  bool detach;
  char ** cmd_arr;
  int cmd_arr_len;
} ;
public:
	static void usage();
	
	static void handleCommand (int argc,  char *argv[]);
private:
  static void exec(std::optional<ContainerInfo> container, container_exec_arg_t &mopt);
};
#endif