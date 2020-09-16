#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <sys/syscall.h>

#include "container_manager.h"
#include "container.h"
#include "container_run_cli.h"
#include "pty_exec_util.h"
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


static bool waitfg;
static bool conatinerExit;

static void startContainer(container_run_option_t mopt);
/*****************
 * Signal handlers
 *****************/

/*
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.
 */
static void sigchld_handler(int sig)
{
  int olderrno = errno;
  pid_t pid;
  int status;
  while ( (pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0)
  {
    if (WIFEXITED(status))
    {
      printf("===exit===\n");
      waitfg = false;
      conatinerExit = true;
     
      /*if (verbose)*/
      /*printf("Job [%d] (%d)  normally exit by signal %d\t%s\n", pid2jid(pid), pid, WEXITSTATUS(status), getjobpid(jobs, pid)->cmdline);*/
     // deletejob(jobs, pid);
    }
    else if (WIFSIGNALED(status))
    {
      printf("(%d) terminated by signal %d\n",  pid, WTERMSIG(status));
     // deletejob(jobs, pid);
    }
    else if (WIFSTOPPED(status))
    {
      printf("(%d) stopted by signal %d\n", pid, WSTOPSIG(status));
     // getjobpid(jobs, pid)->state = ST;
    }
    else if (WIFCONTINUED(status))
    {
    }
  }

  if (pid == -1 && errno != ECHILD)
  {
    err_msg(errno, "sigchld_handler error");
  }
  errno = olderrno;
}

static void sighup_handler(int sig) {
  printf("sighup_handler\n");
  waitfg = false;
}


void ContainerRunCli::usage()
{
  printf("usage: param error\n");
}


// void exec() {
//    /* chroot 隔离目录 */
//     if ( chdir(rootfs) != 0 || chroot("./") != 0 )
//     {
//       err_exit(errno, "chdir/chroot:%s", rootfs);
//     }


//     if (system("touch /usr/lib/tmp") != 0)
//     {
//       err_exit(errno, "touch /usr/lib/tmp");
//     }

//     execvp(mopt.cmd_arr[0], mopt.cmd_arr);
         // err_msg(errno, "execvp: %s\n", mopt.cmd_arr[0]);
// }

 

static int container_run_main(void* arg)
{
  
  printf("in container...\n ");
 
 

  container_run_option_t & mopt = * ((container_run_option_t *)arg);
 
  ContainerManager::mount_container(mopt.container_id);
// 容器卷
  if ( mopt.volume != NULL)
  {
    ContainerManager::mount_volume(mopt.container_id,  mopt.volume);
  }


  char rootfs[1024];
  char logfile[1024];
  
  pty_exe_opt_t ptyopt = {};
  ptyopt.containerId = mopt.container_id;
  sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, mopt.container_id);
  ptyopt.rootfs = rootfs;
  ptyopt.cmd = mopt.cmd_arr;
  ptyopt.detach = mopt.detach;
  sprintf(logfile, "%s/containers/%s/stdout.%ld.log",kucker_repo,  mopt.container_id, time(0));
  ptyopt.logfile = logfile;
  printf("logfile = %s\n", ptyopt.logfile);
  

  pty_exec(ptyopt);

  //char serverFifo[1024];
  // sprintf(serverFifo, "%s/containers/%s/server.fifo",kucker_repo,  mopt.container_id);
  // printf("serverFifo = %s\n", serverFifo);
  // char clientFifo[1024];
  // sprintf(clientFifo, "/tmp/client.%s.fifo", time(0));
  // ptyopt.serverFifo= serverFifo;
  // ptyopt.clientFifo= clientFifo;

  // pty_proxy_exec(ptyopt);
  // pty_client(ptyopt);
  

  return 1;
}




