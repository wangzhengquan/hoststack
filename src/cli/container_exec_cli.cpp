#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <sys/syscall.h>
#include "container_dao.h"
#include "container_info.h"
#include "container_exec_cli.h"
#include "pty_exec_util.h"
#include "path_assembler.h"

struct container_exec_arg_t {
  bool detach;
  char ** cmd_arr;
  int cmd_arr_len;
} ;


void exec_cmd(pty_exe_opt_t arg);

void ContainerExecCli::usage()
{
  fprintf(stderr, "Usage: kucker exec [OPTIONS] CONTAINER COMMAND [ARG...]\n\n");
  fprintf(stderr, "Run a command in a running container.\n\n");
  fprintf(stderr, "Options:\n\n");
  #define fpe(str) fprintf(stderr, "  %s", str);
  fpe("-d, --detach               Detached mode: run command in the background\n");
  fpe("\n");
  
}


void ContainerExecCli::handleCommand(int argc, char *argv[]) {
  char *container_id;
  if(argc == 2 && strcmp(argv[1], "--help") == 0) {
    usage();
    return;
  }

  if (argc < 3) {
    printf("Miss command argment\n");
    usage();
    return;
  }

  
  container_exec_arg_t mopt = {};
  mopt.detach = false;
  opterr = 0;

  static struct option long_options[] =
  {
    /* These options set a flag. */
    {"detach", no_argument,      0, 'd'},
    {0, 0, 0, 0}
  };
  /* getopt_long stores the option index here. */
  int option_index = 0;
  char c;
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

  container_id = argv[optind];
  optind++;
  // printf ("optind = %d, argc=%d \n", optind, argc);
  /* Print any remaining command line arguments (not options). */
  if (optind < argc)
  {
    mopt.cmd_arr = &argv[optind];
    mopt.cmd_arr_len = argc - optind;
    // printf ("non-option ARGV-elements: ");
    // while (optind < argc) {
    // 	printf ("%d, %d, %s \n", optind, argc, argv[optind]);
    // 	optind++;
    // }
     
  } else {
     printf("Miss command argment\n");
     usage();
     return;
  }


  char nspath[1024];
  char *namespaces[] = {"pid", "mnt", NULL};
  int i = 0, fd;

  ContainerInfo container = ContainerDao::get_container_by_id_or_name(container_id);
  if(container.id.empty() || container.status != CONTAINER_RUNNING) {
    err_msg(0, "ContainerExecCli >> No container identify by %s, or it's not a container in running.", container_id);
    return;
  }
  if(mopt.detach) {
    if(daemon(0, 1) != 0) {
      err_exit(errno, "conatiner_exec_cli >> daemon");
    }
  }
  while(namespaces[i] != NULL) {
     sprintf(nspath, "/proc/%d/ns/%s",  container.pid, namespaces[i]);
     if( (fd = open(nspath, O_RDONLY)) == -1 ) {
        err_exit(errno, "ContainerExecCli >> open %s", nspath);
     }
     if(setns(fd, 0) == -1) {
        err_exit(errno, "ContainerExecCli >> setns:");
     }
     close(fd);
     i++;
  }
  pty_exe_opt_t ptyopt = {};
  ptyopt.containerId = container.id.c_str();
  ptyopt.cmd = mopt.cmd_arr;
  ptyopt.detach = mopt.detach;

 
  pty_exec(ptyopt);
	//exec_cmd(ptyopt);
}


void exec_cmd(pty_exe_opt_t arg) {
  const char *rootfs = PathAssembler::getMergedDir(arg.containerId, NULL);
  /* chroot 隔离目录 */
  if ( chdir(rootfs) != 0 || chroot("./") != 0 )
  {
    err_exit(errno, "chdir/chroot:%s", rootfs);
  }


  if (system("touch /usr/lib/tmp") != 0)
  {
    err_msg(errno, "touch /usr/lib/tmp");
  }

  execvp(arg.cmd[0], arg.cmd);
  err_msg(errno, "execvp: %s\n", arg.cmd[0]);
}
