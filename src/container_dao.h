#ifndef __CONTAINER_DAO_H
#define __CONTAINER_DAO_H

#include "usg_common.h"
#include "container.h"
#include <jsoncpp/json.h>
 



class ContainerDao {
private:
	static Container pack_container_info(const Json::Value &jsonData) ;
	static Json::Value & de_pack_container_info(Json::Value &jsonData,  const Container &info);
	// static Container get_container_by_configfile(const std::string& config_file);
public:
	static void insert(const Container &info);
	static void update(const Container &info);
	static void delete_by_id(const char *containerName);


	static void change_status_to_stop( const std::string & name);
	

	static Container get_container_by_id(const std::string& id);
	static Container get_container_by_name(const std::string& name);
	static Container get_container_by_id_or_name(const std::string& value);
	static Container get_container_by(const char * name,const std::string& value);

	static std::vector<Container>* list() ;

	
	static char* gen_id(char *uuidstr);

};

#endif