#
include $(ROOT)/usr/include/make/commondefs

CSRCS=	butdata.c font.c keyboard.c main.c makeshell.c \
	menu.c misc.c shelltool.c softq.c term_v50.c textport.c winlib.c \
	paste.c clone.c icon.c
GSHOBJS=	$(CSRCS:.c=.o)
BINDKEYOBJS= bindkey.o
IMGLIB= /usr/people/gifts/mextools/imglib/libimage.a

LCOPTS= -O
#LCDEFS= -DSHRINK
TARGETS= gsh bindkey

default all: $(TARGETS) $(DOC)

clean:
	rm -f *.o a.out core

clobber: clean
	rm -f $(TARGETS)

depend:
	$(MKDEPEND) Makefile $(CSRCS)

install:

gsh: $(GSHOBJS)
	$(CC) $(CFLAGS) -o gsh $(GSHOBJS) -lm -lgl

bindkey: bindkey.o
	$(CC) $(CFLAGS) -o bindkey bindkey.o
