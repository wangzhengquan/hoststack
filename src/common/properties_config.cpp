#include "properties_config.h"

PropertiesConfig::PropertiesConfig(std::string __propertiesFile) : propertiesFile(__propertiesFile) {

	std::ifstream fin(propertiesFile);
	char line[1024];
	//std::string line;
	char *key, *value;
  const char *delim = "=";
	while(fin.getline(line, 1024)) {
		// printf("line=%s\n", line);
		if(strlen(trim(line, NULL))== 0)
			continue;
		if(*line == '#') {
			continue;
		}

	    key = trim(strtok(line, delim), 0);
	    value = trim(strtok(NULL, delim), 0);
	    // propertiesMap.insert({key, value});
	    propertiesMap.insert(std::pair<std::string, std::string>(key, value));
	    // printf("key = %s, value=%s\n", key, value);
	}
	fin.close();

}


std::string PropertiesConfig::get(std::string name) {
  std::map<std::string, std::string>::iterator propertiesIter = propertiesMap.find(name);
  if( propertiesIter != propertiesMap.end() ) {
     return propertiesIter->second;
  }
  return "";
}

int PropertiesConfig::getInt(std::string name) {
  std::map<std::string, std::string>::iterator propertiesIter = propertiesMap.find(name);
  if( propertiesIter != propertiesMap.end() ) {
     return  std::stoi(propertiesIter->second);
  }
  return 0;
}