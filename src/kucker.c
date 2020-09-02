#include "usg_common.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <uuid.h>

/* 定义一个给 clone 用的栈，栈大小1M */
#define STACK_SIZE (1024 * 1024)
static char container_stack[STACK_SIZE];
static const char *kucker_repo = "/data/kucker";
typedef struct arg_t
{
  int argc;
  char **argv;

} arg_t;



int pipefd[2];

void set_map(char* file, int inside_id, int outside_id, int len)
{
  FILE* mapfd = fopen(file, "w");
  if (NULL == mapfd)
  {
    perror("open file error");
    return;
  }
  fprintf(mapfd, "%d %d %d", inside_id, outside_id, len);
  fclose(mapfd);
}

void set_uid_map(pid_t pid, int inside_id, int outside_id, int len)
{
  char file[256];
  sprintf(file, "/proc/%d/uid_map", pid);
  set_map(file, inside_id, outside_id, len);
}

void set_gid_map(pid_t pid, int inside_id, int outside_id, int len)
{
  char file[256];
  sprintf(file, "/proc/%d/gid_map", pid);
  set_map(file, inside_id, outside_id, len);
}

void usage()
{
  printf("usage:\n");
}


char* gen_container_id(char *uuid1_str)
{
  uuid_t uuid1;
  uuid_generate(uuid1);
  uuid_unparse(uuid1, uuid1_str);
  fprintf(stdout, "uuid1 result: %s\n", uuid1_str);
  return uuid1_str;
}

/* 挂在容器volume */
void bind_mount(const char *container_id, const char *src, const char *_dest)
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

void mount_volume (char *container_id, char *volume) {
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

void create_container(char *container_id)
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

  sprintf(line, "test -d %s/containers/%s || mkdir %s/containers/%s", kucker_repo, container_id, kucker_repo, container_id);
  if (system(line) != 0)
  {
    perror(line);
  }
  sprintf(line, "test -d %s/aufs/diff/%s || mkdir %s/aufs/diff/%s", kucker_repo, container_id, kucker_repo, container_id);
  if (system(line) != 0)
  {
    perror(line);
  }


  sprintf(line, "cd %s && mkdir -p  bin  dev/pts dev/shm etc  home  lib  lib64  mnt  opt  proc  root  run  sbin  sys  tmp  usr  var", rootfs);
  if (system(line) != 0)
  {
    perror(line);
  }

  // mount file
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

  // ====================


}


void mount_container(const char *container_id)
{
  //remount "/proc" to make sure the "top" and "ps" show container's information
  char rootfs[1024];
  char line[8192];
  sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, container_id);
  sprintf(line, "%s/proc", rootfs);
  if (mount("proc", line, "proc", 0, NULL) != 0 )
  {
    perror("proc");
  }

  sprintf(line, "%s/tmp", rootfs);
  if (mount("none", line, "tmpfs", 0, NULL) != 0)
  {
    perror("tmp");
  }

  sprintf(line, "%s/dev/pts", rootfs);
  if (mount("devpts", line, "devpts", 0, NULL) != 0)
  {
    perror("dev/pts");
  }
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

int container_main(void* arg_)
{
  char *rootfs;
  size_t rootfs_len;
  char *cmd;
  size_t cmd_len;
  printf("in container...\n ");
 
  close(pipefd[1]);
  // char buf[1024];\
  // size_t len;
  // while((len = read(pipefd[0], buf, 1024)) > 0) {
  //   // puts("===========\n");
  //   write(1, buf, len);
  // }
  FILE * rf = fdopen(pipefd[0], "r");

  // char *line = NULL;
  //  size_t len = 0;
  //  ssize_t read;

  //  while ((read = getline(&line, &len, rf)) != -1) {
  //    printf("Retrieved line of length %zu :\n", read);
  //    printf("%s", line);
  //  }

  

  if(getline(&rootfs, &rootfs_len, rf) == -1) {
    err_exit(errno, "container_main rootfs");
  }

  if(rootfs[strlen(rootfs) - 1] == '\n') {
    rootfs[strlen(rootfs) - 1] = '\0';
  }
  //printf("rootfs=%s =====", rootfs);

  if (getline(&cmd, &cmd_len, rf) == -1) {
    err_exit(errno, "container_main cmd");
  }
  // printf("cmd=%s\n", cmd);
  fclose(rf);
  close(pipefd[0]);

  /* chroot 隔离目录 */
  if ( chroot (rootfs) != 0 || chdir("/") != 0)
  {
    err_exit(errno,"chdir/chroot:%s", rootfs);
  }
  
  char **cmd_arr;
  size_t cmd_arr_len = str_split(cmd, " ", &cmd_arr);
  if(cmd_arr_len == 0) {
    err_exit(0, "container_main: invalid command");
  }
  execvp(cmd_arr[0], cmd_arr);
  perror("exec");

  return 1;
}


