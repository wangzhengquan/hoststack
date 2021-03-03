#include "usg_common.h"

static void logPgrp(const char *id) {
	printf("%s: pid=%d, pgrp=%d, foreground process group=%d, getsid=%d, stdin is a terminal=%d\n",
 		id, getpid(), getpgrp(), tcgetpgrp(STDIN_FILENO), getsid(0), isatty(STDIN_FILENO));
}

int main() {
 
	logPgrp("1");
	 
	if( fork() == 0 ) {
		logPgrp("2");
		if( fork() == 0 ) {
			logPgrp("3");
		}  
	}
	wait(0);
	return 0;
}