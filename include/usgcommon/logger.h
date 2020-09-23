#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "usg_common.h"
#include "usg_typedef.h"

struct LoggerConfig {
  std::string logFile;
  int level; 
  bool console;
};

class Logger {
private:
  std::string configFile;
  LoggerConfig config;
  FILE *logFile;

  void dolog(const char *fmt, va_list ap, int level, int err = 0) {
    char buf[MAXBUF];

    struct timeval tv;
    struct tm *info;
    gettimeofday(&tv, NULL);
    info = localtime(&tv.tv_sec);
    strftime(buf, MAXBUF - 1, "%Y-%d-%m %H:%M:%S", info);
    snprintf(buf + strlen(buf), MAXBUF - strlen(buf) - 1, ",%ld [%s] ",  tv.tv_usec, strlevel(level));
    vsnprintf(buf + strlen(buf), MAXBUF - strlen(buf) - 1, fmt, ap);

    if (err != 0) {
      snprintf(buf + strlen(buf), MAXBUF - strlen(buf) - 1, ": %s", strerror(err));
    }
    strcat(buf, "\n");
    fflush(stdout); /* in case stdout and stderr are the same */
    
    if(logFile != NULL) {
      fputs(buf, logFile);
    }
    if(config.console) {
       fputs(buf, stdout);
    }
   
    fflush(NULL); /* flushes all stdio output streams */
  }
  void init();

private:
  static const char *LOGGER_LEVEL_STR[] ;
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
  void error(int err, const char *fmt, ...);
  void fatal(const char *fmt, ...);


  static const char * strlevel(int level);
};

#endif