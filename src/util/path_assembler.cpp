#include "path_assembler.h"
#include "log.h"

#define PATH_LEN 1024

const char * PathAssembler::getUnionFS(char *dist) {
	static char path[PATH_LEN];
	char *dir;
	if(dist != NULL) {
		 dir = dist;
	} else {
		dir = path;
	}
	sprintf(dir, "%s/%s", hoststack_repo, FS_TYPE);
	return dir;
}

const char * PathAssembler::getLayerDir(const char * containerId, char *dist) {
	static char path[PATH_LEN];
	char *dir;
	if(dist != NULL) {
		 dir = dist;
	} else {
		dir = path;
	}
	sprintf(dir, "%s/%s", getUnionFS(NULL), containerId);
	return dir;
}

const char * PathAssembler::getMergedDir(const char * containerId, char *dist) {
	static char path[PATH_LEN];
	char *dir;
	if(dist != NULL) {
		 dir = dist;
	} else {
		dir = path;
	}
	sprintf(dir, "%s/merged", getLayerDir(containerId, NULL));
	return dir;
}

const char * PathAssembler::getWorkDir(const char * containerId, char *dist) {
	static char path[PATH_LEN];
	char *dir;
	if(dist != NULL) {
		 dir = dist;
	} else {
		dir = path;
	}
	sprintf(dir, "%s/work", getLayerDir(containerId, NULL));
	return dir;
}

const char * PathAssembler::getDiffDir(const char * containerId, char *dist) {
 
	static char path[PATH_LEN];
	char *dir;
	if(dist != NULL) {
		 dir = dist;
	} else {
		dir = path;
	}
	sprintf(dir, "%s/diff", getLayerDir(containerId, NULL));
	return dir;
}

