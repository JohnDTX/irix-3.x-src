# $Source: /d2/3.7/src/bin/csh/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 14:53:10 $

include ${ROOT}/usr/include/make/commondefs

CTAGS	= ctags
CXREF	= cxref

OBJS	= sh.o sh.dol.o sh.err.o sh.exec.o sh.exp.o sh.func.o sh.glob.o \
	  sh.hist.o sh.lex.o sh.misc.o sh.parse.o sh.print.o sh.sem.o sh.set.o \
	  sh.wait.o alloc.o sh.init.o printf.o doprnt.o sh.dir.o

LLDLIBS	= -lsun -lbsd -ldbm

all:	csh

install:all
	${INSTALL} -u bin -g bin -m 755 -F /bin  \
	-idb "root upd" csh

clean:
	rm -f *.o

clobber:clean
	rm -f csh

csh:	$(OBJS)
	$(CCF) -o $@ $(OBJS) $(LDFLAGS)

lint:
	lint sh*.h sh*.c

print:
	@pr READ_ME
	@pr makefile makefile.*
	@(size -l csh; size *.o) | pr -h SIZES
	@$(CXREF) sh*.c | pr -h XREF
	@ls -l | pr 
	@pr sh*.h [a-rt-z]*.h sh*.c alloc.c

tags:
	$(CTAGS) sh*.c

sh.o:	sh.c sh.h sh.local.h

.c.o:
	$(CCF) -DPWB -DTELL -DBUFSIZ=1024 -c $<
