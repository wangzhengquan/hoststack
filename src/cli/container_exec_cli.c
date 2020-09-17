#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include <sys/syscall.h>
#include "container_manager.h"
#include "container.h"
#include "container_exec_cli.h"
#include "pty_exec_util.h"
#include "path_assembler.h"

struct container_exec_arg_t {
  bool interactive;
  bool detach;
  char *volume;
  char *name;
  char ** cmd_arr;
  int cmd_arr_len;
  char *container_id;
} ;


void exec_cmd(pty_exe_opt_t arg);

void ContainerExecCli::usage()
{
  printf("usage: param error\n");
}


void ContainerExecCli::handleCommand(int argc, char *argv[]) {
  if (argc < 3) {
    usage();
    return;
  }
  
  char *container_id;
  container_exec_arg_t mopt = {};
  opterr = 0;

  static struct option long_options[] =
  {
    /* These options set a flag. */
    {"interactive", no_argument,      0, 'i'},
    {"detach", no_argument,      0, 'd'},
    {0, 0, 0, 0}
  };
  /* getopt_long stores the option index here. */
  int option_index = 0;
  char c;
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
     
  }


  char nspath[1024];
  char *namespaces[] = {"pid", "mnt", NULL};
  int i = 0, fd;

  Container container = ContainerManager::get_container_by_id_or_name(container_id);
  if(container.id.empty() || container.status != CONTAINER_RUNNING) {
    err_msg(0, "ContainerExecCli >> No container identify by %s, or it's not a container in running.", container_id);
    return;
  }
  if(mopt.detach) {
    daemon(0, 1);
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
  const char *rootfs = PathAssembler::getRootFS(arg.containerId, NULL);
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
