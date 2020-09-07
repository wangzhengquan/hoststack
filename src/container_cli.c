#include "container_cli.h"
#include "container_manager.h"
#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <sys/syscall.h>
/* 定义一个给 clone 用的栈，栈大小1M */
#define STACK_SIZE (1024 * 1024)
static char container_stack[STACK_SIZE];

typedef struct run_option_t {
  bool interactive;
  bool detach;
  char *volume;
  char *name;
  char ** cmd_arr;
  int cmd_arr_len;
  char *container_id;
} run_option_t;


int container_main(void* arg)
{
  // char *rootfs;
  // size_t rootfs_len;
  // char *cmd;
  // size_t cmd_len;
  printf("in container...\n ");
  run_option_t * mopt = (run_option_t *)arg;
  // close(pipefd[1]);
  
  // FILE * rf = fdopen(pipefd[0], "r");

  // char *line = NULL;
  //  size_t len = 0;
  //  ssize_t read;

  //  while ((read = getline(&line, &len, rf)) != -1) {
  //    printf("Retrieved line of length %zu :\n", read);
  //    printf("%s", line);
  //  }

  

  // if(getline(&rootfs, &rootfs_len, rf) == -1) {
  //   err_exit(errno, "container_main rootfs");
  // }

  // if(rootfs[strlen(rootfs) - 1] == '\n') {
  //   rootfs[strlen(rootfs) - 1] = '\0';
  // }

  // if (getline(&cmd, &cmd_len, rf) == -1) {
  //   err_exit(errno, "container_main cmd");
  // }
  // fclose(rf);
  // close(pipefd[0]);

  char line[4096];
  char rootfs[1024];
  sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, mopt->container_id);

  ContainerManager::mount_container(rootfs);
 
  
  // char **cmd_arr;
  // size_t cmd_arr_len = str_split(cmd, " ", &cmd_arr);
  // if(cmd_arr_len == 0) {
  //   err_exit(0, "container_main: invalid command");
  // }
 




  /* chroot 隔离目录 */
  if ( chdir(rootfs) != 0 || chroot("./") != 0 )
  {
    err_exit(errno,"chdir/chroot:%s", rootfs);
  }
  // mpivot_root(rootfs);

   //sprintf(line, "%s/containers/%s/out.log", kucker_repo, mopt->container_id);
  //printf("log:%s\n", line);
  // if(mopt->detach) {
  //   // int outfd = open("out.log", O_RDWR | O_CREAT,
  //   //             S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
  //   //             S_IROTH | S_IWOTH);
  //   int outfd = open("/dev/tty", O_RDWR, 0);
  //   if(outfd == -1) {
  //     err_exit(errno, "container_main open:%s", line);
  //   }
  //   dup2(outfd, 1);
  //   //dup2(outfd, 2);
  //   int infd = open("/dev/tty", O_RDONLY, 0);
  //   dup2(infd, 0);
  // }


  system("touch /usr/lib/tmp");

  printf("==============2 %s\n",  mopt->cmd_arr[0]);
  execvp(mopt->cmd_arr[0], mopt->cmd_arr);
  perror("exec");

  return 1;
}




void ContainerCli::exe_run_commond (int argc, char *argv[])
{
  printf("Parent [%5d] - start a container!\n", getpid());
  int c;

  // bool interactive = false;
  // bool detach = false;
  // char *volume = NULL;
  // char *name = NULL;

  // char ** cmd_arr;
  run_option_t mopt = {};
  char * default_cmd_arr[] =
  {
    "/bin/bash",
    "-l",
    NULL
  };
  mopt.cmd_arr = default_cmd_arr;
  mopt.cmd_arr_len = 2;

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

    c = getopt_long (argc, argv, "+idv:", long_options, &option_index);

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
      mopt.interactive = true;
      break;
    case 'd':
      // puts ("==interactive \n");
      mopt.detach = true;
      break;

    case 'v':
      // printf ("==volume with value `%s'\n", optarg);
      mopt.volume = (optarg);
      break;

    case 'n':
      // printf ("==name with value `%s'\n", optarg);
      mopt.name = (optarg);
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
    mopt.cmd_arr = &argv[optind];
    mopt.cmd_arr_len = argc - optind;
    // printf ("non-option ARGV-elements: ");
    // while (optind < argc)
    //   printf ("%d, %d, %s \n", optind, argc, argv[optind++]);
    // putchar ('\n');
  }
  char container_id[37];
  ContainerManager::gen_id(container_id);
  ContainerManager::create_container(container_id);

