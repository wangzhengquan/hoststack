#include "path_assembler.h"

#define PATH_LEN 1024

const char * PathAssembler::getUnionFS(char *dir) {
	static char path[PATH_LEN];
	sprintf(path, "%s/aufs", kucker_repo);
	
	if(dir != NULL) {
		 return strcpy(dir, path);
	} else {
		return path;
	}
}

const char * PathAssembler::getRootFS(const char * containerId, char *dir) {
	static char path[PATH_LEN];
	sprintf(path, "%s/mnt/%s", getUnionFS(NULL), containerId);
	
	if(dir != NULL) {
		 return strcpy(dir, path);
	} else {
		return path;
	}
}
