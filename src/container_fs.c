#include "container_fs.h"
#include "container_dao.h"
#include <sys/mount.h>


struct mnt_dir_t {
  const char *src;
  const char *target;
  const char *type;
};

static  mnt_dir_t mnt_dir_arr[]= { 
  {"/bin", "/bin", FS_TYPE}, 
  {"/etc", "/etc", FS_TYPE},
  {"/lib", "/lib", FS_TYPE}, 
  {"/lib64", "/lib64", FS_TYPE},  
  {"/opt","/opt", FS_TYPE}, 
  {"/run", "/run", FS_TYPE}, 
  {"/sbin", "/sbin", FS_TYPE},
  {"/usr", "/usr", FS_TYPE}, 
  {"/var", "/var", FS_TYPE},
  {"proc", "/proc", "proc"}, 
  {"sysfs", "/sys", "sysfs"}, 
  {"udev", "/dev", "devtmpfs"},
  {"devpts", "/dev/pts", "devpts"},
  {"shm", "/dev/shm", "tmpfs"},
  {"none", "/tmp", "tmpfs"}, 
  {}
};

void ContainerFs::create_repo() {
  char line[1024];
  
  sprintf(line, "%s/containers", kucker_repo);
  if (mkdir_r(line, DIR_MODE) != 0)
  {
    LoggerFactory::getRunLogger().error(errno, line);
  }

  if (mkdir_r(PathAssembler::getUnionFS(NULL), DIR_MODE) != 0)
  {
    LoggerFactory::getRunLogger().error(errno, "initRepoDir >> create unionfs");
  }

}

void ContainerFs::create_container(const char *container_id)
{
  // const char *unionfs = PathAssembler::getUnionFS(NULL);
  char line[1024];
  sprintf(line, "%s/containers/%s", kucker_repo, container_id);
  if (mkdir_r(line, DIR_MODE) != 0)
  {
    LoggerFactory::getRunLogger().error(errno, "ContainerFs::create_container: %s", line);
  }

  
  if (mkdir_r(PathAssembler::getDiffDir(container_id, 0), DIR_MODE) != 0)
  {
    LoggerFactory::getRunLogger().error(errno, "ContainerFs::create_container: %s", line);
  }

  if (mkdir_r(PathAssembler::getWorkDir(container_id, 0), DIR_MODE) != 0)
  {
    LoggerFactory::getRunLogger().error(errno, "ContainerFs::create_container: %s", line);
  }

  if (mkdir_r(PathAssembler::getMergedDir(container_id, NULL), DIR_MODE) != 0)
  {
    LoggerFactory::getRunLogger().error(errno, "ContainerFs::create_container: %s", line);
  }

  sprintf(line, "cd %s && sudo mkdir -p  bin  dev/pts dev/shm etc  home  lib  lib64  mnt  opt  proc  root  run  sbin  sys  tmp  usr  var", 
    PathAssembler::getMergedDir(container_id, NULL));
  if (system(line) != 0)
  {
    LoggerFactory::getRunLogger().error(errno, "ContainerFs::create_container: %s", line);
  }

}

void ContainerFs::remove_container(const char *container_id) {
 
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
}