void ContainerRunCli::handle_command (int argc, char *argv[])
{
  int c;

  char *shell = getenv("SHELL");
  if (shell == NULL || *shell == '\0')
    shell = "/bin/bash";
  // char ** cmd_arr;
  
  char * default_cmd_arr[] =
  {
    shell,
    "-l",
    NULL
  };

  container_run_option_t mopt = {};
  mopt.detach = false;
  mopt.cmd_arr = default_cmd_arr;
  mopt.cmd_arr_len = 2;

  opterr = 0;

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
  while (1)
  {
    

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
     // printf ("==? optopt=%c, %s, `%s', %d\n", optopt, optarg, argv[optind], optind);
      /* getopt_long already printed an error message. */
      usage();
      exit(1);
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

  startContainer(mopt);
}

static void startContainer(container_run_option_t mopt) {
  waitfg = !mopt.detach;
  conatinerExit = false;




  ContainerManager::create_container(mopt.container_id);
  /* Create the child in new namespace(s) */
  void *stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
  if (stack == MAP_FAILED)
      err_exit(0, "handle_run_command mmap:");

  int container_pid = clone(container_run_main, (char *)stack + STACK_SIZE,
                               CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, &mopt);
 
  
  Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */
  Signal(SIGHUP, sighup_handler);

  printf("Parent pid [%5d] - Container pid[%5d]!\n", getpid(), container_pid);

 
 // -------save container info to json file
  Container info = {};
  info.id = mopt.container_id;
  info.pid = container_pid;
  if(mopt.name != NULL) {
    info.name = mopt.name;
  }
  
  char *cmd_str = array_join(mopt.cmd_arr, " ");
  info.command = cmd_str;
  // std::cout << "info.command ===" << info.command << std::endl;
  free(cmd_str);

  info.create_time = time(0);
  info.start_time = time(0);
 
  if (mopt.volume != NULL) {
     info.volume = mopt.volume;
  }
  info.status = CONTAINER_RUNNING;
  ContainerManager::insert(info);
  // ------save end---------
  // char ch;
  // while(read(STDIN_FILENO, &ch, sizeof(char)) > 0 ) {
  //   //write(STDOUT_FILENO, &ch, sizeof(char));
  //   printf("%d\n", ch);
  // }


  sigset_t mask, prev;
  sigemptyset(&mask);
  sigaddset(&mask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &mask, &prev);
  while (waitfg)
  {
    sigsuspend(&prev);
  }
  sigprocmask(SIG_SETMASK, &prev, NULL);

  if(conatinerExit) {
    ContainerManager::umount_container(info.id);
    ContainerManager::change_status_to_stop(info.id);
  } else {
    printf("containerName=%s\n", info.id.c_str());
  }
 

  // if(!mopt.detach) {
  //   int status;
  //   waitpid(container_pid, &status, 0);
  //   if (WIFEXITED(status))
  //   {
  //     printf("===WIFEXITED\n");
  //     Container info = ContainerManager::get_container_by_id(container_id);
  //     ContainerManager::umount_container(info.id);
  //     ContainerManager::change_status_to_stop(info.id);
    
  //   } else if (WIFSIGNALED(status)) {
  //     printf("====SIGCHLD\n");
  //   }
   
  //   printf("Parent - container stopped!\n");
  // } else {
  //   printf("%s\n", container_id);
  // }
}

// void mpivot_root(const char *rootfs) {
//   if (mount(rootfs, rootfs, "bind", MS_BIND | MS_REC, NULL)!=0) {
//     err_exit(errno, "mpivot_toot mount: %s.", rootfs);
//   }
//   const char *pivot_dir = path_join(rootfs, ".pivot_root", NULL);
//   if (mkdir(pivot_dir, DIR_MODE) == -1) {
//     err_exit(errno, "mpivot_toot mkdir: %s", pivot_dir);
//   }

//   // if(unshare(CLONE_NEWNS) == -1) {
//   //    err_exit(errno, "mpivot_toot unshare");
//   // }
//   //system("unshare -m");

//   if(syscall(SYS_pivot_root, rootfs, pivot_dir) == -1) {
//      err_exit(errno, "mpivot_toot pivot_root: %s, %s", rootfs, pivot_dir);
//   }

//   if ( chroot(rootfs) != 0)
//   {
//     err_exit(errno, "mpivot_toot  chroot: %s", rootfs);
//   }

//   if ( chdir("/") != 0  )
//   {
//     err_exit(errno, "mpivot_toot  chdir: %s", rootfs);
//   }

//   if(umount2("/.pivot_root", MNT_DETACH) == -1) {
//      err_exit(errno, "mpivot_toot  umount2");
//   }

//   if(remove("/.pivot_root") == -1) {
//     err_msg(errno, "mpivot_toot  remove");
//   }

// }
