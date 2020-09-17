#ifndef __PATH_ASSEMBLER_H
#define __PATH_ASSEMBLER_H
#include "usg_common.h"

#define kucker_repo  "/data/kucker"

#define kucker_data_file "/data/kucker/data.json"

class PathAssembler {
public:
	static const char * getUnionFS(char *dir);

	static const char * getRootFS(const char * containerId, char *dir);
};

#endif