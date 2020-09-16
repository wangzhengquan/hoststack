#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <sys/syscall.h>
#include "logger_factory.h"
#include "sem_util.h"

#include "container_manager.h"
#include "container.h"
#include "container_start_cli.h"
#include "container_service.h"



// static int containerPid;
// static char * containerId;


struct container_start_arg_t
{
  bool interactive;
  char * containerName; // container identify
  // char **container_arr;
  // int container_arr_len;
} ;


static void startContainer(container_start_arg_t & mopt, char * containerName)  ;

// static void sighup_handler(int sig) {
//   LoggerFactory::getDebugLogger().debug("container_start_cli.sighup_handler containerPid=%d", containerPid);
  
//   if(kill(containerPid, SIGHUP) != 0) {
//    // err_msg(errno, "container_start_cli.sighup_handler");
//     LoggerFactory::getDebugLogger().debug("container_start_cli.sighup_handler kill %s", strerror(errno));
//   }
// }

 


void ContainerStartCli::usage()
{
  printf("usage: param error\n");
}




void ContainerStartCli::handle_command (int argc, char *argv[])
{
  printf("Parent [%5d] - start a container!\n", getpid());
  int c;

  container_start_arg_t mopt = {};
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

  // mopt.containerName = ;
  startContainer(mopt, container_arr[0]);


}



static void startContainer(container_start_arg_t & mopt, char * containerName)
{

  
  Container info = ContainerManager::get_container_by_id_or_name(containerName);
  container_start_option_t startOpt = {};
  startOpt.containerId = info.id.c_str();
  startOpt.cmd = str_split(const_cast<char*>(info.command.c_str()), BLANK, 0);
  startOpt.detach = !mopt.interactive;
  if(!info.volume.empty()) {
    startOpt.volume = info.volume.c_str();
  }
  
  ContainerService::start(startOpt, [&](int pid){
    info.pid = pid;
    info.start_time = time(0);
    info.status = CONTAINER_RUNNING;
    ContainerManager::update(info);
  });
  // -------save container info to json file
}


// static int container_run_main(void* arg)
// {

//   printf("in container...\n ");

//   // Container & info = *((Container *)arg);

//   container_start_arg_t mopt = *((container_start_arg_t *)arg);
//   Container info = ContainerManager::get_container_by_id_or_name(mopt.containerName);

//   ContainerManager::mount_container(info.id.c_str());
// // 容器卷
//   if ( !info.volume.empty() )
//   {
//     ContainerManager::mount_volume(info.id.c_str(),  const_cast<char*>(info.volume.c_str()) );
//   }


//   char rootfs[1024];
//   char logfile[1024];

//   pty_exe_opt_t ptyopt = {};
//   ptyopt.synchSem = mopt.synchSem;
//   ptyopt.containerId = info.id.c_str();
//   ptyopt.containerName = mopt.containerName;
//   sprintf(rootfs, "%s/aufs/mnt/%s", kucker_repo, info.id.c_str());
//   ptyopt.rootfs = rootfs;
//   ptyopt.cmd = str_split(const_cast<char*>(info.command.c_str()), BLANK, 0);
//   ptyopt.detach = !mopt.interactive;
//   // stdout 重定向日志文件
//   sprintf(logfile, "%s/containers/%s/stdout.%ld.log", kucker_repo, info.id.c_str(), time(0));
//   ptyopt.logfile = logfile;
//   printf("logfile = %s\n", ptyopt.logfile);
  
//   pty_proxy_exec(ptyopt);
//  // pty_exec(ptyopt);

//   return 1;
// }

