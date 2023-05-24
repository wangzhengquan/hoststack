#include "usg_common.h"

int main() {
	 int outfd = open("/home/wzq/wk/hoststack/tmp.txt", O_RDWR | O_CREAT,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                S_IROTH | S_IWOTH);
	 if(outfd == -1) {
	 	perror("open:");
	 }
	 close(outfd);
}
