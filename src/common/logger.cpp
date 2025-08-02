#include "logger.h"
#include "properties_config.h"

const char *Logger::LOGGER_LEVEL_STR[]  = {"ALL", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", "OFF" };

const char * Logger::strlevel(int level){
	return LOGGER_LEVEL_STR[level];
}

Logger::Logger(std::string cf): configFile(cf) {
	PropertiesConfig properties(configFile);
	 

	config.level = properties.getInt("log_level");
	config.logFile = properties.get("log_file");
	config.console = (bool)properties.getInt("console");

	// std::cout <<  properties.get("log_file") << std::endl;
	// printf("==log_file=%s\n", properties.get("log_file").c_str());
	// printf("=console==%d\n", properties.getInt("console"));
	// new (this)Logger(conf);
	init();
}

Logger::Logger(LoggerConfig & conf): config(conf) {
	init();
}

Logger::Logger(int l) {
	config.level = l;
	config.console = true;
	init();
}


Logger::~Logger() {
	if(logFile != NULL) {
		if(fclose(logFile) != 0) {
      fprintf(stderr, "Logger fclose error: %s\n", strerror(errno));
      exit(errno);
		}
	}
	
}


void Logger::init() {
	logFile = NULL;
  if(!config.logFile.empty()) {
     logFile = fopen(config.logFile.c_str(), "a+");
     if(logFile == NULL) {
      fprintf(stderr, "Logger fopen error: %s\n", strerror(errno));
      exit(errno);
     }
  }
}

void Logger::dolog(const char *fmt, va_list ap, int level, int err) {
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
  strcat(buf, "\r\n");
  fflush(stdout); /* in case stdout and stderr are the same */
  
  if(logFile != NULL) {
    fputs(buf, logFile);
    fflush(logFile);
  }
  if(config.console) {
     fputs(buf, stdout);
  }
 
}

void Logger::log(int _level,  const char *fmt, ...) {

	if(_level < config.level)
		return;

	va_list		ap;
	va_start(ap, fmt);
	dolog(fmt, ap, _level);
	va_end(ap);
}

void Logger::debug(const char *fmt, ...) {
	if(DEBUG < config.level)
		return;

	va_list		ap;
	va_start(ap, fmt);
	dolog(fmt, ap, DEBUG);
	va_end(ap);
}


void Logger::debug(int err, const char *fmt, ...) {
	if(DEBUG < config.level)
		return;

	va_list		ap;
	va_start(ap, fmt);
	dolog(fmt, ap, DEBUG, err);
	va_end(ap);
}

void Logger::info(const char *fmt, ...) {
	if(INFO < config.level)
		return;

	va_list		ap;
	va_start(ap, fmt);
	dolog(fmt, ap, INFO);
	va_end(ap);
}

void Logger::warn(const char *fmt, ...) {
	if(WARN < config.level)
		return;

	va_list		ap;
	va_start(ap, fmt);
	dolog(fmt, ap, WARN);
	va_end(ap);
}

void Logger::error(const char *fmt, ...) {
	if(ERROR < config.level)
		return;

	va_list		ap;
	va_start(ap, fmt);
	dolog(fmt, ap, ERROR);
	va_end(ap);
}

void Logger::error(int err, const char *fmt, ...) {
	if(ERROR < config.level)
		return;

	va_list		ap;
	va_start(ap, fmt);
	dolog(fmt, ap, ERROR, err);
	va_end(ap);
}

void Logger::fatal(const char *fmt, ...) {
	if(FATAL < config.level)
		return;
	va_list		ap;
	va_start(ap, fmt);
	dolog(fmt, ap, FATAL);
	va_end(ap);
  abort();
}

void Logger::fatal(int err, const char *fmt, ...) {
	if(FATAL < config.level)
		return;
	va_list		ap;
	va_start(ap, fmt);
	dolog(fmt, ap, FATAL, err);
	va_end(ap);
  abort();
  // exit(err);
}
