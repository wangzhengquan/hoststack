#ifndef __CONTAINER_RUN_CLI_H
#define __CONTAINER_RUN_CLI_H

#include "usg_common.h"
#include "hoststack_config.h"
#include <termios.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>

struct container_run_arg_t {
  const char *container_id;
  bool detach;
  std::set<std::string> volume_list;
  const char *name;
  char ** cmd_arr;
  int cmd_arr_len;
}  ;

class ContainerRunCli {

private:
	static void startContainer(container_run_arg_t &mopt,   struct termios *ttyAttr, struct winsize *ttyWs);

public:
	static void usage();
	
	static void handleCommand (int argc,  char *argv[]);
};

#endif