void ContainerFs::mount_container(const char * container_id)
{
  //remount "/proc" to make sure the "top" and "ps" show container's information
  const char *rootfs = PathAssembler::getMergedDir(container_id, NULL);
  char line[1024];
  char data[1024];
  char subdiff[1024];
  char subwork[1024];
  char target[1024];

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
      sprintf(subdiff, "%s%s", PathAssembler::getDiffDir(container_id, NULL), mnt_dir->target);
      sprintf(target, "%s%s", rootfs, mnt_dir->target);
      if (mkdir_r(subdiff, DIR_MODE) != 0)
      {
        LoggerFactory::getRunLogger().error(errno, "create subdiff : %s", line);
      }
      sprintf(data, "dirs=%s=rw:%s=ro", subdiff, mnt_dir->src);
// printf("data=%s\n target=%s\n", data, target);
      if(mount("none", target, "aufs", 0, data) != 0) {
        LoggerFactory::getRunLogger().error(errno, "data=%s\n target=%s\n", data, target);
        exit(1);
      }

    } else if(strcmp(mnt_dir->type, "overlay2") == 0) {

      sprintf(subdiff, "%s%s", PathAssembler::getDiffDir(container_id, NULL), mnt_dir->target);
      sprintf(subwork, "%s%s", PathAssembler::getWorkDir(container_id, NULL), mnt_dir->target);
      sprintf(target, "%s%s", rootfs, mnt_dir->target);

      if (mkdir_r(subdiff, DIR_MODE) != 0)
      {
        LoggerFactory::getRunLogger().error(errno, "create subdiff : %s", line);
      }

      if (mkdir_r(subwork, DIR_MODE) != 0)
      {
        LoggerFactory::getRunLogger().error(errno, "create subwork : %s", line);
      }

      // sprintf(line, "sudo mount -t  overlay -o lowerdir=%s,upperdir=%s,workdir=%s overlay %s",
      //   mnt_dir->src, 
      //   subdiff,
      //   subwork,
      //   target
      // );

// printf("%s \n\n", line);
      // if (system(line) != 0)
      // {
      //   LoggerFactory::getRunLogger().error(errno, "mount_container mount overlay2: %s", line);
      //   exit(1);
      // }

      sprintf(data, "lowerdir=%s,upperdir=%s,workdir=%s",  mnt_dir->src, subdiff, subwork);
      if(mount("overlay", target, "overlay", 0, data) != 0) {
        LoggerFactory::getRunLogger().error(errno, "ContainerFs::mount_container overlay mount : data:%s , target:%s\n", data, target);
        exit(1);
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

void ContainerFs::mount_volume_list (const char *container_id, std::set<std::string> &volume_list) {
  if(volume_list.size() == 0) {
    return;
  }
  for(std::string v : volume_list) {
     mount_volume(container_id, v.c_str());
  }
}

void ContainerFs::mount_volume (const char *container_id, const char *_volume) {
  char *src = NULL, *dest = NULL;
  char *volume = strdup(_volume);
  src = strtok(const_cast<char *>(volume), ":");
  if (src != NULL && strlen(src) > 0)
  {
    if (*src != '/')
    {
      free(volume);
      LoggerFactory::getRunLogger().error(0, "invalid mount path: '%s' mount path must be absolute.", src);
    }
    dest = strtok(NULL, ":");
    if (dest != NULL && strlen(dest) > 0)
    {
      if (*dest != '/')
      {
        free(volume);
        LoggerFactory::getRunLogger().error(0, "invalid mount path: '%s' mount path must be absolute.", dest);
      }
      bind_mount(container_id, src, dest);
    }
    else
    {
      free(volume);
      LoggerFactory::getRunLogger().error(0, "param volume format err.");
    }
  }
  else
  {
    free(volume);
    LoggerFactory::getRunLogger().error(0, "param volume format err.");
  }
}

void ContainerFs::umount_container(const char * container_id) {
  const char *rootfs = PathAssembler::getMergedDir(container_id, NULL);
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
    //   LoggerFactory::getRunLogger().error(errno, line);
    // }
    sprintf(line, "%s%s", rootfs, mnt_dir->target);
    if(umount2(line, MNT_DETACH) == -1) {
      LoggerFactory::getRunLogger().error(errno, "umount_container : %s", line);
    }
    
  }

  umount_volume_list(container_id);
}


void ContainerFs::umount_volume_list (const char *container_id) {
  const std::set<std::string> &volume_list = ContainerDao::get_container_by_id_or_name(container_id).volume_list;
  if(volume_list.size() == 0) {
    return;
  }
  for(std::string v : volume_list) {
    umount_volume(container_id, v.c_str());
  }
}

void ContainerFs::umount_volume (const char * container_id, const char* _volume) {
 
  if(_volume == NULL) {
    return;
  }
  char *volume = strdup(_volume);
  char *src = NULL, *dest = NULL;
  const char *rootfs = PathAssembler::getMergedDir(container_id, NULL);

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
        LoggerFactory::getRunLogger().error(0, "invalid umount path: '%s' umount path must be absolute.", dest);
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
void ContainerFs::bind_mount(const char *container_id, const char *src, const char *_dest)
{
  char line[4096];
  const char *rootfs = PathAssembler::getMergedDir(container_id, NULL);

  sprintf(line, "test -d %s || sudo mkdir -p %s", src, src);
  if (system(line) != 0)
  {
    LoggerFactory::getRunLogger().error(errno, line);
  }

  char *dest = path_join(rootfs, _dest, NULL);
  sprintf(line, "test -d %s || sudo mkdir -p %s", dest, dest);
  if (system(line) != 0)
  {
    LoggerFactory::getRunLogger().error(errno, line);
  }
  // LoggerFactory::getRunLogger().debug("src=%s\n dest=%s\n", src, dest);
  // sprintf(line, "sudo mount -t aufs -o dirs=%s=rw none %s", src, dest);
  // if (system(line) != 0)
  // {
  //   LoggerFactory::getRunLogger().error(errno, line);
  // }

  if (mount(src, dest, "none", MS_BIND, NULL)!=0) {
    LoggerFactory::getRunLogger().error(errno, "mount_volume");
    exit(1);
  }

  free(dest);

}