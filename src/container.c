#include "container.h"

//重载输出运算符
std::ostream & operator<<(std::ostream & out, Container & info){
	//ID NAME PID STATUS COMMAND CREATED 
	char create_time_str[100];   
	strftime(create_time_str, 100, "%Y-%m-%d %H:%M:%S", localtime(&info.create_time));
  out << info.id << "\t" << (info.name.empty() ? "---" : info.name) << "\t" 
  	<< info.pid<< "\t\"" << info.command << "\"\t\"" << create_time_str << "\"" << std::endl;
  return out;
}


std::string Container::get_status_name() {
	std::string status_name = "---";
	switch (status) {
		case CONTAINER_RUNNING:
			status_name = "running";
			break;
		case CONTAINER_STOPED:
			status_name = "stoped";
			break;
		default:
			break;
	}
	return status_name;
}
