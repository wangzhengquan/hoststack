#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

/* 定义一个给 clone 用的栈，栈大小1M */
#define STACK_SIZE (1024 * 1024)
static char container_stack[STACK_SIZE];
static const char *kucker_repo = "/data/kucker";
typedef struct arg_t
{
  int argc;
  char **argv;
    
}arg_t;

char* const container_args[] = {
    "/bin/bash",
    "-l",
    NULL
};

void usage() {
    printf("usage:\n");
}

int container_main(void* arg_)
{
   
    printf("Container [%5d] - inside the container!\n", getpid());
   
    arg_t* arg = (arg_t *)arg_;
    //set hostname
    // int rv;
    char rootfs[1024];
    char line[8192];
    char *action;
    if(arg->argc < 2) {
        usage();
        return 1;
    } else {
        action = arg->argv[1];
    }

printf("action===%s\n", action);
    const char *container_id="2";
    

    sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, container_id);

     if(strcmp(action, "run") == 0) {
        // sethostname("container",10);
        // bin  dev  etc  home  lib  lib64  lost+found  media  mnt  opt  proc  root  run  sbin  srv  sys  tmp  usr  var
        sprintf(line, "test -d %s/aufs || sudo mkdir -p %s/aufs", kucker_repo, kucker_repo);
        if(system(line) != 0) {
            perror(line);
        }
        
        sprintf(line, "test -d %s/aufs/mnt || sudo mkdir -p %s/aufs/mnt", kucker_repo, kucker_repo);
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
        sprintf(line, "test -d %s/aufs/mnt/%s || mkdir %s/aufs/mnt/%s", kucker_repo, container_id, kucker_repo, container_id);
        if(system(line) != 0) {
            perror(line);
        }

        sprintf(line, "cd %s/aufs/mnt/%s && mkdir -p  bin  dev/pts dev/shm etc  home  lib  lib64  mnt  opt  proc  root  run  sbin  sys  tmp  usr  var", kucker_repo, container_id);
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
    }
    
   
   if(strcmp(action, "run") == 0 || strcmp(action, "start") == 0) {
        //remount "/proc" to make sure the "top" and "ps" show container's information
        sprintf(line, "%s/proc", rootfs);
        if (mount("proc", line, "proc", 0, NULL) !=0 ) {
            perror("proc");
        }
        sprintf(line, "%s/sys", rootfs);
        if (mount("sysfs", line, "sysfs", 0, NULL)!=0) {
            perror("sys");
        }
        sprintf(line, "%s/tmp", rootfs);
        if (mount("none", line, "tmpfs", 0, NULL)!=0) {
            perror("tmp");
        }
        sprintf(line, "%s/dev", rootfs);
        if (mount("udev", line, "devtmpfs", 0, NULL)!=0) {
            perror("dev");
        }
        sprintf(line, "%s/dev/pts", rootfs);
        if (mount("devpts", line, "devpts", 0, NULL)!=0) {
            perror("dev/pts");
        }
        // sprintf(line, "%s/dev/shm", rootfs);
        // if (mount("shm", line, "tmpfs", 0, NULL)!=0) {
        //     perror("dev/shm");
        // }
        sprintf(line, "%s/run", rootfs);
        if (mount("tmpfs", line, "tmpfs", 0, NULL)!=0) {
            perror("run");
        }
        /* 
         * 模仿Docker的从外向容器里mount相关的配置文件 
         * 你可以查看：/var/lib/docker/containers/<container_id>/目录，
         * 你会看到docker的这些文件的。
         */
        // if (mount("conf/hosts", "rootfs/etc/hosts", "none", MS_BIND, NULL)!=0 ||
        //       mount("conf/hostname", "rootfs/etc/hostname", "none", MS_BIND, NULL)!=0 ||
        //       mount("conf/resolv.conf", "rootfs/etc/resolv.conf", "none", MS_BIND, NULL)!=0 ) {
        //     perror("conf");
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
        execv(container_args[0], container_args);
        perror("exec");
    }
  
    printf("Something's wrong!\n");

    return 1;
}

int main(int argc, char *argv[])
{
    printf("Parent [%5d] - start a container!\n", getpid());
    
    arg_t arg = {argc, argv};
    // arg.argc = ;
    // arg.argv = argv;
    // int rv;
    // char mnt[1024];
    // char rootfs[1024];
    // char line[8192];
    // char *action = argv[1];
    // char *container_id="2";
   
    int container_pid = clone(container_main, container_stack+STACK_SIZE, 
        CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, &arg);
    // sleep(5);
    waitpid(container_pid, NULL, 0);
    printf("Parent - container stopped!\n");
    return 0;
}