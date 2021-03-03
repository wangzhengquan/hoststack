ROOT := ..
PLATFORM=$(shell $(ROOT)/systype.sh)
include $(ROOT)/Make.defines.$(PLATFORM)

#RPATH += -Wl,-rpath=${ROOT}/lib
# 开源工具包路径
#LDDIR += -L${ROOT}/lib

# 开源工具包
LDLIBS += -lusgcommon -luuid -ljsoncpp

INCLUDES += -I$(ROOT)/include/usgcommon -I$(ROOT)/include/uuid  -I./cli -I./pty

# SOURCES := $(wildcard *.c ./**/*.c)

SOURCES := ./container_fs.c \
					./path_assembler.c \
					./container_service.c \
					./cli/container_cli.c \
					./cli/container_attach_cli.c \
					./cli/container_exec_cli.c \
					./cli/container_run_cli.c \
					./cli/container_start_cli.c \
					./cli/container_ls_cli.c \
					./cli/container_rm_cli.c \
					./cli/container_stop_cli.c \
					./container_info.c \
					./pty/pty_server.c \
					./pty/pty_fork.c \
					./pty/pty_exec_util.c \
					./pty/tty_functions.c \
					./pty/pty_master_open.c \
					./container_dao.c \


PROGS =	$(DEST)/kucker $(DEST)/kucker_daemon

DEPENDENCES = $(patsubst %, %.d, $(PROGS)) 

#LIBCOMMON=${ROOT}/lib/libusgcommon.a

build:	 $(PROGS)
	#cp -a $(ROOT)/etc $(DEST) 
	sudo chown root:root $(DEST)/kucker
	sudo chmod u+s $(DEST)/kucker
	#sudo chmod 766 $(DEST)/kucker
	sudo mv $(DEST)/kucker $(DEST)/backer
	md5sum $(DEST)/backer > $(DEST)/backer.md5
	# md5sum -c $(DEST)/backer.md5

# class
$(DEST)/kucker : $(SOURCES)
$(DEST)/kucker_daemon : $(SOURCES)


clean:
	rm -f $(PROGS) $(DEPENDENCES) $(TEMPFILES)

# $(LIBCOMMON):
# 	@(cd $(ROOT)/common && $(MAKE))

-include $(DEPENDENCES)
include $(ROOT)/Make.common.inc


