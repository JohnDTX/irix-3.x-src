#
# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.2 (Berkeley) 9/17/85
#
# Berkeley Mail
#
#
include ${ROOT}/usr/include/make/commondefs

OS=	USG
LCDEFS =-O -D${OS}
XSTR=	xstr
CTAGS=	ctags -w
VPRINT=
LLDLIBS=-lsun -lbsd -ldbm
AS=	-as
RM=	-rm
OBJS=	version.o aux.o cmd1.o cmd2.o cmd3.o cmdtab.o collect.o \
	config.o edit.o fio.o \
	getname.o head.o ${OS}.local.o lex.o list.o main.o \
	names.o optim.o popen.o quit.o send.o strings.o temp.o tty.o \
	vars.o str.o sigretro.o
SRCS=	aux.c cmd1.c cmd2.c cmd3.c cmdtab.c collect.c config.c edit.c fio.c \
	getname.c head.c ${OS}.local.c lex.c list.c main.c \
	names.c optim.c popen.c quit.c send.c strings.c temp.c tty.c vars.c \
	version.c sigretro.c
HDRS=	rcv.h configdefs.h def.h glob.h ${OS}.local.h local.h
ALL=	Mail fmt
S = $(SRCS) $(HDRS)

#
# Special massaging of C files for sharing of strings
#
.c.o:
	${CCF} -E $*.c | ${XSTR} -c -
	${CCF} -c x.c 
	mv x.o $*.o
	@$(RM) -f x.c

all:	${ALL}

Mail:	$S $(OBJS)
	$(RM) -f Mail
	$(CCF) -o Mail $(OBJS) $(LDFLAGS)
	@size Mail

install: all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin -idb "mail usr" "Mail"
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin -idb "upd usr" "fmt"
	cd misc; make install

version.o:	version.c
	$(CCF) -c version.c

$S:
	sccs get $@;

srcs:	$S

tags:	$(SRCS)
	${CTAGS} $(SRCS);

clean clobber:
	$(RM) -f *.o strings
	rm -f Mail a.out fmt x.c xs.c tags core

lint:
	lint $(CFLAGS) $(SRCS)

fmt:	fmt.c head.c
	$(CCF) fmt.c head.c -o fmt $(LDFLAGS)
	$(RM) fmt.o head.o
	size fmt

str.o: strings
	$(XSTR)
	$(CCF) -c xs.c
	mv xs.o str.o
	$(RM) xs.c

#cmdtab.o: cmdtab.c
#	$(CCF) -R -c cmdtab.c

print:	$S fmt.c
	@$(VPRINT) Makefile rcv.h def.h glob.h local.h v7.local.h $(SRCS) fmt.c

wc:
	@wc rcv.h def.h glob.h local.h v7.local.h $(SRCS)

sc:
	@grep -c \; rcv.h def.h glob.h local.h v7.local.h $(SRCS)
