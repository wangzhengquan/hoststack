#ifndef PTY_EXEC_UTIL_H
#define PTY_EXEC_UTIL_H

struct pty_exe_opt_t
{
  char *rootfs;
  char **cmd;
  bool detach;
  char *logfile;
  char *outfile;
  char *infile;
};

int pty_exec(pty_exe_opt_t arg);

#endif