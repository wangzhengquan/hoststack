#
# Makefile for common library.
#
ROOT=..

PLATFORM=$(shell $(ROOT)/systype.sh)
include $(ROOT)/Make.defines.$(PLATFORM)

PREFIX = ${DEST}
LIBKUCKER = $(DEST)/libkucker.a
DLIBKUCKER = $(DEST)/libkucker.so

SOURCES := $(wildcard *.c)
OBJS   = $(patsubst %.c, $(DEST)/%.o, $(SOURCES)) 

MYLIBS = $(LIBKUCKER) $(DLIBKUCKER)

ifeq ($(DEBUG),y)
  MYLIBS = $(LIBKUCKER)
else
  MYLIBS = $(LIBKUCKER) $(DLIBKUCKER)
endif

all: build
 

build: $(MYLIBS)

#static lib
$(LIBKUCKER): $(OBJS)
	$(AR) rv $@ $?
	$(RANLIB) $@

#dynamic lib
$(DLIBKUCKER): $(SOURCES)
	rm -f *.o
	$(CC) -fPIC -shared $(CFLAGS) $^ -o $@ $(LDFLAGS)


#PREFIX is environment variable, but if it is not set, then set default value
ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

# 使用方式： g++ test1.c  -lcommon
install: $(MYLIBS)
	install -d $(PREFIX)/lib/
	install -m 644 $^ $(PREFIX)/lib/
	install -d $(PREFIX)/include/kucker
	install -m 644 include/* $(PREFIX)/include/kucker

clean:
	rm -f $(TEMPFILES)

.PHONY: build clean install
# include $(ROOT)/Make.libapue.inc


$(DEST)/%.o:	%.c
	$(CC) $(CFLAGS) -c $^ -o $@ $(LDFLAGS) $(LDLIBS)