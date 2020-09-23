#include "path_assembler.h"

#define PATH_LEN 1024

const char * PathAssembler::getUnionFS(char *dist) {
	return getAufs(dist);
}

const char * PathAssembler::getLayerDir(const char * containerId, char *dist) {
	static char path[PATH_LEN];
		sprintf(path, "%s/%s", getUnionFS(NULL), containerId);
	
	if(dist != NULL) {
		 return strcpy(dist, path);
	} else {
		return path;
	}
}

const char * PathAssembler::getMergedDir(const char * containerId, char *dist) {
	static char path[PATH_LEN];
	sprintf(path, "%s/%s/merged", getUnionFS(NULL), containerId);
	
	if(dist != NULL) {
		 return strcpy(dist, path);
	} else {
		return path;
	}
}

const char * PathAssembler::getWorkDir(const char * containerId, char *dist) {
	static char path[PATH_LEN];
	sprintf(path, "%s/%s/work", getUnionFS(NULL), containerId);
	
	if(dist != NULL) {
		 return strcpy(dist, path);
	} else {
		return path;
	}
}

const char * PathAssembler::getDiffDir(const char * containerId, char *dist) {
	static char path[PATH_LEN];
	sprintf(path, "%s/%s/diff", getUnionFS(NULL), containerId);
	
	if(dist != NULL) {
		 return strcpy(dist, path);
	} else {
		return path;
	}
}



const char * PathAssembler::getAufs(char *dist) {
	static char path[PATH_LEN];
	sprintf(path, "%s/aufs", kucker_repo);
	
	if(dist != NULL) {
		 return strcpy(dist, path);
	} else {
		return path;
	}
}

const char * PathAssembler::getOverlay2FS(char *dist) {
	static char path[PATH_LEN];
	sprintf(path, "%s/overlay2", kucker_repo);
	
	if(dist != NULL) {
		 return strcpy(dist, path);
	} else {
		return path;
	}
}
