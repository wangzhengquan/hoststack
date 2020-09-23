


void ContainerManager::create_container(const char *container_id)
{
  const char *unionfs = PathAssembler::getUnionFS(NULL);
  char line[1024];
  //======================
  sprintf(line, "sudo mkdir -p %s/containers/%s", kucker_repo, container_id);
  if (system(line) != 0)
  {
    LoggerFactory::getRunLogger().error(errno, line);
  }

  
  sprintf(line, "sudo mkdir -p %s", PathAssembler::getDiffDir(container_id, 0));
  if (system(line) != 0)
  {
    LoggerFactory::getRunLogger().error(errno, line);
  }

  sprintf(line, "sudo mkdir -p %s", PathAssembler::getWorkDir(container_id, 0));
  if (system(line) != 0)
  {
    LoggerFactory::getRunLogger().error(errno, line);
  }

  sprintf(line, "sudo mkdir -p %s", PathAssembler::getMergedDir(container_id, NULL));
  if (system(line) != 0)
  {
    LoggerFactory::getRunLogger().error(errno, line);
  }

  sprintf(line, "cd %s && sudo mkdir -p  bin  dev/pts dev/shm etc  home  lib  lib64  mnt  opt  proc  root  run  sbin  sys  tmp  usr  var", 
    PathAssembler::getMergedDir(container_id, NULL));
  if (system(line) != 0)
  {
    LoggerFactory::getRunLogger().error(errno, line);
  }

}


void ContainerManager::remove_container(const char *containerName) {
  Container info = ContainerManager::get_container_by_id_or_name(containerName);
  const char *container_id = info.id.c_str();
  const char *rootfs = PathAssembler::getMergedDir(container_id, NULL);
  const char *unionfs = PathAssembler::getUnionFS(NULL);
  char line[1024];

  sprintf(line, "sudo rm  -rf %s/containers/%s", kucker_repo, container_id);
  if (system(line) != 0)
  {
    LoggerFactory::getRunLogger().error(errno, line);
  }

  

  sprintf(line, "sudo rm -rf %s", PathAssembler::getLayerDir(container_id, 0));
  if (system(line) != 0)
  {
    LoggerFactory::getRunLogger().error(errno, line);
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
  {"/bin", "/bin", "overlay2"}, 
  {"/etc", "/etc", "overlay2"},
  {"/lib", "/lib", "overlay2"}, 
  {"/lib64", "/lib64", "overlay2"},  
  {"/opt","/opt", "overlay2"}, 
  {"/run", "/run", "overlay2"}, 
  {"/sbin", "/sbin", "overlay2"},
  {"/usr", "/usr", "overlay2"}, 
  {"/var", "/var", "overlay2"},
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
  const char *rootfs = PathAssembler::getMergedDir(container_id.c_str(), NULL);
  const char *unionfs = PathAssembler::getUnionFS( NULL);
  char line[1024];
  char data[1024];
  char subdiff[1024];

  size_t i = 0;
  mnt_dir_t *mnt_dir = &mnt_dir_arr[i];
  while( mnt_dir->src != NULL ) {
    if(strcmp(mnt_dir->type, "aufs") == 0) {
      // sprintf(line, "sudo mount -t aufs -o dirs=%s/diff/%s=rw:%s=ro none %s/mnt/%s%s",
      //     unionfs, container_id, mnt_dir->src, unionfs, container_id, mnt_dir->target);
      // if (system(line) != 0)
      // {
      //   LoggerFactory::getRunLogger().error(errno, line);
      // }
      char *target =  path_join(rootfs, mnt_dir->target, NULL);
      sprintf(data, "dirs=%s=rw:%s=ro", 
        PathAssembler::getDiffDir(container_id.c_str(), NULL),
        mnt_dir->src);
// printf("data=%s\n target=%s\n", data, target);
      if(mount("none", target, "aufs", 0, data) != 0) {
        LoggerFactory::getRunLogger().error(errno, "data=%s\n target=%s\n", data, target);
      }
      free(target);

    } else if(strcmp(mnt_dir->type, "overlay2") == 0) {

      // sprintf(subdiff, "%s%s", PathAssembler::getDiffDir(container_id.c_str(), NULL), mnt_dir->target);

      // sprintf(line, "test -d %s || sudo mkdir -p %s", subdiff, subdiff);
      // if (system(line) != 0)
      // {
      //   LoggerFactory::getRunLogger().error(errno, "create subdiff : %s", line);
      // }

      sprintf(line, "sudo mount -t overlay overlay -o lowerdir=%s,upperdir=%s,workdir=%s %s%s",
          mnt_dir->src, 
          PathAssembler::getDiffDir(container_id.c_str(), NULL),
          //subdiff,
          PathAssembler::getWorkDir(container_id.c_str(), NULL), 
          rootfs, mnt_dir->target
      );

      printf("%s \n \n", line);
      if (system(line) != 0)
      {
        LoggerFactory::getRunLogger().error(errno, line);
      }
    }
    else {
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
