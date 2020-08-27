#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include "usg_common.h"

/* 定义一个给 clone 用的栈，栈大小1M */
#define STACK_SIZE (1024 * 1024)
static char container_stack[STACK_SIZE];
char* const container_args[] = {
    "/bin/bash",
    "-l",
    NULL
};
int container_main(void* arg)
{
    printf("Container [%5d] - inside the container!\n", getpid());
    //set hostname
    const char *rootfs = "/var/lib/kucker/aufs/1/mnt";
    char mnt[1024];
    sethostname("container",10);
    // bin  dev  etc  home  lib  lib64  lost+found  media  mnt  opt  proc  root  run  sbin  srv  sys  tmp  usr  var

   
    //remount "/proc" to make sure the "top" and "ps" show container's information
    sprintf(mnt, "%s/proc", rootfs);
    if (mount("proc", mnt, "proc", 0, NULL) !=0 ) {
        perror("proc");
    }
    sprintf(mnt, "%s/sys", rootfs);
    if (mount("sysfs", mnt, "sysfs", 0, NULL)!=0) {
        perror("sys");
    }
    sprintf(mnt, "%s/tmp", rootfs);
    if (mount("none", mnt, "tmpfs", 0, NULL)!=0) {
        perror("tmp");
    }
    sprintf(mnt, "%s/dev", rootfs);
    if (mount("udev", mnt, "devtmpfs", 0, NULL)!=0) {
        perror("dev");
    }
    sprintf(mnt, "%s/dev/pts", rootfs);
    if (mount("devpts", mnt, "devpts", 0, NULL)!=0) {
        perror("dev/pts");
    }
    sprintf(mnt, "%s/dev/shm", rootfs);
    if (mount("shm", mnt, "tmpfs", 0, NULL)!=0) {
        perror("dev/shm");
    }
    sprintf(mnt, "%s/run", rootfs);
    if (mount("tmpfs", mnt, "tmpfs", 0, NULL)!=0) {
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
    printf("Something's wrong!\n");
    return 1;
}
int main()
{
    printf("Parent [%5d] - start a container!\n", getpid());
    int container_pid = clone(container_main, container_stack+STACK_SIZE, 
            CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, NULL);
    waitpid(container_pid, NULL, 0);
    printf("Parent - container stopped!\n");
    return 0;
}