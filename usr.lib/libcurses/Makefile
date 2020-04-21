#
include ${ROOT}/usr/include/make/commondefs
#
# if the headers are changed one MUST do "make clobber"!!!
#
# @(#)$Header: /d2/3.7/src/usr.lib/libcurses/RCS/Makefile,v 1.1 89/03/27 18:34:31 root Exp $
# $Log:	Makefile,v $
# Revision 1.1  89/03/27  18:34:31  root
# Initial check-in for 3.7
# 
# Revision 1.9  87/08/25  22:35:58  andre
# dont't forget to ranlib the newly install lib
# 
# Revision 1.8  87/05/04  11:36:19  andre
# 3.6 Product, New 5.3 Style Installs Added, Native Installs done
# 
# Revision 1.7  87/04/19  01:45:35  andre
# 3.6 Product, New 5.3 Style Installs Added, Native Installs done
# 
# Revision 1.5  85/03/15  18:43:58  bob
# Changed so that clobber removes *.o (only needed if initial make fails).
# 

HEADERS	= curses.h unctrl.h cr_ex.h curses.ext

CFILES	= mvprintw.c scanw.c getstr.c printw.c addstr.c getch.c initscr.c \
	overlay.c overwrite.c addch.c clear.c insertln.c tstp.c endwin.c \
	refresh.c box.c cr_put.c cr_tty.c mvscanw.c mvwin.c scroll.c \
	clrtobot.c clrtoeol.c curses.c deleteln.c delwin.c erase.c longname.c \
	move.c newwin.c standout.c touchwin.c unctrl.c insch.c delch.c

# set by Makefile for profiling
CFLAGS2	=

#	The member files.
MEMBERS	= \
	${LIB}(mvprintw.o)	\
	${LIB}(scanw.o)		\
	${LIB}(getstr.o)	\
	${LIB}(printw.o)	\
	${LIB}(addstr.o)	\
	${LIB}(getch.o)		\
	${LIB}(initscr.o)	\
	${LIB}(overlay.o)	\
	${LIB}(overwrite.o)	\
	${LIB}(addch.o)		\
	${LIB}(clear.o)		\
	${LIB}(insertln.o)	\
	${LIB}(tstp.o)		\
	${LIB}(endwin.o)	\
	${LIB}(refresh.o)	\
	${LIB}(box.o)		\
	${LIB}(cr_put.o)	\
	${LIB}(cr_tty.o)	\
	${LIB}(mvscanw.o)	\
	${LIB}(mvwin.o)		\
	${LIB}(scroll.o)	\
	${LIB}(clrtobot.o)	\
	${LIB}(clrtoeol.o)	\
	${LIB}(curses.o)	\
	${LIB}(deleteln.o)	\
	${LIB}(delwin.o)	\
	${LIB}(erase.o)		\
	${LIB}(longname.o)	\
	${LIB}(move.o)		\
	${LIB}(newwin.o)	\
	${LIB}(standout.o)	\
	${LIB}(touchwin.o)	\
	${LIB}(unctrl.o)	\
	${LIB}(insch.o)		\
	${LIB}(delch.o)		\
	${LIB}(ttysubs.o)	\
	${NOOP}

CTAGS	= ctags
LINT	= lint
LPASS1	= /usr/lib/lint/lint1
LIB	= libcurses.a
LIBP	= libcurses_p.a

all:	${LIB} ${LIBP}

${LIB}:	${MEMBERS}
	ranlib $@

${LIBP}:${MEMBERS}
	${MAKE} LIB=${LIBP} LIBP=noop CFLAGS2=-p ${LIBP}

install:all
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix" "${LIB}"
	ranlib ${ROOT}/usr/lib/"${LIB}"
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix" "${LIBP}"
	ranlib ${ROOT}/usr/lib/"${LIBP}"

clean:
	rm -f *.o
	
clobber:clean
	rm -f ${LIB} ${LIBP}

.c.o:
	${CC} ${CFLAGS} ${CFLAGS2} -c $< ; ${LD} -r -x $@ ; mv a.out $@

.c.a:
	${CC} ${CFLAGS} ${CFLAGS2} -c $< ; set +e;${LD} -r -x $%;test $$? -le 1
	/bin/mv a.out $% ; ar rv $@ $% ; /bin/rm -f $%

.PRECIOUS:${LIB}

test:	${LIB} test.c testprof
	${CC} ${NFLAG} ${LDFLAGS} ${CFLAGS} ${CFLAGS2} -o $@ test.c ${LIB} \
	  -ltermcap

testprof:${LIBP} test.c
	${CC} ${NFLAG} ${LDFLAGS} ${CFLAGS} ${CFLAGS2} -o $@ test.c ${LIBP} \
	  -ltermcap

cat:	cat.c curses.h

ctags:
	${CTAGS} ${CFILES} curses.h cr_ex.h

lint:
	${LINT} -hxb ${CFL} ${CFILES} -lcurses > lint.out

lpr:
	-pr curses.h ${CFILES} | lpr

tp:
	tp crm0 Makefile tags ${HEADERS} ${CFILES} llib-lcurses

tar:
	tar -crvf curses.tar Makefile tags ${HEADERS} ${CFILES} llib-lcurses

llib-lcurses.ln:llib-lcurses
	-(/lib/cpp -C -Dlint llib-lcurses | ${LPASS1} > llib-lcurses.ln ) \
	  2>&1 | grep -v warning
