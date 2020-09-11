#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <sys/syscall.h>
#include "container_manager.h"
#include "container.h"
#include "container_start_cli.h"

#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "pty_master_open.h"
#include "pty_fork.h"                   /* Declares ptyFork() */
#include "usg_common.h"
#include "tty_functions.h"
#include "pty_exec_util.h"

/* 定义一个给 clone 用的栈，栈大小1M */
#define STACK_SIZE (1024 * 1024)

struct container_strat_option_t
{
  bool interactive;
  char * container_id;
  // char **container_arr;
  // int container_arr_len;
} ;

static void start_container(container_strat_option_t mopt)  ;

void ContainerStartCli::usage()
{
  printf("usage: param error\n");
}

static int container_run_main(void* arg)
{

  printf("in container...\n ");

  // Container & info = *((Container *)arg);

  container_strat_option_t mopt = *((container_strat_option_t *)arg);
  Container info = ContainerManager::get_container_by_id_or_name(mopt.container_id);

  ContainerManager::mount_container(info.id.c_str());
// 容器卷
  if ( !info.volume.empty() )
  {
    ContainerManager::mount_volume(info.id.c_str(),  const_cast<char*>(info.volume.c_str()) );
  }


  char rootfs[1024];
  char logfile[1024];

  pty_exe_opt_t ptyopt = {};
  sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, mopt.container_id);
  ptyopt.rootfs = rootfs;
  ptyopt.cmd = str_split(const_cast<char*>(info.command.c_str()), BLANK, 0);
  ptyopt.detach = !mopt.interactive;
  if(ptyopt.detach)
  {
    sprintf(logfile, "%s/containers/%s/stdout.%ld.log", kucker_repo,  mopt.container_id, time(0));
    ptyopt.logfile = logfile;
    // printf("logfile = %s\n", ptyopt.logfile);
  } 
  pty_exec(ptyopt);

  return 1;
}




void ContainerStartCli::handle_command (int argc, char *argv[])
{
  printf("Parent [%5d] - start a container!\n", getpid());
  int c;

  // char ** cmd_arr;
  container_strat_option_t mopt = {};
  mopt.interactive = false;

  char **container_arr;
  int container_arr_len;
  opterr = 0;

  static struct option long_options[] =
  {
    /* These options set a flag. */
    {"interactive", no_argument,      0, 'i'},
    {0, 0, 0, 0}
  };
  /* getopt_long stores the option index here. */
  int option_index = 0;
  while (1)
  {


    c = getopt_long (argc, argv, "+i", long_options, &option_index);

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
    container_arr = &argv[optind];
    container_arr_len = argc - optind;
    // printf ("non-option ARGV-elements: ");
    // while (optind < argc)
    //   printf ("%d, %d, %s \n", optind, argc, argv[optind++]);
    // putchar ('\n');
  }
  else
  {
    usage();
    exit(1);
  }

  // for (int i = 0; i <  mopt.container_arr_len; i++)
  // {
  //   start_container(mopt.container_arr[i]);
  // }

  mopt.container_id = container_arr[0];
  start_container(mopt);


}


static void start_container(container_strat_option_t mopt)
{

  
  // if (info.id.empty() || info.status == CONTAINER_RUNNING)
  //   continue;
  /* Create the child in new namespace(s) */
  void *stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
  if (stack == MAP_FAILED)
    err_exit(0, "handle_run_command mmap:");

  int container_pid = clone(container_run_main, (char *)stack + STACK_SIZE,
                            CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, &mopt);


  // printf("%s\n", info.id.c_str());
  printf("Parent pid [%5d] - Container pid[%5d]!\n", getpid(), container_pid);


  // -------save container info to json file
  Container info = ContainerManager::get_container_by_id_or_name(mopt.container_id);
  info.pid = container_pid;
  info.start_time = time(0);
  info.status = CONTAINER_RUNNING;
  ContainerManager::update(info);
  // ------save end---------

  if (mopt.interactive)
  {
    int status;
    waitpid(container_pid, &status, 0);
    if (WIFEXITED(status))
    {
      printf("===WIFEXITED\n");
    }
    else if (WIFSIGNALED(status))
    {
      printf("====SIGCHLD\n");
    }
    ContainerManager::umount_container(info.id);
    ContainerManager::change_status_to_stop(info);

    printf("Parent - container stopped!\n");
  }
  else
  {
    printf("containerid=%s\n", info.id.c_str());
  }
}
