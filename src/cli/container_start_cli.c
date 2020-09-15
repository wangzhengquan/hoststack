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
#include "logger_factory.h"

/* 定义一个给 clone 用的栈，栈大小1M */
#define STACK_SIZE (1024 * 1024)

static bool waitfg;
static bool conatinerExit;
static int container_pid;

struct container_start_option_t
{
  bool interactive;
  char * containerName; // container identify
  // char **container_arr;
  // int container_arr_len;
} ;

static void startContainer(container_start_option_t & mopt)  ;


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
      LoggerFactory::getDebugLogger().debug("%d container_start_cli.sigchld_handler exit", pid);
      waitfg = false;
      conatinerExit = true;
     
      /*if (verbose)*/
      /*printf("Job [%d] (%d)  normally exit by signal %d\t%s\n", pid2jid(pid), pid, WEXITSTATUS(status), getjobpid(jobs, pid)->cmdline);*/
     // deletejob(jobs, pid);
    }
    else if (WIFSIGNALED(status))
    {
      LoggerFactory::getDebugLogger().debug("%d container_start_cli.sigchld_handler terminated by signal %s", 
        pid, 
        strsignal(WTERMSIG(status)));
      waitfg = false;
      conatinerExit = true;
     // deletejob(jobs, pid);
    }
    else if (WIFSTOPPED(status))
    {
      LoggerFactory::getDebugLogger().debug("(%d) stopted by signal %d\n", pid, WSTOPSIG(status));
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
  LoggerFactory::getDebugLogger().debug("container_start_cli.sighup_handler container_pid=%d", container_pid);
  
  if(kill(container_pid, SIGHUP) != 0) {
   // err_msg(errno, "container_start_cli.sighup_handler");
    LoggerFactory::getDebugLogger().debug("container_start_cli.sighup_handler kill %s", strerror(errno));
  }
  // sleep(2);
  waitfg = false;
}

 


void ContainerStartCli::usage()
{
  printf("usage: param error\n");
}

static int container_run_main(void* arg)
{

  printf("in container...\n ");

  // Container & info = *((Container *)arg);

  container_start_option_t mopt = *((container_start_option_t *)arg);
  Container info = ContainerManager::get_container_by_id_or_name(mopt.containerName);

  ContainerManager::mount_container(info.id.c_str());
// 容器卷
  if ( !info.volume.empty() )
  {
    ContainerManager::mount_volume(info.id.c_str(),  const_cast<char*>(info.volume.c_str()) );
  }


  char rootfs[1024];
  char logfile[1024];

  pty_exe_opt_t ptyopt = {};
  ptyopt.containerId = info.id.c_str();
  ptyopt.containerName = mopt.containerName;
  sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, info.id.c_str());
  ptyopt.rootfs = rootfs;
  ptyopt.cmd = str_split(const_cast<char*>(info.command.c_str()), BLANK, 0);
  ptyopt.detach = !mopt.interactive;
  // stdout 重定向日志文件
  sprintf(logfile, "%s/containers/%s/stdout.%ld.log", kucker_repo, info.id.c_str(), time(0));
  ptyopt.logfile = logfile;
  printf("logfile = %s\n", ptyopt.logfile);
  
  pty_proxy_exec(ptyopt);
 // pty_exec(ptyopt);

  return 1;
}




void ContainerStartCli::handle_command (int argc, char *argv[])
{
  printf("Parent [%5d] - start a container!\n", getpid());
  int c;

  container_start_option_t mopt = {};
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
  //   startContainer(mopt.container_arr[i]);
  // }

  mopt.containerName = container_arr[0];
  startContainer(mopt);


}


static void startContainer(container_start_option_t & mopt)
{
  waitfg = mopt.interactive;
  conatinerExit = false;
  

  Container info = ContainerManager::get_container_by_id_or_name(mopt.containerName);
  if(info.id.empty()) {
    err_exit(0, "there is no container identify by %s", mopt.containerName);
  }
  // if (info.id.empty() || info.status == CONTAINER_RUNNING)
  //   continue;
  /* Create the child in new namespace(s) */
  void *stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
  if (stack == MAP_FAILED)
    err_exit(0, "handle_run_command mmap:");

  container_pid = clone(container_run_main, (char *)stack + STACK_SIZE,
                            CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, &mopt);

  Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */
  Signal(SIGHUP, sighup_handler);

  // printf("%s\n", info.id.c_str());
  printf("Parent pid [%5d] - Container pid[%5d]!\n", getpid(), container_pid);
  // -------save container info to json file
 
  info.pid = container_pid;
  info.start_time = time(0);
  info.status = CONTAINER_RUNNING;
  ContainerManager::update(info);
  // ------save end---------
 
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
    printf("exit\n");
  } else {
    printf("containerName=%s\n", info.id.c_str());
  }


 
  // if (mopt.interactive)
  // {
  //   int status;
  //   waitpid(container_pid, &status, 0);
  //   if (WIFEXITED(status))
  //   {
  //     printf("===WIFEXITED\n");
  //   }
  //   else if (WIFSIGNALED(status))
  //   {
  //     printf("====SIGCHLD\n");
  //   }
  //   ContainerManager::umount_container(info.id);
  //   ContainerManager::change_status_to_stop(info.id);

  //   printf("Parent - container stopped!\n");
  // }
  // else
  // {
  //   printf("containerName=%s\n", info.id.c_str());
  // }
}
