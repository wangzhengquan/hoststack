#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <sys/syscall.h>

#include "container_fs.h"
#include "container_dao.h"
#include "container_service.h"
#include "container_info.h"
#include "container_rm_cli.h"

struct container_rm_arg_t
{
	bool force;
}; 


static void removeContainer(container_rm_arg_t &mopt, const char * containerId) ;

void ContainerRMCli::usage()
{
  fprintf(stderr, "Usage:	kucker container rm [OPTIONS] CONTAINER [CONTAINER...]\n\n");
  fprintf(stderr, "Remove one or more containers\n\n");
  fprintf(stderr, "Options:\n\n");
  #define fpe(str) fprintf(stderr, "  %s", str);
  fpe("-f, --force     Force the removal of a running container (uses SIGKILL)\n");
  fpe("\n");
}


void ContainerRMCli::handleCommand(int argc, char *argv[]) {
	char **container_arr;
  int container_arr_len;
  if (argc < 2) {
    usage();
    return;
  }

  if(argc == 2 && strcmp(argv[1], "--help") == 0) {
    usage();
    return;
  }

  container_rm_arg_t mopt = {};
  mopt.force = false;
  opterr = 0;

  static struct option long_options[] =
  {
    /* These options set a flag. */
    {"force", no_argument,      0, 'f'},
    {0, 0, 0, 0}
  };
  /* getopt_long stores the option index here. */
  int option_index = 0;
  char c;
  while (1)
  {
    
    c = getopt_long (argc, argv, "+f", long_options, &option_index);

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
      printf ("\n");
      break;

    case 'f':
      mopt.force = true;
      break;
    case '?':
      usage();
      exit(1);
      //printf ("==? optopt=%c, optarg=%s, argv[optind]=%s, optind=%d\n", optopt, optarg, argv[optind], optind);
      /* getopt_long already printed an error message. */
      break;

    default:
      //printf ("==default optopt=%c, %s, `%s'\n",optopt, optarg,  argv[optind]);
      break;
      //abort ();
    }
  }


   /* Print any remaining command line arguments (not options). */
  if (optind < argc)
  {
    container_arr = &argv[optind];
    container_arr_len = argc - optind;
    // printf ("non-option ARGV-elements: ");
    // while (optind < argc)
    //   printf ("%d, %d, %s \n", optind, argc, argv[optind++]);
  }
  else
  {
    usage();
    exit(1);
  }

  for(int i = 0; i < container_arr_len; i++) {
   
    removeContainer(mopt, container_arr[i]);
  }
  
}

static void removeContainer(container_rm_arg_t &mopt, const char * containerName) {
	ContainerInfo info = ContainerDao::get_container_by_id_or_name(containerName);
	if(info.id.empty()) {
		err_msg(0, "No container named %s", containerName);
		return;
	}
	if(info.status == CONTAINER_RUNNING) {
		if(mopt.force) {
		 ContainerService::stop(containerName);
		} else {
			err_msg(0, "Container %s is running, can't be removed. Use option '-f' to force remove container", info.getName().c_str());
			return;
		}
	}

	//ContainerManager::umount_container(info.id);
	ContainerFs::remove_container(info.id.c_str());
  ContainerDao::delete_by_id(info.id.c_str());
}
