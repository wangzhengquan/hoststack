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
    
}arg_t;



int pipefd[2];

void set_map(char* file, int inside_id, int outside_id, int len) {
    FILE* mapfd = fopen(file, "w");
    if (NULL == mapfd) {
        perror("open file error");
        return;
    }
    fprintf(mapfd, "%d %d %d", inside_id, outside_id, len);
    fclose(mapfd);
}

void set_uid_map(pid_t pid, int inside_id, int outside_id, int len) {
    char file[256];
    sprintf(file, "/proc/%d/uid_map", pid);
    set_map(file, inside_id, outside_id, len);
}

void set_gid_map(pid_t pid, int inside_id, int outside_id, int len) {
    char file[256];
    sprintf(file, "/proc/%d/gid_map", pid);
    set_map(file, inside_id, outside_id, len);
}

void usage() {
    printf("usage:\n");
}


char* gen_container_id(char *uuid1_str) {
    uuid_t uuid1;
    uuid_generate(uuid1);
    uuid_unparse(uuid1, uuid1_str);
    fprintf(stdout, "uuid1 result: %s\n", uuid1_str);
    return uuid1_str;
}

void mount_volume(const char *container_id, const char *src, const char *dest) {
    char rootfs[1024];
    char line[8192];
    sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, container_id);


    

    sprintf(line, "test -d %s || sudo mkdir -p %s", src, src);
    if(system(line) != 0) {
        perror(line);
    }

    char *dest_tmp = path_join(2, rootfs, dest);
    sprintf(line, "test -d %s || sudo mkdir -p %s", dest_tmp, dest_tmp);
    if(system(line) != 0) {
        perror(line);
    }
printf("src=%s\n dest=%s\n", src, dest_tmp);
    sprintf(line, "sudo mount -t aufs -o dirs=%s=rw none %s", src, dest_tmp);
    if(system(line) != 0) {
        perror(line);
    }

    free(dest_tmp);

}

