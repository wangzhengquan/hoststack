
#ifndef __CONTAINER_MANAGER_H
#define __CONTAINER_MANAGER_H

#include "usg_common.h"
#include "kucker_config.h"
#include <jsoncpp/json.h>

 

class ContainerManager {
public:
	static void saveContainerInfo(container_info_t &info) {
		std::ostringstream value;
		Json::Value root;
	printf("==============2\n");
		root["id"] = info.id;
		root["name"] = info.name;
		root["pid"] = info.pid;
		root["command"] = info.command;
		value.seekp(0);
		value << info.create_time;
		root["create_time"] = value.str();
		root["status"] = info.status;

	  auto str = root.toStyledString();
    std::cout << str << std::endl;

printf("==============3\n");
    //sprintf(line, "test -d %s/containers/%s || mkdir %s/containers/%s", kucker_repo, container_id, kucker_repo, container_id);
  
  	std::ostringstream info_location;
  	info_location << kucker_repo << "/containers/" << info.id << "/config.json";
    std::ofstream ofss;
    ofss.open(info_location.str());
    ofss << str;
    ofss.close();
	}
};

#endif