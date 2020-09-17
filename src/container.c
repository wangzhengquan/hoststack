#include "container.h"

//重载输出运算符
std::ostream & operator<<(std::ostream & out, Container & info){
	//ID NAME PID STATUS COMMAND CREATED 
	char create_time_str[100];   
	strftime(create_time_str, 100, "%Y-%m-%d %H:%M:%S", localtime(&info.create_time));

  out << info.id << "\t" 
  	<< "\"" << (info.name.empty() ? "---" : info.name) << "\"\t" 
  	<< info.pid<< "\t" 
  	<< info.getStatusName() << "\t" 
  	<< "\"" << info.command << "\"\t" 
  	<< "\"" << create_time_str << "\"" << std::endl;

  return out;
}

const char *Container::TITLE_FORMAT = "%-40s %-40s %-10d %-10s %-30s %-20s \n";
const char *Container::INFO_FORMAT  = "%-40s %-40s %-10s %-10s %-30s %-20s \n";

char *  Container::formatTime(const time_t time, char *_time_str) {
	static char time_str[100];
	char *tmp;
	if(_time_str != NULL) {
		tmp = _time_str;
	} else {
		tmp = time_str;
	}
	strftime(tmp, 100, "%Y-%m-%d %H:%M:%S", localtime(&time));
	return tmp;
}

void Container::show() {
	printf(TITLE_FORMAT, 
		id.c_str(), 
		name.empty() ? id.c_str() : name.c_str(),
		pid,
		getStatusName().c_str(),
		command.c_str(),
		formatTime(create_time, 0)
	);
}

void Container::showTitle() {
	printf(INFO_FORMAT, "ID", "NAME", "PID",  "STATUS",  "COMMAND", "CREATED TIME");
}

std::string Container::getStatusName() {
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

std::string & Container::getName() {
	return name.empty() ? id : name;
}
