/* See COPYRIGHT for copyright information. */

#ifndef COMMON_LOG_H
#define COMMON_LOG_H

#include "usg_common.h"
/*
 * Variable panicstr contains argument to first call to panic; used as flag
 * to indicate that the system has already called panic.
 */
static const char *panicstr;

static void _dolog(const char * level, const char *file, const int line, const char *fmt, ...);

/// Log warning (red)
#define log_error(fmt, ...) _dolog("ERROR", __FILE__, __LINE__,  fmt , ##__VA_ARGS__) 

#define err_msg(errnum, fmt, ...) \
    do { \
        log_error(fmt, ##__VA_ARGS__); \
        (errnum != 0) && fprintf(stderr, "%s\n", strerror(errnum)); \
    } while (0)

#define err_exit(errnum, fmt, ...) \
  do { \
   err_msg(errnum, fmt, ##__VA_ARGS__); \
   exit(errnum); \
  } while (0)
 
/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", and then enters the kernel monitor.
 */
#define panic(fmt, ...)   \
  do{ \
    if (panicstr)   \
        goto dead;   \
      panicstr = fmt;   \
      _dolog("FATAL", __FILE__, __LINE__, "\033[0;31m" fmt "\033[0m", ##__VA_ARGS__); \
    dead:    \
      /* break into the kernel monitor */ \
      while (1)   \
        ; \
  } while (0) 



// 	do { if (!(x)) panic("assertion failed: %s", #x); } while (0)
#define MY_ASSERT(expr, fmt, ...)        \
    do { if (!(expr)) {warn(#expr fmt, ##__VA_ARGS__); exit(1);}} while (0)

  

// static_assert(x) will generate a compile-time error if 'x' is false.
#define MY_STATIC_ASSERT(x)	switch (x) case 0: case (x):



static void _dolog(const char * level, const char *file, const int line, const char *fmt, ...) {
  char buf[MAXBUF];

  struct timeval tv;
  struct tm *info;
  va_list args;
  va_start(args, fmt);
  gettimeofday(&tv, NULL);
  info = localtime(&tv.tv_sec);
  strftime(buf, MAXBUF - 1, "%Y-%d-%m %H:%M:%S", info);
  snprintf(buf + strlen(buf), MAXBUF - strlen(buf) - 1, ",%ld [%s](%s:%d) ",  tv.tv_usec, level, file, line);
  vsnprintf(buf + strlen(buf), MAXBUF - strlen(buf) - 1, fmt, args);

  va_end(args);
  // if (err != 0) {
  //   snprintf(buf + strlen(buf), MAXBUF - strlen(buf) - 1, ": %s", strerror(err));
  // }
  strcat(buf, "\n");
  fflush(stdout); /* in case stdout and stderr are the same */
  
  fputs(buf, stdout);
  // fflush(stdout);
 
}

// 	do { if (!(x)) panic("assertion failed: %s", #x); } while (0)
 

#endif 
