#include <sys/mount.h>
#include <getopt.h>
#include <sys/syscall.h>

#include "container_dao.h"
#include "container_fs.h"
#include "container_info.h"
#include "container_run_cli.h"
#include "pty_exec_util.h"
#include "container_service.h"
#include "log.h"




void ContainerRunCli::usage()
{
  fprintf(stderr, "Usage: hoststack container run [OPTIONS] [COMMAND] [ARG...]\n\n");
  fprintf(stderr, "Run a command in a new container\n\n");
  fprintf(stderr, "Options:\n\n");
  #define fpe(str) fprintf(stderr, "  %s", str);
  fpe("-d, --detach                         Run container in background and print container ID\n");
  fpe("-v, --volume list                    Bind mount a volume\n");
  fpe("-n, --name string                    Assign a name to the container\n");
  fpe("\n");
}


void ContainerRunCli::handleCommand (int argc,  char *argv[])
{
  int c;

  if(argc == 2 && strcmp(argv[1], "--help") == 0) {
    usage();
    return;
  }

  const char * shell = getenv("SHELL");
  if (shell == NULL || *shell == '\0')
    shell = "/bin/bash";
  // char ** cmd_arr;
  
  const char *  default_cmd_arr[] =
  {
    shell,
    "-l",
    NULL
  };

  container_run_arg_t mopt = {};
  mopt.detach = false;
  mopt.cmd_arr = const_cast<char **>(default_cmd_arr);
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
  if(mopt.name != NULL && ContainerDao::get_container_by_name(mopt.name)) {
    fprintf(stderr, "Container name '%s' already exists!\n", mopt.name);
    return;
  }
 

 // char container_id[37];
  // ContainerDao::gen_id();
  std::string uuid = ContainerDao::gen_id();
// std::cout << "mopt.container_id=" << uuid << ",  " << mopt.container_id << std::endl;
  mopt.container_id = uuid.c_str();
  ContainerFs::create_container(mopt.container_id);

  struct termios ttyOrig;
  struct winsize ws;
  if (tcgetattr(STDIN_FILENO, &ttyOrig) == -1)
    err_msg(errno, "tcgetattr");
  if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0)
    err_msg(errno, "ioctl-TIOCGWINSZ");

  

  startContainer(mopt, &ttyOrig, &ws);
}

void ContainerRunCli::startContainer( container_run_arg_t &mopt, struct termios *ttyAttr,  struct winsize *ttyWs) {
  container_start_option_t startOpt = {};
  startOpt.containerId =  mopt.container_id;
  startOpt.cmd = mopt.cmd_arr;
  startOpt.detach = mopt.detach;
  startOpt.volume_list = &mopt.volume_list;
  startOpt.ttyAttr = ttyAttr;
  startOpt.ttyWs = ttyWs;
 
  
  ContainerService::start(startOpt, [&](int pid){
    // -------save container info to json file
    ContainerInfo info = {};
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
    info.status = CONTAINER_RUNNING;
    info.abnormal_stoped = 1;
    ContainerDao::insert(info);
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
