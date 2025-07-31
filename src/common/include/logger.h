#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "usg_common.h"


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

  void dolog(const char *fmt, va_list ap, int level, int err = 0);

  
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
  void fatal(int err, const char *fmt, ...) ;

  static const char * strlevel(int level);
};

#endif