#ifndef __LOGGER_FACTORY_H__
#define __LOGGER_FACTORY_H__
#include "logger.h"
#include "path_assembler.h"

class LoggerFactory {
public:

	static Logger& getLogger() {
//ERROR ALL DEBUG INFO WARN
		static Logger logger(Logger::WARN);
		return logger;
	}

	static Logger getContainerLogger(const char * id) {
//ERROR ALL DEBUG INFO WARN
		char logFile[1024];
		sprintf(logFile, "%s/container/%s/container.log", hoststack_repo, id);
		LoggerConfig config;
		config.level = Logger::DEBUG;
		config.logFile = logFile;
		config.console = 1;
		Logger logger(config);
		return logger;
	}

	static Logger& getRunLogger() {
//ERROR ALL DEBUG INFO WARN
		LoggerConfig config;
		config.level = Logger::DEBUG;
		char logFile[1024];
		sprintf(logFile, "%s/run.log", hoststack_repo);
		config.logFile = logFile;
		config.console = 1;
		static Logger logger(config);
		return logger;
	}

	static Logger& getKuckerDaemonLogger() {
//ERROR ALL DEBUG INFO WARN
		LoggerConfig config;
		config.level = Logger::DEBUG;
		char logFile[1024];
		sprintf(logFile, "%s/hoststack_daemon.log", hoststack_repo);
		config.logFile = logFile;
		config.console = 1;
		static Logger logger(config);
		return logger;
	}

	static void logPgrp(const char *id) {
		printf("%s: pid=%d, pgrp=%d, foreground process group=%d, getsid=%d, stdin is a terminal=%d\n",
   		id, getpid(), getpgrp(), tcgetpgrp(STDIN_FILENO), getsid(0), isatty(STDIN_FILENO));
	}
};

#endif


