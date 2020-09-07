#include "kucker_config.h"
//重载输出运算符
std::ostream & operator<<(std::ostream & out, container_info_t & info){
	//ID NAME PID STATUS COMMAND CREATED 
	char create_time_str[100];   
	strftime(create_time_str, 100, "%Y-%m-%d %H:%M:%S", localtime(&info.create_time));
  out << info.id << "\t" << (info.name.empty() ? "---" : info.name) << "\t" 
  	<< info.pid<< "\t\"" << info.command << "\"\t\"" << create_time_str << "\"" << std::endl;
  return out;
}