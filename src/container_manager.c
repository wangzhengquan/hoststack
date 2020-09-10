#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <jsoncpp/json.h>
#include "container_manager.h"
#include "container.h"

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
	infojson["id"] = info.id;
	infojson["name"] = info.name;
	infojson["pid"] = info.pid;
	infojson["command"] = info.command;
	infojson["create_time"] = (int)info.create_time;
	infojson["status"] = info.status;
  infojson["volume"] = info.volume;

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

 
void ContainerManager::save_to_stop( Container &info) {
  info.status = CONTAINER_STOPED;
  info.pid = 0;
  ContainerManager::update(info);
  ContainerManager::umount_container(info.id.c_str());
}

void ContainerManager::stop(const std::string & name) {
  Container container = ContainerManager::get_container_by_id_or_name(name);
  if(container.id.empty() || container.status != CONTAINER_RUNNING) {
    err_msg(0, "No container identify by %s, or it's not a container in running.", name.c_str());
    return;
  }
  printf("killing pid %d\n", container.pid);
  if(kill(container.pid, SIGTERM) != 0) {
    err_exit(errno, "SIGTERM Stop container %s failed.", name.c_str());
    return;
  }

  sleep(3);
  if(kill(container.pid, SIGKILL) != 0) {
    //err_msg(errno, "SIGKILL Stop container %s failed.", name.c_str());
  }

  
  save_to_stop(container);
 
}



Container ContainerManager::get_container_by_id(const std::string& value) {
  return get_container_by("id", value);
}

Container ContainerManager::get_container_by_name(const std::string& value) {
  return get_container_by("name", value);
}

Container ContainerManager::get_container_by(const char * name,const std::string& value) {
  Json::Value root;
  
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
    if(root[i][name] == value) {
      return pack_container_info(root[i]);
    }
  }
  return {};
}

Container ContainerManager::get_container_by_id_or_name(const std::string& value) {
  Json::Value root;
  
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
    if(root[i]["name"] == value || root[i]["id"] == value) {
      return pack_container_info(root[i]);
    }
  }
  return {};
}

std::vector<Container>* ContainerManager::list(container_ls_opt_t &opt) {
  
  std::vector<Container> * vector = new std::vector<Container>;
  Json::Value root;
  
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
    if(opt.all)
      vector->push_back(pack_container_info(root[i]));
    else if(root[i]["status"].asInt() == CONTAINER_RUNNING) {
      vector->push_back(pack_container_info(root[i]));
    }
  }
  return vector;
}




Container ContainerManager::pack_container_info(const Json::Value &jsonData) {
  Container info = {};
  info.id = jsonData["id"].asString();
  info.name = jsonData["name"].asString();
  info.pid = jsonData["pid"].asInt();
  info.command = jsonData["command"].asString();
  info.volume = jsonData["volume"].asString();
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
  jsonData["status"] = info.status;
  jsonData["volume"] = info.volume;
  return jsonData;
}



void ContainerManager::create_container(const char *container_id)
{
  char rootfs[1024];
  char line[8192];


  sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, container_id);


  // sethostname("container",10);
  // bin  dev  etc  home  lib  lib64  lost+found  media  mnt  opt  proc  root  run  sbin  srv  sys  tmp  usr  var


  sprintf(line, "test -d %s || sudo mkdir -p %s", rootfs, rootfs);
  if (system(line) != 0)
  {
    perror(line);
  }
  sprintf(line, "test -d %s/aufs/diff || sudo mkdir -p %s/aufs/diff", kucker_repo, kucker_repo);
  if (system(line) != 0)
  {
    perror(line);
  }
  sprintf(line, "test -d %s/aufs/layers || sudo mkdir -p %s/aufs/layers", kucker_repo, kucker_repo);
  if (system(line) != 0)
  {
    perror(line);
  }
  sprintf(line, "test -d %s/aufs/containers || sudo mkdir -p %s/aufs/containers", kucker_repo, kucker_repo);
  if (system(line) != 0)
  {
    perror(line);
  }

  sprintf(line, "test -d %s/containers/%s || sudo mkdir -p %s/containers/%s", kucker_repo, container_id, kucker_repo, container_id);
  printf("%s\n", line);
  if (system(line) != 0)
  {
    perror(line);
  }
  sprintf(line, "test -d %s/aufs/diff/%s || sudo mkdir -p %s/aufs/diff/%s", kucker_repo, container_id, kucker_repo, container_id);
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
  //{"/dev", "/dev", "aufs"},
  {"proc", "/proc", "proc"}, 
  {"sysfs", "/sys", "sysfs"}, 
  {"udev", "/dev", "devtmpfs"},
  {"devpts", "/dev/pts", "devpts"},
  {"shm", "/dev/shm", "tmpfs"},
  
  {"none", "/tmp", "tmpfs"}, 
  {}
};

void ContainerManager::umount_container(const char *container_id) {
  char rootfs[1024];
  char line[4096];
  sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, container_id);

  size_t i = 0;
 
  while( mnt_dir_arr[i].src != NULL ) {
    i++;
  }

  mnt_dir_t *mnt_dir ;
  while ( i-- > 0) {
    mnt_dir = &mnt_dir_arr[i];
    sprintf(line, "%s%s", rootfs, mnt_dir->target);
    printf("umount %s\n", line);
    if(umount(line) == -1) {
      err_msg(errno, "umount_container umount %s", line);
    }
    
  }

  umount_volume(container_id);
}

