ROOT := ..
PLATFORM=$(shell $(ROOT)/systype.sh)
include $(ROOT)/Make.defines.$(PLATFORM)

#RPATH += -Wl,-rpath=${ROOT}/lib
# 开源工具包路径
#LDDIR += -L${ROOT}/lib

# 开源工具包
LDLIBS += -lusgcommon -luuid

INCLUDES += -I$(ROOT)/include/usgcommon -I$(ROOT)/include/uuid


PROGS =	${DEST}/test_str ${DEST}/clone_user ${DEST}/getopt_long ${DEST}/test_uuid ${DEST}/test_strtok ${DEST}/test \
	${DEST}/string_find

DEPENDENCES = $(patsubst %, %.d, $(PROGS)) 

#LIBCOMMON=${ROOT}/lib/libusgcommon.a

all:	 $(PROGS)

# class
#$(DEST)/kucker : kucker.c


clean:
	rm -f $(PROGS) $(DEPENDENCES) $(TEMPFILES)

# $(LIBCOMMON):
# 	@(cd $(ROOT)/common && $(MAKE))

-include $(DEPENDENCES)
include $(ROOT)/Make.common.inc



