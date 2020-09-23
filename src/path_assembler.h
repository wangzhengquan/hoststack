#ifndef __PATH_ASSEMBLER_H
#define __PATH_ASSEMBLER_H
#include "usg_common.h"

#define FS_TYPE "aufs"

#define kucker_repo  "/data/kucker"

#define kucker_data_file "/data/kucker/data.json"

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