void create_container(char *container_id) {
    char rootfs[1024];
    char line[8192];
     

    sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, container_id);

     
    // sethostname("container",10);
    // bin  dev  etc  home  lib  lib64  lost+found  media  mnt  opt  proc  root  run  sbin  srv  sys  tmp  usr  var
    
    
    sprintf(line, "test -d %s || sudo mkdir -p %s", rootfs, rootfs);
    if(system(line) != 0) {
        perror(line);
    }
    sprintf(line, "test -d %s/aufs/diff || sudo mkdir -p %s/aufs/diff", kucker_repo, kucker_repo);
    if(system(line) != 0) {
        perror(line);
    }
    sprintf(line, "test -d %s/aufs/layers || sudo mkdir -p %s/aufs/layers", kucker_repo, kucker_repo);
    if(system(line) != 0) {
        perror(line);
    }
    sprintf(line, "test -d %s/aufs/containers || sudo mkdir -p %s/aufs/containers", kucker_repo, kucker_repo);
    if(system(line) != 0) {
        perror(line);
    }
   
    sprintf(line, "test -d %s/containers/%s || mkdir %s/containers/%s", kucker_repo, container_id, kucker_repo, container_id);
    if(system(line) != 0) {
        perror(line);
    }
    sprintf(line, "test -d %s/aufs/diff/%s || mkdir %s/aufs/diff/%s", kucker_repo, container_id, kucker_repo, container_id);
    if(system(line) != 0) {
        perror(line);
    }
   

    sprintf(line, "cd %s && mkdir -p  bin  dev/pts dev/shm etc  home  lib  lib64  mnt  opt  proc  root  run  sbin  sys  tmp  usr  var", rootfs);
    if(system(line) != 0) {
        perror(line);
    }

    // mount file
    sprintf(line, "sudo mount -t aufs -o dirs=%s/aufs/diff/%s=rw:/bin=ro none %s/aufs/mnt/%s/bin", 
        kucker_repo, container_id, kucker_repo, container_id);
    if(system(line) != 0) {
        perror(line);
    }

    sprintf(line, "sudo mount -t aufs -o dirs=%s/aufs/diff/%s=rw:/lib=ro none %s/aufs/mnt/%s/lib", 
        kucker_repo, container_id, kucker_repo, container_id);
    if(system(line) != 0) {
        perror(line);
    }

    sprintf(line, "sudo mount -t aufs -o dirs=%s/aufs/diff/%s=rw:/lib64=ro none %s/aufs/mnt/%s/lib64", 
        kucker_repo, container_id, kucker_repo, container_id);
    if(system(line) != 0) {
        perror(line);
    }

    sprintf(line, "sudo mount -t aufs -o dirs=%s/aufs/diff/%s=rw:/sbin=ro none %s/aufs/mnt/%s/sbin", 
        kucker_repo, container_id, kucker_repo, container_id);
    if(system(line) != 0) {
        perror(line);
    }

    sprintf(line, "sudo mount -t aufs -o dirs=%s/aufs/diff/%s=rw:/usr=ro none %s/aufs/mnt/%s/usr", 
        kucker_repo, container_id, kucker_repo, container_id);
    if(system(line) != 0) {
        perror(line);
    }

    sprintf(line, "sudo mount -t aufs -o dirs=%s/aufs/diff/%s=rw:/var=ro none %s/aufs/mnt/%s/var", 
        kucker_repo, container_id, kucker_repo, container_id);
    if(system(line) != 0) {
        perror(line);
    }

    sprintf(line, "sudo mount -t aufs -o dirs=%s/aufs/diff/%s=rw:/etc=ro none %s/aufs/mnt/%s/etc", 
        kucker_repo, container_id, kucker_repo, container_id);
    if(system(line) != 0) {
        perror(line);
    }

    sprintf(line, "sudo mount -t aufs -o dirs=%s/aufs/diff/%s=rw:/run=ro none %s/aufs/mnt/%s/run", 
        kucker_repo, container_id, kucker_repo, container_id);
    if(system(line) != 0) {
        perror(line);
    }
     sprintf(line, "sudo mount -t aufs -o dirs=%s/aufs/diff/%s=rw:/opt=ro none %s/aufs/mnt/%s/opt", 
        kucker_repo, container_id, kucker_repo, container_id);
    if(system(line) != 0) {
        perror(line);
    }

    // ===================

    sprintf(line, "%s/sys", rootfs);
    if (mount("sysfs", line, "sysfs", 0, NULL)!=0) {
        perror("sys");
    }
    sprintf(line, "%s/dev", rootfs);
    if (mount("udev", line, "devtmpfs", 0, NULL)!=0) {
        perror("dev");
    }

    // ====================


}


void start_container(const char *container_id, int argc, char *argv[]) {
    //remount "/proc" to make sure the "top" and "ps" show container's information
    char rootfs[1024];
    char line[8192];
    sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, container_id);
    sprintf(line, "%s/proc", rootfs);
    if (mount("proc", line, "proc", 0, NULL) !=0 ) {
        perror("proc");
    }
    
    sprintf(line, "%s/tmp", rootfs);
    if (mount("none", line, "tmpfs", 0, NULL)!=0) {
        perror("tmp");
    }
   
    sprintf(line, "%s/dev/pts", rootfs);
    if (mount("devpts", line, "devpts", 0, NULL)!=0) {
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



    /* 模仿docker run命令中的 -v, --volume=[] 参数干的事 */
    // sprintf(mnt, "%s/mnt", rootfs);
    // if (mount("/tmp/t1", mnt, "none", MS_BIND, NULL)!=0) {
    //     perror("mnt");
    // }
    /* chroot 隔离目录 */
    if ( chdir(rootfs) != 0 || chroot("./") != 0 ){
        perror("chdir/chroot");
    }
    execvp(argv[0], argv);
    perror("exec");
}

