#ifndef PTY_EXEC_UTIL_H
#define PTY_EXEC_UTIL_H

struct pty_exe_opt_t
{
  char *rootfs;
  char **cmd;
  bool detach;
  char *logfile;
  char *serverFifo;
  char *clientFifo;
};

int pty_exec(pty_exe_opt_t arg);
int pty_proxy_exec(pty_exe_opt_t arg);
int pty_client(pty_exe_opt_t arg) ;
#endif