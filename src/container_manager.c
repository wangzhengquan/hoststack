#include "container_manager.h"
#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <jsoncpp/json.h>

char* gen_container_id(char *uuid1_str);
void bind_mount(const char *container_id, const char *src, const char *_dest);
void mount_volume (char *container_id, char *volume);

struct mnt_dir_t {
  const char *src;
  const char *target;
  const char *type;
};


void ContainerManager::save(container_info_t &info) {

	Json::Value root;
// printf("==============2\n");
	root["id"] = info.id;
	root["name"] = info.name;
	root["pid"] = info.pid;
	root["command"] = info.command;
	root["create_time"] = (int)info.create_time;
	root["status"] = info.status;

  auto str = root.toStyledString();
  std::cout << str << std::endl;

// printf("==============3\n");
  
	std::ostringstream info_location;
	info_location << kucker_repo << "/containers/" << info.id << "/config.json";
  std::ofstream ofss;
  ofss.open(info_location.str());
  ofss << str;
  ofss.close();
}


std::vector<container_info_t>* ContainerManager::list() {
	Json::Reader jsonreader;
 
	DIR *dirp;
  struct dirent *dp;
  bool isCurrent;          /* True if 'dirpath' is "." */
  std::string dirpath = std::string(kucker_repo) + "/containers";
  std::string configFile;
  container_info_t info;
  Json::Value jsonData;
  std::vector<container_info_t> * vector = new std::vector<container_info_t>;

  isCurrent = dirpath == ".";

  dirp = opendir(dirpath.c_str());
  if (dirp  == NULL) {
      err_msg(0, "opendir failed on '%s'", dirpath.c_str());
      return NULL;
  }

  /* For each entry in this directory, print directory + filename */

  for (;;) {
      errno = 0;              /* To distinguish error from end-of-directory */
      dp = readdir(dirp);
      if (dp == NULL)
          break;

      if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
          continue;           /* Skip . and .. */

      configFile = dirpath + "/" + dp->d_name + "/config.json";
    	std::cout << "configFile : " << configFile << std::endl;
      std::ifstream fin(configFile); 
      if(!fin) {
      	err_msg(errno, "ContainerManager list fin:");
      	printf("!fin\n");
      	fin.close();
      	continue;
      }

      if(jsonreader.parse(fin, jsonData)) {

      	info.id = jsonData["id"].asString();
        info.name = jsonData["name"].asString();
        info.pid = jsonData["pid"].asInt();
        info.command = jsonData["command"].asString();
				info.create_time = (time_t)jsonData["create_time"].asInt();
		 		info.status =  (container_status_t)jsonData["status"].asInt();
		 		vector->push_back(info);

		 		printf("parse ===== %d\n", info.pid);
      }
      printf("finished\n");
      fin.close();
  }

  if (errno != 0)
      err_msg(errno, "list readdir");

  if (closedir(dirp) == -1)
      err_msg(errno, "closedir");
   return vector;
}


void ContainerManager::umount_container(const char *container_id) {
  
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

  // ====== mount file
  // if (mount("none", "%s/aufs/mnt/%s/bin", "aufs", flags, "dirs=%s/aufs/diff/%s=rw:/bin=ro") == -1) {

  // }
 


}



