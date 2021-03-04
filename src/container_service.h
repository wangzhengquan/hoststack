#ifndef __CONTAINER_SERVICE_H
#define __CONTAINER_SERVICE_H

#include "usg_common.h"

/* 定义一个给 clone 用的栈，栈大小1M */
#define STACK_SIZE (1024 * 1024)
struct container_start_option_t
{
  const char * containerId;
  char **cmd;
  bool detach;
  std::set<std::string> *volume_list;
  
  struct winsize *ttyWs;
  struct termios *ttyAttr;
  
} ;

class ContainerService {
public:
	static void start(container_start_option_t & startOpt,  std::function<void(int)>  startSuccess);
	static void stop(const std::string & name);
};

#endif