#include "usg_common.h"

int main() {
	char path[] = "/aaa/bb";
	printf("%d, %d, %d\n", *path, '/', *path == '/');
}