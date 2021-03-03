#include "usg_common.h"
#include <sys/mount.h>
#include <getopt.h>
#include <uuid.h>
#include "path_assembler.h"
#include "kucker_config.h"
#include "container_cli.h"
#include "container_fs.h"
#include "logger_factory.h"
#include "container_dao.h"

static Logger& logger = LoggerFactory::getKuckerDaemonLogger();

static char kucker[256];

void usage(const char *name) {
	printf("Usage: %s {kucker-path}");
}

void recheck(std::string &id) {
	sleep(3);
	char line[526];
	ContainerInfo info = ContainerDao::get_container_by_id(id);
	if(info.status == CONTAINER_RUNNING) {
    sprintf(line, "/proc/%d", info.pid);
    if(access(line, F_OK) == -1) {
    	ContainerDao::change_status_to_stop(info.id);
    	printf("1 start %s\n", info.name.c_str());
    	sprintf(line, "sudo %s start -d %s", kucker, info.name.c_str());
    	system(line);
    	printf("2 start %s\n", info.name.c_str());
    	logger.info("start %s", info.name.c_str());
    }
  }
}

int main(int argc, char *argv[])
{

	char line[526];
	
	// char *kuckerPath = NULL;
	if(argc == 2 ) {
		// kuckerPath = argv[0];
		sprintf(kucker, "%s/backer",  argv[1]);
	} else {
		sprintf(kucker, "backer" );
	}


	

  while(true) {
  	std::vector<ContainerInfo>* vector = ContainerDao::list();
  	for(ContainerInfo & info : *vector) {
		  if(info.status == CONTAINER_RUNNING) {
		  	
		    sprintf(line, "/proc/%d", info.pid);
		    if(access(line, F_OK) == -1) {
		     	recheck(info.id);
		    }
		  }
	    // if(info.status == CONTAINER_STOPED && info.abnormal_stoped == 1) {
	    // 	sprintf(command, "sudo %s start -d %s", kucker, info.name.c_str());
	    // 	system(command);
	    // 	logger.info("start %s", info.name.c_str());
	    // }
	  }
	  delete vector;
	  sleep(3);
  }
  

  
} 