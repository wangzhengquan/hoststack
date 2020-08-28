ROOT=..
PLATFORM=$(shell $(ROOT)/systype.sh)
include $(ROOT)/Make.defines.$(PLATFORM)

LDLIBS+=-Wl,-rpath=${ROOT}/lib
# 开源工具包路径
LDDIR += -L${ROOT}/lib

# 开源工具包
LDLIBS += -lusgcommon

INCLUDE += -I${ROOT}/include/usgcommon

PROGS =	${DEST}/clone1 ${DEST}/clone_user ${DEST}/getopt_long

default: build

build: ${PROGS}
	test -d ${DEST} || mkdir ${DEST}

# class
${DEST}/clone1: clone1.c


clean:
	rm -f $(PROGS) $(TEMPFILES)

${DEST}/%:	%.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

# include $(ROOT)/Make.common.inc
