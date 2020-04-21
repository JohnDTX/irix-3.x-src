#	lib300.a make file
#	SCCS:	@(#)t300.mk	1.3

CC = cc$(SYS)

OL = /
SL = /usr/src/lib
INSDIR = $(OL)usr/lib
RDIR = $(SL)/libplot/t300
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

all:	lib$(SYS)300.a
	:

lib$(SYS)300.a:	$(OFILES)
	$(AR) r $(TESTDIR)/lib$(SYS)300.a $(OFILES)
	$(INS) $(TESTDIR)/lib$(SYS)300.a $(INSDIR)
	chmod 664 $(INSDIR)/lib$(SYS)300.a
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSDIR); chown bin lib$(SYS)300.a; chgrp bin lib$(SYS)300.a; \
	 fi

install:
	$(MAKE) -f t300.mk INS=cp OL=$(OL) SYS=$(SYS) AR=$(AR)

build:	bldmk
	get -p -r`gsid t300 $(REL)` s.t300.src $(REWIRE) | ntar -d $(RDIR) -g
bldmk:
	get -p -r`gsid t300.mk $(REL)` s.t300.mk > $(RDIR)/t300.mk

listing:
	pr t300.mk $(SOURCE) | $(LIST)
listmk:
	pr t300.mk | $(LIST)

edit:
	get -p -e s.t300.src | ntar -g

delta:
	ntar -p $(SOURCE) > t300.src
	delta s.t300.src
	rm -f $(SOURCE)

mkedit:  ;  get -e s.t300.mk
mkdelta: ;  delta s.t300.mk

clean:
	-rm -f $(OFILES)

clobber: clean
	-rm -f $(TESTDIR)/lib300.a

delete:	clobber
	rm -f $(SOURCE)
