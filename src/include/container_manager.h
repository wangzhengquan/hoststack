
#ifndef __CONTAINER_MANAGER_H
#define __CONTAINER_MANAGER_H

#include "usg_common.h"
#include "kucker_config.h"


 

class ContainerManager {
public:
	static void save(container_info_t &info);

	static std::vector<container_info_t>* list() ;

	static void create_container(const char *container_id);

	static void mount_container(const char *rootfs);

	static char* gen_id(char *uuidstr);

	static void mount_volume (char *container_id, char *volume) ;
	static void bind_mount(const char *container_id, const char *src, const char *_dest);
	
};

#endif