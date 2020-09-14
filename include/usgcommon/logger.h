#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "usg_common.h"
#include "usg_typedef.h"

struct LoggerConfig {
  std::string logFile;
  int level; 
};

class Logger {
  std::string configFile;
  LoggerConfig config;
  FILE *logFile;

  void dolog(const char *fmt, va_list ap) {
    char buf[MAXBUF];

    struct timeval tv;
    struct tm *info;
    gettimeofday(&tv, NULL);
    info = localtime(&tv.tv_sec);
    strftime(buf, MAXLINE - 1, "%Y-%d-%m %H:%M:%S ", info);
    snprintf(buf + strlen(buf), MAXLINE - strlen(buf) - 1, "(%ld) : ", tv.tv_sec * 1000000 + tv.tv_usec);

    vsnprintf(buf + strlen(buf), MAXLINE - strlen(buf) - 1, fmt, ap);
    strcat(buf, "\n");
    fflush(stdout); /* in case stdout and stderr are the same */
    
    if(logFile != NULL) {
      fputs(buf, logFile);
    }
    fputs(buf, stdout);
    fflush(NULL); /* flushes all stdio output streams */
  }
  void init();

public:
  enum LoggerLevel { ALL, DEBUG, INFO, WARN, ERROR, FATAL, OFF };

  Logger(int l = INFO);
  Logger(std::string cf);
  Logger(LoggerConfig & conf);

  ~Logger();

  void log(int _level, const char *fmt, ...);

  void debug(const char *fmt, ...);
  void info(const char *fmt, ...);
  void warn(const char *fmt, ...);
  void error(const char *fmt, ...);
  void fatal(const char *fmt, ...);
};

#endif