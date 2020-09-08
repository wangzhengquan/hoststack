#ifndef __CONTAINER_MANAGER_H
#define __CONTAINER_MANAGER_H

#include "usg_common.h"
#include "kucker_config.h"
#include "container.h"
#include <jsoncpp/json.h>
 

class ContainerManager {
private:
	static Container packContainerInfo(Json::Value jsonData);
	static Container get_container_by_configfile(const std::string& config_file);
public:
	static void save(const Container &info);

	

	static Container get_container_by_id(const std::string& id);
	static Container get_container_by_name(const std::string& name);

	static std::vector<Container>* list() ;

	static void create_container(const char *container_id);

	static void mount_container(const char *container_id);

  static void umount_container(const char *container_id);

  static void umount_volume (const char *container_id) ;

	static char* gen_id(char *uuidstr);

	static void mount_volume (char *container_id, char *volume) ;
	static void bind_mount(const char *container_id, const char *src, const char *_dest);

};

#endif