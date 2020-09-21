ROOT := ..
PLATFORM=$(shell $(ROOT)/systype.sh)
include $(ROOT)/Make.defines.$(PLATFORM)

#RPATH += -Wl,-rpath=${ROOT}/lib
# 开源工具包路径
#LDDIR += -L${ROOT}/lib

# 开源工具包
LDLIBS += -lusgcommon -luuid -llog4cpp

INCLUDES += -I$(ROOT)/include/usgcommon -I$(ROOT)/include/uuid -I$(ROOT)/include/log4cpp


PROGS =	${DEST}/test_log4cpp ${DEST}/clone_user ${DEST}/getopt_long ${DEST}/test_uuid ${DEST}/test_strtok ${DEST}/test \
	${DEST}/string_find

DEPENDENCES = $(patsubst %, %.d, $(PROGS)) 

#LIBCOMMON=${ROOT}/lib/libusgcommon.a

build:	 $(PROGS)
	mv *.properties ${DEST}

# class
#$(DEST)/kucker : kucker.c


clean:
	rm -f $(PROGS) $(DEPENDENCES) $(TEMPFILES)

# $(LIBCOMMON):
# 	@(cd $(ROOT)/common && $(MAKE))

-include $(DEPENDENCES)
include $(ROOT)/Make.common.inc



