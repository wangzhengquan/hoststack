#include "comm_typedef.h"
#include "log.h"
/*
 * Variable panicstr contains argument to first call to panic; used as flag
 * to indicate that the system has already called panic.
 */
const char *panicstr;

void _dolog(const char *file, const int line, const char *fmt, va_list ap) {
  char buf[MAXBUF];

  struct timeval tv;
  struct tm *info;
  gettimeofday(&tv, NULL);
  info = localtime(&tv.tv_sec);
  strftime(buf, MAXBUF - 1, "%Y-%d-%m %H:%M:%S", info);
  snprintf(buf + strlen(buf), MAXBUF - strlen(buf) - 1, ",%ld [%s](%s:%d) ",  tv.tv_usec, "ERROR", file, line);
  vsnprintf(buf + strlen(buf), MAXBUF - strlen(buf) - 1, fmt, ap);

  // if (err != 0) {
  //   snprintf(buf + strlen(buf), MAXBUF - strlen(buf) - 1, ": %s", strerror(err));
  // }
  strcat(buf, "\n");
  fflush(stdout); /* in case stdout and stderr are the same */
  
  fputs(buf, stdout);
  // fflush(stdout);
 
}

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", and then enters the kernel monitor.
 */
void
_panic(const char *file, int line, const char *fmt,...)
{
	va_list ap;

	if (panicstr)
		goto dead;
	panicstr = fmt;

	va_start(ap, fmt);
	_dolog(file, line, fmt, ap);
	va_end(ap);

dead:
	/* break into the kernel monitor */
	while (1)
		;
}

/* like panic, but don't */
void
_warn(const char *file, int line, const char *fmt,...)
{
	va_list ap;

	va_start(ap, fmt);
	_dolog(file, line, fmt, ap);
	va_end(ap);
}
