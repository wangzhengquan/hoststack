#include "usg_common.h"
#include <sys/mount.h>
#include <getopt.h>
#include "path_assembler.h"
#include "hoststack_config.h"
#include "container_cli.h"
#include "container_fs.h"
#include "logger_factory.h"
#include "container_dao.h"
#include <sys/stat.h>
#include <fcntl.h>

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "log.h"

static Logger& logger = LoggerFactory::getKuckerDaemonLogger();

// static char hoststack[256];

static struct termios ttyOrig = {};
static struct winsize ws= {};

void usage(const char *name) {
	printf("Usage: %s", name);
}


// void recheck(std::string &id) {
// 	sleep(3);
// 	char line[526];
// 	ContainerInfo info = ContainerDao::get_container_by_id(id);
// 	if(info.status == CONTAINER_RUNNING) {
//     sprintf(line, "/proc/%d", info.pid);
//     if(access(line, F_OK) == -1) {
//     	ContainerDao::change_status_to_stop(info.id);
//     	printf("1 start %s\n", info.name.c_str());
//     	sprintf(line, "sudo %s start -d %s", hoststack, info.name.c_str());
//     	system(line);
//     	printf("2 start %s\n", info.name.c_str());
//     	logger.info("start %s\n\n\n\n", info.name.c_str());
//     }
//   }
// }

void *run_checkandrestart(void *ptr) {
  pthread_detach(pthread_self());
  container_start_arg_t mopt = {};
	char line[526];
	std::string *id = (std::string *)ptr;
	sleep(3);

	auto info = ContainerDao::get_container_by_id(*id);
	if(info->status == CONTAINER_RUNNING) {
    sprintf(line, "/proc/%d", info->pid);
    if(access(line, F_OK) == -1) {
    	ContainerDao::change_status_to_stop(info->id);
    	// printf("1 start %s\n", info->name.c_str());
    	// sprintf(line, "sudo %s start -d %s", hoststack, info->name.c_str());
    	// system(line);
    	mopt.detach = true;
    	mopt.containerName = info->id.c_str();
    	ContainerStartCli::startContainer(mopt,  NULL, &ws);

    	printf("2 start %s\n", info->name.c_str());
    	logger.info("start %s\n\n\n\n", info->name.c_str());
    }
  }
  delete id;
  return 0;
}

int main(int argc, char *argv[])
{

	char line[526];
	pthread_t tid;

	if(geteuid() != 0) {
    printf("权限不够, 请查看您是否正以 root 用户运行\n");
    exit(1);
  }
	// char *hoststackPath = NULL;
	// if(argc == 2 ) {
	// 	// hoststackPath = argv[0];
	// 	sprintf(hoststack, "%s/hoststack",  argv[1]);
	// } else {
	// 	sprintf(hoststack, "hoststack" );
	// }

	

  // if (tcgetattr(STDIN_FILENO, &ttyOrig) == -1)
  //   err_msg(errno, "tcgetattr");
  // if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0)
  //   err_msg(errno, "ioctl-TIOCGWINSZ");

  ws.ws_row = 67;
	ws.ws_col = 204;


	if(daemon(1, 1) != 0) {
    logger.error(errno, "hoststack_supervisor daemon");
  }

  while(true) {
  	std::vector<ContainerInfo>* vector = ContainerDao::list();
  	for(ContainerInfo & info : *vector) {
		  if(info.status == CONTAINER_RUNNING) {
		  	
		    sprintf(line, "/proc/%d", info.pid);
		    if(access(line, F_OK) == -1) {
		     	// recheck(info.id);
		     	pthread_create(&tid, NULL, run_checkandrestart, new std::string(info.id));
		    }
		  }
	    
	  }
	  delete vector;
	  sleep(3);
  }
  
} 
