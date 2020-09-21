#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <jsoncpp/json.h>
#include "container_manager.h"
#include "path_assembler.h"
#include "container.h"
#include "logger_factory.h"

struct mnt_dir_t {
  const char *src;
  const char *target;
  const char *type;
};


void ContainerManager::insert(const Container &info) {
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

void ContainerManager::update(const Container &info) {
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

 
void ContainerManager::change_status_to_stop( const std::string & name) {
 
  Container info = ContainerManager::get_container_by_id_or_name(name);
  info.status = CONTAINER_STOPED;
  info.stop_time = time(0);
  info.pid = 0;
  ContainerManager::update(info);
  
}




Container ContainerManager::get_container_by_id(const std::string& value) {
  return get_container_by("id", value);
}

Container ContainerManager::get_container_by_name(const std::string& value) {
  return get_container_by("name", value);
}

Container ContainerManager::get_container_by(const char * name,const std::string& value) {
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

Container ContainerManager::get_container_by_id_or_name(const std::string& value) {
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

std::vector<Container>* ContainerManager::list() {
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





void ContainerManager::create_container(const char *container_id)
{
  const char *rootfs = PathAssembler::getRootFS(container_id, NULL);
  const char *unionfs = PathAssembler::getUnionFS(NULL);
  char line[1024];
  //======================
  sprintf(line, "sudo mkdir -p %s/containers/%s", kucker_repo, container_id);
  if (system(line) != 0)
  {
    perror(line);
  }

  sprintf(line, "sudo mkdir -p %s/containers/%s", unionfs, container_id);
  // printf("%s\n", line);
  if (system(line) != 0)
  {
    perror(line);
  }
  sprintf(line, "sudo mkdir -p %s/diff/%s", unionfs, container_id);
  if (system(line) != 0)
  {
    perror(line);
  }

  sprintf(line, "test -d %s || sudo mkdir -p %s", rootfs, rootfs);
  if (system(line) != 0)
  {
    perror(line);
  }

  sprintf(line, "cd %s && sudo mkdir -p  bin  dev/pts dev/shm etc  home  lib  lib64  mnt  opt  proc  root  run  sbin  sys  tmp  usr  var", rootfs);
  if (system(line) != 0)
  {
    perror(line);
  }

}


void ContainerManager::remove_container(const char *containerName) {
  Container info = ContainerManager::get_container_by_id_or_name(containerName);
  const char *container_id = info.id.c_str();
  const char *rootfs = PathAssembler::getRootFS(container_id, NULL);
  const char *unionfs = PathAssembler::getUnionFS(NULL);
  char line[1024];

  sprintf(line, "sudo rm  -rf %s/containers/%s", kucker_repo, container_id);
  if (system(line) != 0)
  {
    perror(line);
  }

  sprintf(line, "sudo rm  -rf %s/containers/%s", unionfs, container_id);
  if (system(line) != 0)
  {
    perror(line);
  }

  sprintf(line, "sudo rm -rf %s/diff/%s", unionfs, container_id);
  if (system(line) != 0)
  {
    perror(line);
  }

  sprintf(line, "sudo rm -rf %s", rootfs);
  if (system(line) != 0)
  {
    perror(line);
  }

  // remove member of json

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


static  mnt_dir_t mnt_dir_arr[]= { 
  {"/bin", "/bin", "aufs"}, 
  {"/etc", "/etc", "aufs"},
  {"/lib", "/lib", "aufs"}, 
  {"/lib64", "/lib64", "aufs"},  
  {"/opt","/opt", "aufs"}, 
  {"/run", "/run", "aufs"}, 
  {"/sbin", "/sbin", "aufs"},
  {"/usr", "/usr", "aufs"}, 
  {"/var", "/var", "aufs"},
  {"proc", "/proc", "proc"}, 
  {"sysfs", "/sys", "sysfs"}, 
  {"udev", "/dev", "devtmpfs"},
  {"devpts", "/dev/pts", "devpts"},
  {"shm", "/dev/shm", "tmpfs"},
  
  {"none", "/tmp", "tmpfs"}, 
  {}
};



void ContainerManager::mount_container(const std::string & container_id)
{
  //remount "/proc" to make sure the "top" and "ps" show container's information
  const char *rootfs = PathAssembler::getRootFS(container_id.c_str(), NULL);
  const char *unionfs = PathAssembler::getUnionFS( NULL);
  char line[1024];
  char data[1024];

  size_t i = 0;
  mnt_dir_t *mnt_dir = &mnt_dir_arr[i];
  while( mnt_dir->src != NULL ) {
    if(strcmp(mnt_dir->type, "aufs") == 0) {
      // sprintf(line, "sudo mount -t aufs -o dirs=%s/diff/%s=rw:%s=ro none %s/mnt/%s%s",
      //     unionfs, container_id, mnt_dir->src, unionfs, container_id, mnt_dir->target);
      // if (system(line) != 0)
      // {
      //   perror(line);
      // }
      char *target =  path_join(unionfs, "/mnt", container_id.c_str(), mnt_dir->target, NULL);
      sprintf(data, "dirs=%s/diff/%s=rw:%s=ro", unionfs, container_id.c_str(), mnt_dir->src);
// printf("data=%s\n target=%s\n", data, target);
      if(mount("none", target, "aufs", 0, data) != 0) {
        LoggerFactory::getRunLogger().error(errno, "data=%s\n target=%s\n", data, target);
      }
      free(target);

    } else {
      sprintf(line, "%s%s", rootfs, mnt_dir->target);
      //printf("mount_container: %s\n", line);
      if (mount(mnt_dir->src, line, mnt_dir->type, 0, NULL) != 0)
      {
        LoggerFactory::getRunLogger().error(errno, "mount_container: %s", line);
      }
    }
    mnt_dir = &mnt_dir_arr[i];
    i++;
  }

}


char* ContainerManager::gen_id(char *uuidstr)
{
  uuid_t uuid1;
  uuid_generate(uuid1);
  uuid_unparse(uuid1, uuidstr);
  return uuidstr;
}



void ContainerManager::mount_volume_list (const char *container_id, std::set<std::string> &volume_list) {
  if(volume_list.size() == 0) {
    return;
  }
  for(std::string v : volume_list) {
     mount_volume(container_id, v.c_str());
  }
}

void ContainerManager::mount_volume (const char *container_id, const char *_volume) {
  char *src = NULL, *dest = NULL;
  char *volume = strdup(_volume);
  src = strtok(const_cast<char *>(volume), ":");
  if (src != NULL && strlen(src) > 0)
  {
    if (*src != '/')
    {
      free(volume);
      err_exit(0, "invalid mount path: '%s' mount path must be absolute.", src);
    }
    dest = strtok(NULL, ":");
    if (dest != NULL && strlen(dest) > 0)
    {
      if (*dest != '/')
      {
        free(volume);
        err_exit(0, "invalid mount path: '%s' mount path must be absolute.", dest);
      }
      bind_mount(container_id, src, dest);
    }
    else
    {
      free(volume);
      err_exit(0, "param volume format err.");
    }
  }
  else
  {
    free(volume);
    err_exit(0, "param volume format err.");
  }
}

void ContainerManager::umount_container(const std::string & container_id) {
  Container container = ContainerManager::get_container_by_id_or_name(container_id);
  const char *rootfs = PathAssembler::getRootFS(container_id.c_str(), NULL);
  char line[1024];

  size_t i = 0;
 
  while( mnt_dir_arr[i].src != NULL ) {
    i++;
  }

  mnt_dir_t *mnt_dir ;
  while ( i-- > 0) {
    mnt_dir = &mnt_dir_arr[i];
    // sprintf(line, "sudo umount %s%s", rootfs, mnt_dir->target);
    // if (system(line) != 0) {
    //   perror(line);
    // }
    sprintf(line, "%s%s", rootfs, mnt_dir->target);
    if(umount2(line, MNT_DETACH) == -1) {
      LoggerFactory::getRunLogger().error(errno, "umount_container : %s", line);
    }
    
  }

  umount_volume_list(container_id.c_str());
}


void ContainerManager::umount_volume_list (const char *container_id) {
  const std::set<std::string> &volume_list = get_container_by_id_or_name(container_id).volume_list;
  if(volume_list.size() == 0) {
    return;
  }
  for(std::string v : volume_list) {
    umount_volume(container_id, v.c_str());
  }
}

void ContainerManager::umount_volume (const char * container_id, const char* _volume) {
 
  if(_volume == NULL) {
    return;
  }
  char *volume = strdup(_volume);
  char *src = NULL, *dest = NULL;
  const char *rootfs = PathAssembler::getRootFS(container_id, NULL);

  src = strtok(volume, ":");
  if (src != NULL && strlen(src) > 0)
  {
    if (*src != '/')
    {
      err_msg(0, "invalid umount path: '%s' umount path must be absolute.", src);
    }
    dest = strtok(NULL, ":");
    if (dest != NULL && strlen(dest) > 0)
    {
      if (*dest != '/')
      {
        err_exit(0, "invalid umount path: '%s' umount path must be absolute.", dest);
      }
      char *target = path_join(rootfs, dest, NULL );
// printf("====umount_volume %s\n", target);
      if(umount(target) == -1) {
        err_msg(errno, "umount_volume umount:");
      }
      free(target);
      free(volume);
    }
    else
    {
      free(volume);
      err_msg(0, "param volume format err.");
    }
  }
  else
  {
    free(volume);
    err_msg(0, "param volume format err.");
  }
}

/* 挂在容器volume */
void ContainerManager::bind_mount(const char *container_id, const char *src, const char *_dest)
{
  char line[4096];
  const char *rootfs = PathAssembler::getRootFS(container_id, NULL);

  sprintf(line, "test -d %s || sudo mkdir -p %s", src, src);
  if (system(line) != 0)
  {
    perror(line);
  }

  char *dest = path_join(rootfs, _dest, NULL);
  sprintf(line, "test -d %s || sudo mkdir -p %s", dest, dest);
  if (system(line) != 0)
  {
    perror(line);
  }
  // LoggerFactory::getRunLogger().debug("src=%s\n dest=%s\n", src, dest);
  // sprintf(line, "sudo mount -t aufs -o dirs=%s=rw none %s", src, dest);
  // if (system(line) != 0)
  // {
  //   perror(line);
  // }

  if (mount(src, dest, "none", MS_BIND, NULL)!=0) {
      err_exit(errno, "mount_volume");
  }

  free(dest);

}




Container ContainerManager::pack_container_info(const Json::Value &jsonData) {
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

Json::Value & ContainerManager::de_pack_container_info(Json::Value &jsonData, const Container &info) {
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
