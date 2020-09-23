#ifndef __CONTAINER_FS_H
#define __CONTAINER_FS_H

#include "usg_common.h"
#include "container.h"
#include "path_assembler.h"
#include "logger_factory.h"


class ContainerFs {
 
public:
	static void create_repo();
	static void create_container(const char *container_id);
	static void remove_container(const char *container_id);

	static void mount_container(const char * container_id);
  static void umount_container(const char * container_id);

  static void umount_volume_list (const char *container_id) ;
  static void umount_volume (const char * container_id, const char* _volume) ;

	static void mount_volume (const char * container_id, const char *volume) ;
	static void mount_volume_list (const char *container_id, std::set<std::string> &volume_list);
	static void bind_mount(const char *container_id, const char *src, const char *_dest);

};

#endif