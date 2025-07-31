ROOT := ..
PLATFORM=$(shell $(ROOT)/systype.sh)
include $(ROOT)/Make.defines.$(PLATFORM)

#RPATH += -Wl,-rpath=${ROOT}/lib
# 开源工具包路径
#LDDIR += -L${ROOT}/lib

# 开源工具包
LDLIBS += -lusgcommon -luuid -ljsoncpp

INCLUDES += -I$(ROOT)/include/usgcommon -I$(ROOT)/include/uuid  -I./cli/include/


PROGS =	$(DEST)/script

DEPENDENCES = $(patsubst %, %.d, $(PROGS)) 

#LIBCOMMON=${ROOT}/lib/libusgcommon.a

build: $(PROGS)
 

# class
$(DEST)/script : script.c pty_fork.c pty_master_open.c tty_functions.c


clean:
	rm -f $(PROGS) $(DEPENDENCES) $(TEMPFILES)

# $(LIBCOMMON):
# 	@(cd $(ROOT)/common && $(MAKE))

-include $(DEPENDENCES)
include $(ROOT)/Make.common.inc


