#	lib4014.a make file
#	SCCS:	@(#)t4014.mk	1.3

CC = cc$(SYS)

OL = /
SL = /usr/src/lib
INSDIR = $(OL)usr/lib
RDIR = $(SL)/libplot/t4014
REL = current
INS = :
LIST = lp
TESTDIR = .
CFLAGS = -OB
SOURCE = arc.c box.c circle.c close.c dot.c erase.c label.c \
	 line.c linemod.c move.c open.c point.c scale.c space.c subr.c
OFILES = arc.o box.o circle.o close.o dot.o erase.o label.o \
	 line.o linemod.o move.o open.o point.o scale.o space.o subr.o
MAKE = make

all:	lib$(SYS)4014.a
	:

lib$(SYS)4014.a:	$(OFILES)
	$(AR) r $(TESTDIR)/lib$(SYS)4014.a $(OFILES)
	$(INS) $(TESTDIR)/lib$(SYS)4014.a $(INSDIR)
	chmod 664 $(INSDIR)/lib$(SYS)4014.a
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSDIR); chown bin lib$(SYS)4014.a; chgrp bin lib$(SYS)4014.a; \
	 fi

install:
	$(MAKE) -f t4014.mk INS=cp OL=$(OL) SYS=$(SYS) AR=$(AR)

build:	bldmk
	get -p -r`gsid t4014 $(REL)` s.t4014.src $(REWIRE) | ntar -d $(RDIR) -g
bldmk:
	get -p -r`gsid t4014.mk $(REL)` s.t4014.mk > $(RDIR)/t4014.mk

listing:
	pr t4014.mk $(SOURCE) | $(LIST)
listmk:
	pr t4014.mk | $(LIST)

edit:
	get -p -e s.t4014.src | ntar -g

delta:
	ntar -p $(SOURCE) > t4014.src
	delta s.t4014.src
	rm -f $(SOURCE)

mkedit:  ;  get -e s.t4014.mk
mkdelta: ;  delta s.t4014.mk

clean:
	-rm -f $(OFILES)

clobber: clean
	-rm -f $(TESTDIR)/lib4014.a

delete:	clobber
	rm -f $(SOURCE)
