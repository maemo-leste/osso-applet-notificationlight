OBJS=cpnotificationlight.o
SOURCES=cpnotificationlight.c
BUILDDIR=../build
LIB=$(BUILDDIR)/libcpnotificationlight.so
PKG_FLAGS=$(shell pkg-config glib-2.0 hildon-1 libhildondesktop-1 gconf-2.0 --libs --cflags)
CCFLAGS=-shared -fPIC
CC=gcc

all:$(LIB)

$(LIB):$(OBJS)
	$(CC) $(CCFLAGS) $(PKG_FLAGS) $(OBJS) -o $(LIB)

.c.o:
	$(CC) $(CCFLAGS) $(PKG_FLAGS) -c $< -o $@

.PHONE: clean all

clean:
	rm -f $(OBJS)
