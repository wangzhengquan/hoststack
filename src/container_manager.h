#ifndef __CONTAINER_MANAGER_H
#define __CONTAINER_MANAGER_H

#include "usg_common.h"
#include "container.h"
#include <jsoncpp/json.h>
 



class ContainerManager {
private:
	static Container pack_container_info(const Json::Value &jsonData) ;
	static Json::Value & de_pack_container_info(Json::Value &jsonData,  const Container &info);
	// static Container get_container_by_configfile(const std::string& config_file);
public:
	static void insert(const Container &info);
	static void update(const Container &info);


	static void change_status_to_stop( const std::string & name);
	

	static Container get_container_by_id(const std::string& id);
	static Container get_container_by_name(const std::string& name);
	static Container get_container_by_id_or_name(const std::string& value);
	static Container get_container_by(const char * name,const std::string& value);

	static std::vector<Container>* list() ;

	static void create_container(const char *container_id);
	static void remove_container(const char *containerName);

	static void mount_container(const std::string & container_id);
  static void umount_container(const std::string & container_id);

  static void umount_volume_list (const char *container_id) ;
  static void umount_volume (const char * container_id, const char* _volume) ;

	static char* gen_id(char *uuidstr);

	static void mount_volume (const char * container_id, const char *volume) ;
	static void mount_volume_list (const char *container_id, std::set<std::string> &volume_list);
	static void bind_mount(const char *container_id, const char *src, const char *_dest);

};

#endif