void exe_run_commond (int argc, char *argv[]) {
  int c;

  bool interactive = false;
  char *volume = NULL;
  char *name = NULL;
  char ** cmd_arr;
  char * default_cmd_arr[] = {
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
        printf("ffffffff\n");
        /* If this option set a flag, do nothing else now. */
        if (long_options[option_index].flag != 0)
          break;
        printf ("option %s", long_options[option_index].name);
        if (optarg)
          printf (" with arg %s", optarg);
        printf ("\n");
        break;

      case 'i':
        puts ("==interactive \n");
        interactive = true;
        break;

      case 'v':
        printf ("==volume with value `%s'\n", optarg);
        volume = (optarg);
        break;

      case 'n':
        printf ("==name with value `%s'\n", optarg);
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

  printf ("optind = %d, argc=%d \n", optind, argc);
   // printf ("%d,  %s \n", 7, argv[7]);
   /* Print any remaining command line arguments (not options). */
  if (optind < argc)
  {
      cmd_arr = &argv[optind];
      cmd_arr_len = argc - optind;
      printf ("non-option ARGV-elements: ");
      while (optind < argc)
        printf ("%d, %d, %s \n", optind, argc, argv[optind++]);
      putchar ('\n');
  }
  char container_id[37];
  gen_container_id(container_id);
  create_container(container_id);

  if(volume != NULL) {
    char *src=NULL, *dest=NULL;
    src = strtok(volume, ":");
    if(src != NULL && strlen(src) > 0) {
        printf("%d, %d, %d\n", *src, '/', *src == '/');
        if(*src != '/') {
            err_exit(0, "invalid mount path: '%s' mount path must be absolute.", src);
        }
        dest = strtok(NULL, ":");
        if(dest != NULL && strlen(dest) > 0) {
            if(*dest != '/') {
                err_exit(0, "invalid mount path: '%s' mount path must be absolute.", dest);
            }
            mount_volume(container_id, src, dest);
        } else {
            err_exit(0, "param volume format err.");
        }
    } else {
        err_exit(0, "param volume format err.");
    }
     
  }

  start_container(container_id, cmd_arr_len, cmd_arr);

  printf("%s", container_id);
}




int container_main(void* arg_)
{
   
    printf("Container [%5d] - inside the container!\n", getpid());

    printf("Container: eUID = %ld;  eGID = %ld, UID=%ld, GID=%ld\n",
            (long) geteuid(), (long) getegid(), (long) getuid(), (long) getgid());

    /* 等待父进程通知后再往下执行（进程间的同步） */
    // char ch;
    // close(pipefd[1]);
    // if(read(pipefd[0], &ch, 1) == -1) {
    //     perror("read pipefd");
    // }
   
    arg_t* arg = (arg_t *)arg_;
    //set hostname
    // int rv;
    // int argc = arg->argc; 
    // char *argv[] = arg->argv;

    char *action;
    if(arg->argc < 2) {
        usage();
        return 1;
    } else {
        action = arg->argv[1];
    }
//     char cwd[1024];
//     if(getcwd(cwd, 1024)== NULL) {
//         perror("cwd");
//     }

// printf("cwd==%s\n", cwd);
// printf("action===%s\n", action);
    
    if(strcmp(action, "run") == 0) {
        exe_run_commond(arg->argc - 1, arg->argv + 1);
    } else if( strcmp(action, "start") == 0) {
       
    }
  
    printf("Something's wrong!\n");

    return 1;
}

int main(int argc, char *argv[])
{
    printf("Parent [%5d] - start a container!\n", getpid());
    
    arg_t arg = {argc, argv};
    const int gid=getgid(), uid=getuid();

    printf("Parent: eUID = %ld;  eGID = %ld, UID=%ld, GID=%ld\n",
            (long) geteuid(), (long) getegid(), (long) getuid(), (long) getgid());

    // if (pipe(pipefd) == -1) {
    //     perror("main pipe");
    // }
   
    int container_pid = clone(container_main, container_stack+STACK_SIZE, 
        CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, &arg);

    printf("Parent_pid [%5d] - Container [%5d]!\n", getpid(), container_pid);

    //To map the uid/gid, 
    //   we need edit the /proc/PID/uid_map (or /proc/PID/gid_map) in parent
    //The file format is
    //   ID-inside-ns   ID-outside-ns   length
    //if no mapping, 
    //   the uid will be taken from /proc/sys/kernel/overflowuid
    //   the gid will be taken from /proc/sys/kernel/overflowgid

    // set_uid_map(container_pid, 0, uid, 1);
    // set_gid_map(container_pid, 0, gid, 1);
    // printf("Parent [%5d] - user/group mapping done!\n", getpid());
    /* 通知子进程 */
    // close(pipefd[1]);

    waitpid(container_pid, NULL, 0);
    printf("Parent - container stopped!\n");
    return 0;
}