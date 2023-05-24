#ifndef __CONTAINER_DAO_H
#define __CONTAINER_DAO_H

#include "usg_common.h"
#include "container_info.h"
#include <jsoncpp/json.h>
 



class ContainerDao {
private:
	static ContainerInfo pack_container_info(const Json::Value &jsonData) ;
	static Json::Value & de_pack_container_info(Json::Value &jsonData,  const ContainerInfo &info);
	// static Container get_container_by_configfile(const std::string& config_file);
public:
	static void insert(const ContainerInfo &info);
	static void update(const ContainerInfo &info);
	static void delete_by_id(const char *containerName);


	static void change_status_to_stop( const std::string & name);
	

	static ContainerInfo get_container_by_id(const std::string& id);
	static ContainerInfo get_container_by_name(const std::string& name);
	static ContainerInfo get_container_by_id_or_name(const std::string& value);
	static ContainerInfo get_container_by(const char * name,const std::string& value);

	static std::vector<ContainerInfo>* list() ;

	
	static std::string gen_id();

};

#endif