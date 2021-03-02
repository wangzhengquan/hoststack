#ifndef __PATH_ASSEMBLER_H
#define __PATH_ASSEMBLER_H
#include "usg_common.h"
// aufs overlay2
#define FS_TYPE "overlay2"

#define kucker_repo  "/home/kucker"

#define kucker_data_file "/home/kucker/data.json"

class PathAssembler {
private:
	const char * getAufs(char *dist);
public:
	static const char * getUnionFS(char *dir);

	static const char * getLayerDir(const char * containerId, char *dist);
	static const char * getMergedDir(const char * containerId, char *dist);
	static const char * getWorkDir(const char * containerId, char *dist) ;
	static const char * getDiffDir(const char * containerId, char *dist);

};

#endif