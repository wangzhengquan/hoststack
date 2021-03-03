#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <sys/syscall.h>
#include "container_dao.h"
#include "container_info.h"
#include "container_ls_cli.h"

struct container_ls_arg_t {
  bool all;
};
void ContainerLsCli::usage()
{
  fprintf(stderr, "Usage: kucker container ls [OPTIONS]\n\n");
  fprintf(stderr, "List containers\n\n");
  fprintf(stderr, "Options:\n\n");
  #define fpe(str) fprintf(stderr, "  %s", str);
  fpe("-a, --all             Show all containers (default shows just running)\n");
  fpe("\n");
}


void ContainerLsCli::handleCommand(int argc, char *argv[]) {
	char c;
  if(argc == 2 && strcmp(argv[1], "--help") == 0) {
    usage();
    return;
  }
	container_ls_arg_t mopt = {};
  mopt.all = false;
  opterr = 0;
  static struct option long_options[] =
  {
    /* These options set a flag. */
    {"all", no_argument,      0, 'a'},
    {0, 0, 0, 0}
  };
  /* getopt_long stores the option index here. */
  int option_index = 0;

  while (1)
  {
    c = getopt_long (argc, argv, "a", long_options, &option_index);

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

    case 'a':
      // puts ("==interactive \n");
      mopt.all = true;
      break;
    case '?':
      //printf ("==? optopt=%c, %s, `%s', %d\n", optopt, optarg, argv[optind], optind);
      /* getopt_long already printed an error message. */
    	usage();
    	exit(1);
      break;

    default:
    	usage();
    	exit(1);
      //printf ("==default optopt=%c, %s, `%s'\n",optopt, optarg,  argv[optind]);
      break;
      //abort ();
    }
  }


  std::vector<ContainerInfo>* vector = ContainerDao::list();

  ContainerInfo::showTitle();
  if(vector == NULL) {
    return;
  }
  for(ContainerInfo & info : *vector) {
    if(mopt.all || info.status == CONTAINER_RUNNING)
      info.show();
    // printf("%d\n", i++);
  }

  delete vector;
}
