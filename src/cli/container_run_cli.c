#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <sys/syscall.h>

#include "container_manager.h"
#include "container.h"
#include "container_run_cli.h"
#include "pty_exec_util.h"
#include "container_service.h"
 

struct container_run_arg_t {
  bool detach;
  std::set<std::string> volume_list;
  // char *volume_list[16];
  // int volume_list_size;
  char *name;
  char ** cmd_arr;
  int cmd_arr_len;
  char *container_id;
} ;

 

static void startContainer(container_run_arg_t mopt);
  

void ContainerRunCli::usage()
{
  fprintf(stderr, "Usage: kucker container run [OPTIONS] [COMMAND] [ARG...]\n\n");
  fprintf(stderr, "Run a command in a new container\n\n");
  fprintf(stderr, "Options:\n\n");
  #define fpe(str) fprintf(stderr, "  %s", str);
  fpe("-d, --detach                         Run container in background and print container ID\n");
  fpe("-v, --volume list                    Bind mount a volume\n");
  fpe("-n, --name string                    Assign a name to the container\n");
  fpe("\n");
}


void ContainerRunCli::handleCommand (int argc, char *argv[])
{
  int c;

  if(argc == 2 && strcmp(argv[1], "--help") == 0) {
    usage();
    return;
  }

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

  container_run_arg_t mopt = {};
  mopt.detach = false;
  mopt.cmd_arr = default_cmd_arr;
  mopt.cmd_arr_len = 2;
  // mopt.volume_list_size = 0;

  opterr = 0;

  static struct option long_options[] =
  {
    /* These options set a flag. */
    {"detach", no_argument,      0, 'd'},
    {"volume",  required_argument, 0, 'v'},
    {"name",  required_argument, 0, 'n'},
    {0, 0, 0, 0}
  };
  /* getopt_long stores the option index here. */
  int option_index = 0;
  while (1)
  {
    

    c = getopt_long (argc, argv, "+dv:n:", long_options, &option_index);

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
     
    case 'd':
      mopt.detach = true;
      break;

    case 'v':
      mopt.volume_list.insert(optarg);
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

  ContainerManager::create_container(container_id);
  startContainer(mopt);
}

static void startContainer(container_run_arg_t mopt) {
  container_start_option_t startOpt = {};
  startOpt.containerId =  mopt.container_id;
  startOpt.cmd = mopt.cmd_arr;
  startOpt.detach = mopt.detach;
  startOpt.volume_list = &mopt.volume_list;
  // if(mopt.volume_list != NULL) {
    
  //   // startOpt.volume_list_size = mopt.volume_list_size;
  // }
  
  ContainerService::start(startOpt, [&](int pid){
    // -------save container info to json file
    Container info = {};
    info.id = mopt.container_id;
    info.pid = pid;
    if(mopt.name != NULL) {
      info.name = mopt.name;
    }
    
    char *cmd_str = array_join(mopt.cmd_arr, " ");
    info.command = cmd_str;
    free(cmd_str);

    info.create_time = time(0);
    info.start_time = time(0);
    info.volume_list = mopt.volume_list;
    // if (mopt.volume != NULL) {
       
    // }
    info.status = CONTAINER_RUNNING;
    ContainerManager::insert(info);
    // ------save end---------
  });

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
