
#include <getopt.h>
#include <uuid.h>
#include <jsoncpp/json.h>
#include "container_dao.h"
#include "path_assembler.h"
#include "container.h"
#include "logger_factory.h"



void ContainerDao::insert(const Container &info) {
	Json::Value root;
  
  Json::Reader jsonreader;
  std::ifstream fin(kucker_data_file); 
  if(fin) {
    jsonreader.parse(fin, root);
    fin.close();
  }
// printf("==============2\n");
  Json::Value infojson;
  de_pack_container_info(infojson, info);
	 

  root.append(infojson);

  auto str = root.toStyledString();
  // printf("=====insert======= %s\n", kucker_data_file);
  // std::cout << str << std::endl;
  // printf("============\n");
  std::ofstream fout;
  fout.open(kucker_data_file);
  fout << str;
  fout.close();
}



void ContainerDao::delete_by_id(const char *containerName) {
  Container info = ContainerDao::get_container_by_id_or_name(containerName);
 

  Json::Value oldRoot;
  Json::Value newRoot;
  
  Json::Reader jsonreader;
  std::ifstream fin(kucker_data_file);
  if(!fin) {
    return;
  }

  if(!jsonreader.parse(fin, oldRoot)) {
    return;
  }

  int size = oldRoot.size();
  if(size == 0) {
    return;
  }

  for(int i = 0; i < size; i++) {
    if(oldRoot[i]["id"].asString() != info.id)
      newRoot.append(oldRoot[i]);
  }

  auto str = newRoot.toStyledString();
  // std::cout << str << std::endl;
  std::ofstream fout;
  fout.open(kucker_data_file);
  fout << str;
  fout.close();
  
}


void ContainerDao::update(const Container &info) {
  Json::Value root;
  
  Json::Reader jsonreader;
  std::ifstream fin(kucker_data_file);
  if(!fin) {
    insert(info);
  }

  if(!jsonreader.parse(fin, root)) {
    insert(info);
  }

  int size = root.size();
  if(size == 0) {
    insert(info);
  }

  Json::Value *record;
  for(int i = 0; i < size; i++) {
    record = &root[i];
    if((*record)["id"].asString() == info.id) {
      de_pack_container_info(*record, info);
      break;
    }
  }

  auto str = root.toStyledString();
  // printf("=======update===== %s\n", kucker_data_file);
  // std::cout << str << std::endl;
  // printf("============\n");
  std::ofstream fout;
  fout.open(kucker_data_file);
  fout << str;
  fout.close();
}

 
void ContainerDao::change_status_to_stop( const std::string & name) {
 
  Container info = ContainerDao::get_container_by_id_or_name(name);
  info.status = CONTAINER_STOPED;
  info.stop_time = time(0);
  info.pid = 0;
  ContainerDao::update(info);
  
}




Container ContainerDao::get_container_by_id(const std::string& value) {
  return get_container_by("id", value);
}

Container ContainerDao::get_container_by_name(const std::string& value) {
  return get_container_by("name", value);
}

Container ContainerDao::get_container_by(const char * name,const std::string& value) {
  Json::Value root;
  char line[1024];
  
  Json::Reader jsonreader;
  std::ifstream fin(kucker_data_file);
  if(!fin) {
    return {};
  }

  if(!jsonreader.parse(fin, root)) {
    return {};
  }

  int size = root.size();
  if(size == 0) {
    return {};
  }

  for(int i = 0; i < size; i++) {
    if(root[i][name].asString() == value) {
      if(root[i]["status"].asInt() == CONTAINER_RUNNING) {
        sprintf(line, "/proc/%d", root[i]["pid"].asInt());
        if(access(line, F_OK) == -1) {
          root[i]["status"] = CONTAINER_STOPED;
          root[i]["stop_time"] = (int)time(0);
          root[i]["pid"] = 0;
          auto str = root.toStyledString();
          // std::cout << str << std::endl;
          std::ofstream fout;
          fout.open(kucker_data_file);
          fout << str;
          fout.close();
        }
      }
      return pack_container_info(root[i]);
    }
  }
  return {};
}

