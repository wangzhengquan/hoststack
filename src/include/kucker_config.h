
#ifndef __KUCKER_CONFIG_H
#define __KUCKER_CONFIG_H

static const char *kucker_repo = "/data/kucker";

enum container_status_t {
	CONTAINER_RUNNING,
	CONTAINER_STOPED	 
};

typedef struct container_info_t {
	pid_t pid; // 容器在宿主机里的进程ID 
	std::string id; // 容器ID
	std::string name; // 容器名称
	std::string command; // 容器运行的命令
	time_t create_time; // 创建时间
	container_status_t status; //运行状态
} container_info_t;

#endif