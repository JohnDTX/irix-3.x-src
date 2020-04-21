#
include $(ROOT)/usr/include/make/commondefs

CSRCS=	butdata.c font.c keyboard.c main.c makeshell.c \
	menu.c misc.c shelltool.c softq.c term_v50.c textport.c winlib.c \
	paste.c clone.c icon.c
GSHOBJS=	$(CSRCS:.c=.o)
BINDKEYOBJS= bindkey.o

LCOPTS= -O
TARGETS= wsh bindkey

default all: $(TARGETS)

clean:
	rm -f *.o a.out core

clobber: clean
	rm -f $(TARGETS)

depend:
	$(MKDEPEND) $(MAKEFILE) $(CSRCS)

install:
	$(INSTALL) -m 4755 -u root -g sys -F /bin wsh

wsh: $(GSHOBJS)
	$(CC) $(CFLAGS) -o wsh $(GSHOBJS) -lm -lgl

bindkey: bindkey.o
	$(CC) $(CFLAGS) -o bindkey bindkey.o