void exe_run_commond (int argc, char *argv[])
{
  int c;

  bool interactive = false;
  bool detach = false;
  char *volume = NULL;
  char *name = NULL;

  char ** cmd_arr;
  char * default_cmd_arr[] =
  {
    "/bin/bash",
    "-l",
    NULL
  };
  cmd_arr = default_cmd_arr;
  int cmd_arr_len = 2;

  opterr = 0;
  while (1)
  {
    static struct option long_options[] =
    {
      /* These options set a flag. */
      {"interactive", no_argument,      0, 'i'},
      {"detach", no_argument,      0, 'd'},
      {"volume",  required_argument, 0, 'v'},
      {"name",  required_argument, 0, 'n'},
      {0, 0, 0, 0}
    };
    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long (argc, argv, "+iv:", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c)
    {
    case 0:
      // printf("ffffffff\n");
      /* If this option set a flag, do nothing else now. */
      if (long_options[option_index].flag != 0)
        break;
      printf ("option %s", long_options[option_index].name);
      if (optarg)
        printf (" with arg %s", optarg);
      printf ("\n");
      break;

    case 'i':
      // puts ("==interactive \n");
      interactive = true;
      break;
    case 'd':
      // puts ("==interactive \n");
      detach = true;
      break;

    case 'v':
      // printf ("==volume with value `%s'\n", optarg);
      volume = (optarg);
      break;

    case 'n':
      // printf ("==name with value `%s'\n", optarg);
      name = (optarg);
      break;

    case '?':
      //printf ("==? optopt=%c, %s, `%s', %d\n", optopt, optarg, argv[optind], optind);
      /* getopt_long already printed an error message. */
      break;

    default:
      //printf ("==default optopt=%c, %s, `%s'\n",optopt, optarg,  argv[optind]);
      break;
      //abort ();
    }
  }

  // printf ("optind = %d, argc=%d \n", optind, argc);
  /* Print any remaining command line arguments (not options). */
  if (optind < argc)
  {
    cmd_arr = &argv[optind];
    cmd_arr_len = argc - optind;
    // printf ("non-option ARGV-elements: ");
    // while (optind < argc)
    //   printf ("%d, %d, %s \n", optind, argc, argv[optind++]);
    // putchar ('\n');
  }
  char container_id[37];
  gen_container_id(container_id);
  create_container(container_id);

// 容器卷
  if (volume != NULL)
  {
    mount_volume(container_id, volume);
  }

  mount_container(container_id);
   
  pipe(pipefd);
  int container_pid = clone(container_main, container_stack + STACK_SIZE,
                            CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, NULL);

  close(pipefd[0]);
  char rootfs[1024];
  sprintf(rootfs, "%s/aufs/mnt/%s\n", kucker_repo, container_id);
  if(write(pipefd[1], rootfs, strlen(rootfs)) < 0) {
    perror("exe_run_commond write rootfs");
  }

  char *command_line = str_join2(cmd_arr, cmd_arr_len, " ");
  if(write(pipefd[1], command_line, strlen(command_line)) < 0) {
    perror("exe_run_commond write command_line");
  }
  printf("write command_line : %s\n", command_line);
  free(command_line);
  close(pipefd[1]);

  

  printf("Parent_pid [%5d] - Container [%5d]!\n", getpid(), container_pid);


  if(!detach) {
    waitpid(container_pid, NULL, 0);
    printf("Parent - container stopped!\n");
  } else {
    printf("%s\n", container_id);
  }
  
}





int main(int argc, char *argv[])
{
  printf("Parent [%5d] - start a container!\n", getpid());

  char *action;
  if (argc < 2)
  {
    usage();
    return 1;
  }
  else
  {
    action = argv[1];
  }
 

  if (strcmp(action, "run") == 0)
  {
    exe_run_commond(argc - 1, argv + 1);
  }
  else if ( strcmp(action, "start") == 0)
  {

  }


  return 0;
}
