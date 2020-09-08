#ifndef __CONTAINER_H
#define __CONTAINER_H

#include "usg_common.h"
enum container_status_t {
	CONTAINER_RUNNING,
	CONTAINER_STOPED	 
};

class Container {
public:
	pid_t pid; // 容器在宿主机里的进程ID 
	std::string id; // 容器ID
	std::string name; // 容器名称
	std::string command; // 容器运行的命令
	time_t create_time; // 创建时间
	time_t start_time; //启动时间
	container_status_t status; //运行状态
	std::string volume;
public:
	std::string get_status_name();

	friend std::ostream & operator<<(std::ostream & out, Container & info);

} ;

#endif