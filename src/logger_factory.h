#ifndef __LOGGER_FACTORY_H__
#define __LOGGER_FACTORY_H__
#include "logger.h"

class LoggerFactory {
public:

	static Logger& getLogger() {
//ERROR ALL DEBUG INFO WARN
		static Logger logger(Logger::WARN);
		return logger;
	}


	static Logger& getDebugLogger() {
//ERROR ALL DEBUG INFO WARN
		LoggerConfig config;
		config.level = Logger::DEBUG;
		config.logFile = "debug.log";
		static Logger logger(config);
		return logger;
	}

	static void logPgrp(const char *id) {
		printf("%s: pid=%d, pgrp=%d, foreground process group=%d, getsid=%d, stdin is a terminal=%d\n",
   		id, getpid(), getpgrp(), tcgetpgrp(STDIN_FILENO), getsid(0), isatty(STDIN_FILENO));
	}
};

#endif