Container ContainerDao::get_container_by_id_or_name(const std::string& value) {
  Json::Value root;
  char line[1024];
  
  
  Json::Reader jsonreader;
  std::ifstream fin(kucker_data_file);
  if(!fin) {
    return {};
  }

  if(!jsonreader.parse(fin, root)) {
    return {};
  }

  int size = root.size();
  if(size == 0) {
    return {};
  }

  for(int i = 0; i < size; i++) {
    if(root[i]["name"].asString() == value || root[i]["id"].asString() == value) {
      if(root[i]["status"].asInt() == CONTAINER_RUNNING) {
        sprintf(line, "/proc/%d", root[i]["pid"].asInt());
        if(access(line, F_OK) == -1) {
          root[i]["status"] = CONTAINER_STOPED;
          root[i]["stop_time"] = (int)time(0);
          root[i]["pid"] = 0;
          auto str = root.toStyledString();
          // std::cout << str << std::endl;
          std::ofstream fout;
          fout.open(kucker_data_file);
          fout << str;
          fout.close();
        }
      }
      return pack_container_info(root[i]);
    }
  }
  return {};
}

std::vector<Container>* ContainerDao::list() {
  char line[1024];
  std::vector<Container> * vector = new std::vector<Container>;
  Json::Value root;
  bool dirty = false;
  
  Json::Reader jsonreader;
  std::ifstream fin(kucker_data_file);
  if(!fin) {
    return NULL;
  }

  if(!jsonreader.parse(fin, root)) {
    return NULL;
  }

  int size = root.size();
  if(size == 0) {
    return NULL;
  }

  for(int i = 0; i < size; i++) {
    if(root[i]["status"].asInt() == CONTAINER_RUNNING) {
      sprintf(line, "/proc/%d", root[i]["pid"].asInt());
      if(access(line, F_OK) == -1) {
        root[i]["status"] = CONTAINER_STOPED;
        root[i]["stop_time"] = (int)time(0);
        root[i]["pid"] = 0;
        dirty = true;
      }
    }
    vector->push_back(pack_container_info(root[i]));
  }

  if(dirty) {
    auto str = root.toStyledString();
    // std::cout << str << std::endl;
    std::ofstream fout;
    fout.open(kucker_data_file);
    fout << str;
    fout.close();
  }
  return vector;
}




char* ContainerDao::gen_id(char *uuidstr)
{
  uuid_t uuid1;
  uuid_generate(uuid1);
  uuid_unparse(uuid1, uuidstr);
  return uuidstr;
}




Container ContainerDao::pack_container_info(const Json::Value &jsonData) {
  int i = 0;
  Container info = {};
  info.id = jsonData["id"].asString();
  info.name = jsonData["name"].asString();
  info.pid = jsonData["pid"].asInt();
  info.command = jsonData["command"].asString();

  Json::Value volume = jsonData["volume"];
  int vsize = volume.size();
  if(vsize > 0) {
    for(i = 0; i < vsize; i++) {
      info.volume_list.insert(volume[i].asString());
    }
  }
  // info.volume = jsonData["volume"].asString();
  info.create_time = (time_t)jsonData["create_time"].asInt();
  info.status =  (container_status_t)jsonData["status"].asInt();
  return info;
}

Json::Value & ContainerDao::de_pack_container_info(Json::Value &jsonData, const Container &info) {
  jsonData["id"] = info.id;
  jsonData["name"] = info.name;
  jsonData["pid"] = info.pid;
  jsonData["command"] = info.command;
  jsonData["create_time"] = (int)info.create_time;
  jsonData["start_time"] = (int)info.start_time;
  jsonData["stop_time"] = (int)info.stop_time;
  jsonData["status"] = info.status;
  Json::Value volume;
  if(info.volume_list.size() > 0) {
    for(std::string v: info.volume_list) {
      volume.append(v);
    }
  }
  jsonData["volume"] = volume;
  return jsonData;
}
