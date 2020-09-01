ROOT := ..
PLATFORM=$(shell $(ROOT)/systype.sh)
include $(ROOT)/Make.defines.$(PLATFORM)

#RPATH += -Wl,-rpath=${ROOT}/lib
# 开源工具包路径
#LDDIR += -L${ROOT}/lib

# 开源工具包
LDLIBS += -lusgcommon

INCLUDES += -I$(ROOT)/include/usgcommon


PROGS =	$(DEST)/kucker

DEPENDENCES = $(patsubst %, %.d, $(PROGS)) 

#LIBCOMMON=${ROOT}/lib/libusgcommon.a

build:	 $(PROGS)
	cp -a $(ROOT)/etc $(DEST) 
	sudo chmod 700 $(DEST)/kucker
	sudo chown root:root $(DEST)/kucker
# class
$(DEST)/kucker : kucker.c



clean:
	rm -f $(PROGS) $(DEPENDENCES) $(TEMPFILES)

# $(LIBCOMMON):
# 	@(cd $(ROOT)/common && $(MAKE))

-include $(DEPENDENCES)
include $(ROOT)/Make.common.inc