void ContainerManager::mount_container(const char *rootfs)
{
  //remount "/proc" to make sure the "top" and "ps" show container's information
  // char rootfs[1024];
  
  // sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, container_id);
  char line[8192];

   sprintf(line, "sudo mount -t aufs -o dirs=%s/aufs/diff/%s=rw:/bin=ro none %s/aufs/mnt/%s/bin",
          kucker_repo, container_id, kucker_repo, container_id);
  if (system(line) != 0)
  {
    perror(line);
  }

  sprintf(line, "sudo mount -t aufs -o dirs=%s/aufs/diff/%s=rw:/lib=ro none %s/aufs/mnt/%s/lib",
          kucker_repo, container_id, kucker_repo, container_id);
  if (system(line) != 0)
  {
    perror(line);
  }

  sprintf(line, "sudo mount -t aufs -o dirs=%s/aufs/diff/%s=rw:/lib64=ro none %s/aufs/mnt/%s/lib64",
          kucker_repo, container_id, kucker_repo, container_id);
  if (system(line) != 0)
  {
    perror(line);
  }

  sprintf(line, "sudo mount -t aufs -o dirs=%s/aufs/diff/%s=rw:/sbin=ro none %s/aufs/mnt/%s/sbin",
          kucker_repo, container_id, kucker_repo, container_id);
  if (system(line) != 0)
  {
    perror(line);
  }

  sprintf(line, "sudo mount -t aufs -o dirs=%s/aufs/diff/%s=rw:/usr=ro none %s/aufs/mnt/%s/usr",
          kucker_repo, container_id, kucker_repo, container_id);
  if (system(line) != 0)
  {
    perror(line);
  }

  sprintf(line, "sudo mount -t aufs -o dirs=%s/aufs/diff/%s=rw:/var=ro none %s/aufs/mnt/%s/var",
          kucker_repo, container_id, kucker_repo, container_id);
  if (system(line) != 0)
  {
    perror(line);
  }

  sprintf(line, "sudo mount -t aufs -o dirs=%s/aufs/diff/%s=rw:/etc=ro none %s/aufs/mnt/%s/etc",
          kucker_repo, container_id, kucker_repo, container_id);
  if (system(line) != 0)
  {
    perror(line);
  }

  sprintf(line, "sudo mount -t aufs -o dirs=%s/aufs/diff/%s=rw:/run=ro none %s/aufs/mnt/%s/run",
          kucker_repo, container_id, kucker_repo, container_id);
  if (system(line) != 0)
  {
    perror(line);
  }
  sprintf(line, "sudo mount -t aufs -o dirs=%s/aufs/diff/%s=rw:/opt=ro none %s/aufs/mnt/%s/opt",
          kucker_repo, container_id, kucker_repo, container_id);
  if (system(line) != 0)
  {
    perror(line);
  }

  // ===================

  sprintf(line, "%s/sys", rootfs);
  if (mount("sysfs", line, "sysfs", 0, NULL) != 0)
  {
    perror("sys");
  }
  sprintf(line, "%s/dev", rootfs);
  if (mount("udev", line, "devtmpfs", 0, NULL) != 0)
  {
    perror("dev");
  }

  sprintf(line, "%s/proc", rootfs);
  if (mount("proc", line, "proc", 0, NULL) != 0 )
  {
    err_exit(errno, "mount_container proc: %s", line);
  }

  sprintf(line, "%s/tmp", rootfs);
  if (mount("none", line, "tmpfs", 0, NULL) != 0)
  {
    err_exit(errno, "mount_container tmp: %s", line);
  }

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

static const mnt_dir_t mnt_dir_arr[]= { {"/bin", "/bin", "aufs"}, {"/etc", "/etc", "aufs"},
  {"/lib", "/lib", "aufs"}, {"/lib64", "/lib64", "aufs"},  
  {"/opt","/opt", "aufs"}, {"/run", "/run", "aufs"}, {"/sbin", "/sbin", "aufs"},
  {"/usr", "/usr", "aufs"}, {"/var", "/var", "aufs"},
  {"proc", "/proc", "proc"}, 
  {"sysfs", "/sys", "sysfs"}, 
  {"udev", "/dev", "devtmpfs"},
  {"none", "/tmp", "tmpfs"}, {}};


char* ContainerManager::gen_id(char *uuidstr)
{
  uuid_t uuid1;
  uuid_generate(uuid1);
  uuid_unparse(uuid1, uuidstr);
  return uuidstr;
}


void ContainerManager::mount_volume (char *container_id, char *volume) {
   char *src = NULL, *dest = NULL;
    src = strtok(volume, ":");
    if (src != NULL && strlen(src) > 0)
    {
      printf("%d, %d, %d\n", *src, '/', *src == '/');
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




/* 挂在容器volume */
void ContainerManager::bind_mount(const char *container_id, const char *src, const char *_dest)
{
  char rootfs[1024];
  char line[8192];
  sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, container_id);




  sprintf(line, "test -d %s || sudo mkdir -p %s", src, src);
  if (system(line) != 0)
  {
    perror(line);
  }

  char *dest = path_join(2, rootfs, _dest);
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
      perror("mount_volume");
  }

  free(dest);

}