// 容器卷
  if ( mopt.volume != NULL)
  {
    ContainerManager::mount_volume(container_id,  mopt.volume);
  }
  mopt.container_id = container_id;
  
   
  //pipe(pipefd);
  int container_pid = clone(container_main, container_stack + STACK_SIZE,
                               CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, &mopt);

  // pipe向容器进程发送参数
  // close(pipefd[0]);
  // char rootfs[1024];
  // sprintf(rootfs, "%s/aufs/mnt/%s\n", kucker_repo, container_id);
  // if(write(pipefd[1], rootfs, strlen(rootfs)) < 0) {
  //   perror("exe_run_commond write rootfs");
  // }

  // char *command_line = str_join2(cmd_arr, cmd_arr_len, " ");
  // if(write(pipefd[1], command_line, strlen(command_line)) < 0) {
  //   perror("exe_run_commond write command_line");
  // }
  // printf("write command_line : %s\n", command_line);
  // free(command_line);
  // close(pipefd[1]);

  
  container_info_t container_info = {};
  container_info.id = mopt.container_id;
  container_info.pid = container_pid;
  if(mopt.name != NULL) {
    container_info.name = mopt.name;
  }
  
  char *cmd_str = str_join2(mopt.cmd_arr,  mopt.cmd_arr_len, " ");
  container_info.command = cmd_str;
  // std::cout << "container_info.command ===" << container_info.command << std::endl;
  free(cmd_str);
  container_info.create_time = time(0);
  container_info.status = CONTAINER_RUNNING;
  ContainerManager::save(container_info);

  printf("Parent pid [%5d] - Container pid[%5d]!\n", getpid(), container_pid);


  if(!mopt.detach) {
    waitpid(container_pid, NULL, 0);
    printf("Parent - container stopped!\n");
  } else {
    printf("%s\n", container_id);
  }
  
}


void ContainerCli::exe_ps_commond(int argc, char *argv[]) {
  std::vector<container_info_t>* vector = ContainerManager::list();

  std::cout << "ID\tNAME\tPID\tSTATUS\tCOMMAND\tCREATED" << std::endl;
  if(vector == NULL) {
    return;
  }
  int i = 0;
  for(container_info_t & info : *vector) {
   std::cout << info;
    // printf("%d\n", i++);
  }

  delete vector;
}




void mpivot_root(const char *rootfs) {
  if (mount(rootfs, rootfs, "bind", MS_BIND | MS_REC, NULL)!=0) {
    err_exit(errno, "mpivot_toot mount: %s.", rootfs);
  }
  const char *pivot_dir = path_join(2, rootfs, ".pivot_root");
  if (mkdir(pivot_dir, DIR_MODE) == -1) {
    err_exit(errno, "mpivot_toot mkdir: %s", pivot_dir);
  }

  // if(unshare(CLONE_NEWNS) == -1) {
  //    err_exit(errno, "mpivot_toot unshare");
  // }
  system("unshare -m");

  if(syscall(SYS_pivot_root, rootfs, pivot_dir) == -1) {
     err_exit(errno, "mpivot_toot pivot_root: %s, %s", rootfs, pivot_dir);
  }

  if ( chroot(rootfs) != 0)
  {
    err_exit(errno, "mpivot_toot  chroot: %s", rootfs);
  }

  if ( chdir("/") != 0  )
  {
    err_exit(errno, "mpivot_toot  chdir: %s", rootfs);
  }

  if(umount2("/.pivot_root", MNT_DETACH) == -1) {
     err_exit(errno, "mpivot_toot  umount2");
  }

  if(remove("/.pivot_root") == -1) {
    err_msg(errno, "mpivot_toot  remove");
  }

}

