#	lib300s.a make file
#	SCCS:	@(#)t300s.mk	1.3

CC = cc$(SYS)

OL = /
SL = /usr/src/lib
INSDIR = $(OL)usr/lib
RDIR = $(SL)/libplot/t300s
REL = current
INS = :
LIST = lp
TESTDIR = .
CFLAGS = -OB
SOURCE = con.h arc.c box.c circle.c close.c dot.c erase.c label.c \
	 line.c linmod.c move.c open.c point.c space.c subr.c
OFILES = arc.o box.o circle.o close.o dot.o erase.o label.o \
	 line.o linmod.o move.o open.o point.o space.o subr.o
MAKE = make

all:	lib$(SYS)300s.a
	:

lib$(SYS)300s.a:	$(OFILES)
	$(AR) r $(TESTDIR)/lib$(SYS)300s.a $(OFILES)
	$(INS) $(TESTDIR)/lib$(SYS)300s.a $(INSDIR)
	chmod 664 $(INSDIR)/lib$(SYS)300s.a
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSDIR); chown bin lib$(SYS)300s.a; chgrp bin lib$(SYS)300s.a; \
	 fi

install:
	$(MAKE) -f t300s.mk INS=cp OL=$(OL) SYS=$(SYS) AR=$(AR)

build:	bldmk
	get -p -r`gsid t300s $(REL)` s.t300s.src $(REWIRE) | ntar -d $(RDIR) -g
bldmk:
	get -p -r`gsid t300s.mk $(REL)` s.t300s.mk > $(RDIR)/t300s.mk

listing:
	pr t300s.mk $(SOURCE) | $(LIST)
listmk:
	pr t300s.mk | $(LIST)

edit:
	get -p -e s.t300s.src | ntar -g

delta:
	ntar -p $(SOURCE) > t300s.src
	delta s.t300s.src
	rm -f $(SOURCE)

mkedit:  ;  get -e s.t300s.mk
mkdelta: ;  delta s.t300s.mk

clean:
	-rm -f $(OFILES)

clobber: clean
	-rm -f $(TESTDIR)/lib300s.a

delete:	clobber
	rm -f $(SOURCE)
