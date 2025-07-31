#include <getopt.h>
#include <sys/syscall.h>
#include "logger_factory.h"

#include "container_dao.h"
#include "container_info.h"
#include "container_start_cli.h"
#include "container_service.h"


void ContainerStartCli::usage()
{
  fprintf(stderr, "Usage: hoststack container start [OPTIONS] CONTAINER\n\n");
  fprintf(stderr, "Start a stopped container\n\n");
  fprintf(stderr, "Options:\n\n");
  #define fpe(str) fprintf(stderr, "  %s", str);
  fpe("-d, --detach                         Start container in background and print container ID\n");
  fpe("\n");
}


void ContainerStartCli::handleCommand (int argc,  char *argv[])
{
  //LoggerFactory::getRunLogger().debug("Parent [%5d] - start a container!\n", getpid());
  int c;

  if(argc == 2 && strcmp(argv[1], "--help") == 0) {
    usage();
    return;
  }

  container_start_arg_t mopt = {};
  mopt.detach = false;

  char **container_arr;
  int container_arr_len;

  opterr = 0;

  static struct option long_options[] =
  {
    /* These options set a flag. */
    {"detach", no_argument,      0, 'd'},
    {0, 0, 0, 0}
  };
  /* getopt_long stores the option index here. */
  int option_index = 0;
  while (1)
  {


    c = getopt_long (argc, argv, "+d", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c)
    {
    case 0:
      /* If this option set a flag, do nothing else now. */
      if (long_options[option_index].flag != 0)
        break;
      printf ("option %s", long_options[option_index].name);
      if (optarg)
        printf (" with arg %s", optarg);
      break;

    case 'd':
      mopt.detach = true;

      break;
    case '?':
      //printf ("==? optopt=%c, %s, `%s', %d\n", optopt, optarg, argv[optind], optind);
      /* getopt_long already printed an error message. */
      usage();
      exit(1);
      break;

    default:
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


  /* Retrieve the attributes of terminal on which we are started */
  struct termios ttyOrig;
  struct winsize ws;
  if (tcgetattr(STDIN_FILENO, &ttyOrig) == -1)
    err_msg(errno, "tcgetattr");
  if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0)
    err_msg(errno, "ioctl-TIOCGWINSZ");

  startContainer(mopt,  &ttyOrig, &ws);


}



void ContainerStartCli::startContainer(container_start_arg_t & mopt, struct termios *ttyAttr,  struct winsize *ttyWs)
{
  auto info = ContainerDao::get_container_by_id_or_name(mopt.containerName);
  if(!info) {
    fprintf(stderr, "No container named %s \n", mopt.containerName);
    return;
  }
  if(info->status == CONTAINER_RUNNING) {
    fprintf(stderr, "It has been started already\n");
    return;
  }
  container_start_option_t startOpt = {};
  startOpt.containerId = info->id.c_str();
  str_split(info->command.c_str(), BLANK, &(startOpt.cmd) );
  startOpt.detach = mopt.detach;
  startOpt.volume_list = &info->volume_list;
  startOpt.ttyAttr = ttyAttr;
  startOpt.ttyWs = ttyWs;

  
  ContainerService::start(startOpt, [&](int pid){
    info->pid = pid;
    info->start_time = time(0);
    info->status = CONTAINER_RUNNING;
    info->abnormal_stoped = 1;
    ContainerDao::update(*info);
  });
}

 

