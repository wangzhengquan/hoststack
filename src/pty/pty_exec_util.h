#ifndef PTY_EXEC_UTIL_H
#define PTY_EXEC_UTIL_H
#include "usg_common.h"
struct pty_exe_opt_t
{
	const char *containerId; // 容器id
  char **cmd;    // 运行容器的命令
  bool detach;   // 容器是否后台运行
  std::set<std::string> *volume_list;
  int synchSem;  // 通知用的信号量id
  struct winsize *ttyWs; //终端窗口大小
  struct termios *ttyAttr; //终端属性
};

int pty_exec(pty_exe_opt_t arg);
int pty_proxy_exec(pty_exe_opt_t arg);
int pty_client(pty_exe_opt_t arg) ;
#endif