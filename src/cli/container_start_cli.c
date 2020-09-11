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

/* 定义一个给 clone 用的栈，栈大小1M */
#define STACK_SIZE (1024 * 1024)


#define BUF_SIZE 4096
static struct termios ttyOrig;

static void             /* Reset terminal mode on program exit */
ttyReset(void)
{
  if (tcsetattr(STDIN_FILENO, TCSANOW, &ttyOrig) == -1)
    err_exit(errno, "tcsetattr");
}


struct container_strat_option_t
{
  bool interactive;
  bool detach;
  // char **container_arr;
  // int container_arr_len;
} ;

static void start_container(container_strat_option_t & mopt, char *container_id)  ;

void ContainerStartCli::usage()
{
  printf("usage: param error\n");
}

static int container_run_main(void* arg)
{

  printf("in container...\n ");
  char rootfs[1024];


  Container & info = *((Container *)arg);

  ContainerManager::mount_container(info.id.c_str());
// 容器卷
  if ( !info.volume.empty() )
  {
    ContainerManager::mount_volume(info.id.c_str(),  const_cast<char*>(info.volume.c_str()) );
  }

  sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, info.id.c_str());
  /* chroot 隔离目录 */
  if ( chdir(rootfs) != 0 || chroot("./") != 0 )
  {
    err_exit(errno, "chdir/chroot:%s", rootfs);
  }


  if (system("touch /usr/lib/tmp") != 0)
  {
    err_exit(errno, "touch /usr/lib/tmp");
  }
  int cmd_arr_len;
  char **cmd_arr = str_split(const_cast<char*>(info.command.c_str()), BLANK, &cmd_arr_len);
  execvp(cmd_arr[0], cmd_arr);
  perror(" ContainerStartCli execvp");

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
  start_container(mopt, container_arr[0]);


}


static void start_container(container_strat_option_t & mopt, char *container_id) {
    Container info = ContainerManager::get_container_by_id_or_name(container_id);
    if(!mopt.interactive) {
      err_msg(0, "%s\n", info.id.c_str());
    }
    // if (info.id.empty() || info.status == CONTAINER_RUNNING)
    //   continue;
    /* Create the child in new namespace(s) */
    void *stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED)
      err_exit(0, "handle_run_command mmap:");

    // int container_pid = clone(container_run_main, (char *)stack + STACK_SIZE,
    //                           CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, &info);
    int masterFd;
    struct winsize ws;

    if (tcgetattr(STDIN_FILENO, &ttyOrig) == -1)
      err_exit(errno, "tcgetattr");
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0)
      err_exit(errno, "ioctl-TIOCGWINSZ");

    int container_pid = ptyClone(container_run_main, (char *)stack + STACK_SIZE, 
                            CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, &info, &masterFd, &ttyOrig, &ws);
 

    // printf("%s\n", info.id.c_str());
    printf("Parent pid [%5d] - Container pid[%5d]!\n", getpid(), container_pid);
    

    // -------save container info to json file
    info.pid = container_pid;
    info.start_time = time(0);
    info.status = CONTAINER_RUNNING;
    ContainerManager::update(info);
    // ------save end---------
    
    // =================
    int pid =  fork();
    if(pid == -1) {
       ContainerManager::save_to_stop(info);
       err_exit(errno, "fork");
    }
    else if(pid == 0) {
      //server
      //  if(mopt.interactive) {
      //   int status;
      //   waitpid(container_pid, &status, 0);
      //   if (WIFEXITED(status))
      //   {
      //     err_msg(0, "===WIFEXITED\n");
         

      //   }
      //   else if (WIFSIGNALED(status))
      //   {
      //     err_msg(0, "====SIGCHLD\n");
      //   }
      //   ContainerManager::save_to_stop(info);
      // } else {
      //   printf("Parent - container stopped!\n");
      // }

      

    } else if(pid > 0){
      if(!mopt.interactive) {
        return;
      }
      // client
      fd_set inFds;
      char buf[BUF_SIZE];
      ssize_t numRead;
      char logfile[1024];
      int logfd;
      int infd;
      if(!mopt.interactive) {
       
        sprintf(logfile, "%s/containers/%s/stdout.%ld.log",kucker_repo, container_id, time(0));
        logfd = open(logfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if (logfd == -1)
          err_exit(errno, "open typescript");
        dup2(logfd, STDOUT_FILENO);
        dup2(logfd, STDERR_FILENO);
        close(logfd);
        // infd = open("/dev/null", O_RDONLY);
        // dup2(infd, STDIN_FILENO);
        // printf("log:%s\n", logfile);

      } else {
         /* Place terminal in raw mode so that we can pass all terminal
         input to the pseudoterminal master untouched */

        ttySetRaw(STDIN_FILENO, &ttyOrig);

        if (atexit(ttyReset) != 0)
          err_exit(errno, "atexit");
      }
     

      /* Loop monitoring terminal and pty master for input. If the
         terminal is ready for input, then read some bytes and write
         them to the pty master. If the pty master is ready for input,
         then read some bytes and write them to the terminal. */

      for (;;)
      {
        FD_ZERO(&inFds);
        FD_SET(STDIN_FILENO, &inFds);
        FD_SET(masterFd, &inFds);

        if (select(masterFd + 1, &inFds, NULL, NULL, NULL) == -1)
          err_exit(errno, "select");


        if (FD_ISSET(STDIN_FILENO, &inFds))     /* stdin --> pty */
        {
           
          numRead = read(STDIN_FILENO, buf, BUF_SIZE);
          if (numRead <= 0) {
            // exit(EXIT_SUCCESS);
            break;
          }

          if (write(masterFd, buf, numRead) != numRead)
            err_exit(errno, "partial/failed write (masterFd)");
        }

        if (FD_ISSET(masterFd, &inFds))        /* pty --> stdout+file */
        {
          numRead = read(masterFd, buf, BUF_SIZE);
          if (numRead <= 0) {
            // exit(EXIT_SUCCESS);
            break;
          }

          if (write(STDOUT_FILENO, buf, numRead) != numRead)
            err_exit(errno, "partial/failed write (STDOUT_FILENO)");


        }
      }
    }

   

    //ttyReset();
    //==========================
   
   
    
   
    
}