void ContainerManager::mount_container(const char *container_id)
{
  //remount "/proc" to make sure the "top" and "ps" show container's information
  char rootfs[1024];
  sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, container_id);
  char line[1024];
  char data[1024];

  size_t i = 0;
  mnt_dir_t *mnt_dir = &mnt_dir_arr[i];
  while( mnt_dir->src != NULL ) {
    if(strcmp(mnt_dir->type, "aufs") == 0) {
      // sprintf(line, "sudo mount -t aufs -o dirs=%s/aufs/diff/%s=rw:%s=ro none %s/aufs/mnt/%s%s",
      //     kucker_repo, container_id, mnt_dir->src, kucker_repo, container_id, mnt_dir->target);
      // if (system(line) != 0)
      // {
      //   perror(line);
      // }
      char *target =  path_join(kucker_repo, "/aufs/mnt", container_id, mnt_dir->target, NULL);
      sprintf(data, "dirs=%s/aufs/diff/%s=rw:%s=ro", kucker_repo, container_id, mnt_dir->src);
// printf("data=%s\n target=%s\n", data, target);
      if(mount("none", target, "aufs", 0, data) != 0) {
        err_exit(errno, "data=%s\n target=%s\n", data, target);
      }
      free(target);

    } else {
      sprintf(line, "%s%s", rootfs, mnt_dir->target);
      printf("mount_container: %s\n", line);
      if (mount(mnt_dir->src, line, mnt_dir->type, 0, NULL) != 0)
      {
        err_exit(errno, "mount_container: %s", line);
      }
    }
    mnt_dir = &mnt_dir_arr[i];
    i++;
  }

 
 

  // ===================

 
  // sprintf(line, "%s/dev", rootfs);
  // if (mount("udev", line, "devtmpfs", 0, NULL) != 0)
  // {
  //   perror("dev");
  // }

  // sprintf(line, "%s/dev/pts", rootfs);
  // if (mount("devpts", line, "devpts", 0, NULL) != 0)
  // {
  //   err_exit(errno, "mount_container dev/pts: %s", line);
  // }
  // sprintf(line, "%s/dev/net", rootfs);
  // if (mount("net", line, "net", 0, NULL)!=0) {
  //     perror("dev/net");
  // }
  // sprintf(line, "%s/dev/shm", rootfs);
  // if (mount("shm", line, "tmpfs", 0, NULL)!=0) {
  //     perror("dev/shm");
  // }

  // sprintf(line, "%s/proc", rootfs);
  // if (mount("proc", line, "proc", 0, NULL) != 0 )
  // {
  //   err_exit(errno, "mount_container proc: %s", line);
  // }

  // sprintf(line, "%s/tmp", rootfs);
  // if (mount("none", line, "tmpfs", 0, NULL) != 0)
  // {
  //   err_exit(errno, "mount_container tmp: %s", line);
  // }

  // sprintf(line, "%s/run", rootfs);
  // if (mount("tmpfs", line, "tmpfs", 0, NULL)!=0) {
  //     perror("run");
  // }
  /*
   * 模仿Docker的从外向容器里mount相关的配置文件
   * 你可以查看：/var/lib/docker/containers/<container_id>/目录，
   * 你会看到docker的这些文件的。
   */
  // sprintf(line, "%s/etc/hosts", cwd);
  // if (mount("etc/hosts", line, "none", MS_BIND, NULL)!=0) {
  //      perror("etc/hosts");
  // }

  // sprintf(line, "%s/etc/hostname", cwd);
  // if (mount("etc/hostname", line, "none", MS_BIND, NULL)!=0) {
  //      perror("etc/hostname");
  // }

  // sprintf(line, "%s/etc/resolv.conf", cwd);
  // if (mount("etc/resolv.conf", line, "none", MS_BIND, NULL)!=0) {
  //      perror("etc/resolv.conf");
  // }

}


char* ContainerManager::gen_id(char *uuidstr)
{
  uuid_t uuid1;
  uuid_generate(uuid1);
  uuid_unparse(uuid1, uuidstr);
  return uuidstr;
}


void ContainerManager::mount_volume (const char *container_id, char *volume) {
   char *src = NULL, *dest = NULL;
    src = strtok(volume, ":");
    if (src != NULL && strlen(src) > 0)
    {
      if (*src != '/')
      {
        err_exit(0, "invalid mount path: '%s' mount path must be absolute.", src);
      }
      dest = strtok(NULL, ":");
      if (dest != NULL && strlen(dest) > 0)
      {
        if (*dest != '/')
        {
          err_exit(0, "invalid mount path: '%s' mount path must be absolute.", dest);
        }
        bind_mount(container_id, src, dest);
      }
      else
      {
        err_exit(0, "param volume format err.");
      }
    }
    else
    {
      err_exit(0, "param volume format err.");
    }
}


void ContainerManager::umount_volume (const char *container_id) {
  std::string _volume = get_container_by_id(container_id).volume;
  if(_volume.empty()) {
    return;
  }
  char *volume = strdup(_volume.c_str());
  char *src = NULL, *dest = NULL;
  char rootfs[1024];
  // char line[4096];
  sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, container_id);

  src = strtok(volume, ":");
  if (src != NULL && strlen(src) > 0)
  {
    if (*src != '/')
    {
      err_msg(0, "invalid mount path: '%s' mount path must be absolute.", src);
    }
    dest = strtok(NULL, ":");
    if (dest != NULL && strlen(dest) > 0)
    {
      if (*dest != '/')
      {
        err_exit(0, "invalid mount path: '%s' mount path must be absolute.", dest);
      }
      char *target = path_join(rootfs, dest, NULL );
printf("====umount_volume %s\n", target);
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
  char rootfs[1024];
  char line[4096];
  sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, container_id);

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
  printf("src=%s\n dest=%s\n", src, dest);
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
