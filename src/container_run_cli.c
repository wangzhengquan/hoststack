#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <sys/syscall.h>

#include "container_manager.h"
#include "container.h"
#include "container_run_cli.h"

/* 定义一个给 clone 用的栈，栈大小1M */
#define STACK_SIZE (1024 * 1024)

struct container_run_option_t {
  bool interactive;
  bool detach;
  char *volume;
  char *name;
  char ** cmd_arr;
  int cmd_arr_len;
  char *container_id;
} ;


void ContainerRunCli::usage()
{
  printf("usage: param error\n");
}

int container_run_main(void* arg)
{
  
  printf("in container...\n ");
  char rootfs[1024];
 

  container_run_option_t & mopt = * ((container_run_option_t *)arg);
 
  ContainerManager::mount_container(mopt.container_id);
// 容器卷
  if ( mopt.volume != NULL)
  {
    ContainerManager::mount_volume(mopt.container_id,  mopt.volume);
  }

  sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, mopt.container_id);
  /* chroot 隔离目录 */
  if ( chdir(rootfs) != 0 || chroot("./") != 0 )
  {
    err_exit(errno,"chdir/chroot:%s", rootfs);
  }
   

  if(system("touch /usr/lib/tmp") !=0) {
  	err_exit(errno, "touch /usr/lib/tmp");
  }

  execvp(mopt.cmd_arr[0], mopt.cmd_arr);
  perror("exec");

  return 1;
}




void ContainerRunCli::handle_command (int argc, char *argv[])
{
  printf("Parent [%5d] - start a container!\n", getpid());
  int c;

  // char ** cmd_arr;
  container_run_option_t mopt = {};
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

    c = getopt_long (argc, argv, "+idv:n:", long_options, &option_index);

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

  // check arguments
  // name 去重
  if( mopt.name != NULL) {
  	 if(!ContainerManager::get_container_by_name(mopt.name).id.empty()) {
  	 		err_exit(0, "duplicate name %s", mopt.name);
  	 }
  }
 


  char container_id[37];
  ContainerManager::gen_id(container_id);
  mopt.container_id = container_id;

  ContainerManager::create_container(mopt.container_id);
  /* Create the child in new namespace(s) */
  void *stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
  if (stack == MAP_FAILED)
      err_exit(0, "handle_run_command mmap:");

  int container_pid = clone(container_run_main, (char *)stack + STACK_SIZE,
                               CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, &mopt);
 
  

  printf("Parent pid [%5d] - Container pid[%5d]!\n", getpid(), container_pid);

 
 // -------save container info to json file
  Container container_info = {};
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
 
  if (mopt.volume != NULL) {
  	 container_info.volume = mopt.volume;
  }
  container_info.status = CONTAINER_RUNNING;
  ContainerManager::insert(container_info);
  // ------save end

  if(!mopt.detach) {
    waitpid(container_pid, NULL, 0);
    Container info = ContainerManager::get_container_by_id(container_id);
    info.status = CONTAINER_STOPED;
    info.pid = 0;
    ContainerManager::update(info);
    ContainerManager::umount_container(container_id);
    
    printf("Parent - container stopped!\n");
  } else {
    printf("%s\n", container_id);
  }
  
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
  //system("unshare -m");

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
