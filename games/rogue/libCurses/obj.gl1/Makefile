
include ${ROOT}/usr/include/make/commondefs

LCDEFS = -Diris -DCOLOR -D$(GLHDWR) 
LCINCS = -I. -I${ROOT}/usr/include/$(GLHDWR) 

SOURCE = Curses.h addch.c addstr.c box.c clear.c \
	clrtobot.c clrtoeol.c cr_ex.h cr_put.c cr_tty.c curses.c \
	curses.ext curses.h delch.c deleteln.c delwin.c endwin.c \
	erase.c getch.c getstr.c initscr.c insch.c insertln.c iris.c \
	longname.c move.c mvprintw.c mvscanw.c mvwin.c newwin.c \
	overlay.c overwrite.c printw.c refresh.c scanw.c scroll.c \
	standout.c touchwin.c tstp.c ttyctl.c unctrl.c unctrl.h

OBJS =	addch.o addstr.o box.o clear.o clrtobot.o clrtoeol.o cr_put.o \
	cr_tty.o curses.o delch.o deleteln.o delwin.o endwin.o erase.o \
	getch.o getstr.o initscr.o insch.o insertln.o iris.o longname.o \
	move.o mvprintw.o mvscanw.o mvwin.o newwin.o overlay.o \
	overwrite.o printw.o refresh.o scanw.o scroll.o standout.o \
	touchwin.o tstp.o ttyctl.o unctrl.o

.c.o:
	${CCF} -c ../$<

libCurses.a: $(OBJS)
	rm -f libCurses.a
	ar crv libCurses.a $(OBJS)
	ranlib libCurses.a

install: libCurses.a

clean:
	rm -f libCurses.a *.o $(SOURCE)

$(OBJS): $(SOURCE)

$(SOURCE):
	d=`pwd` ; cd .. ; ln $(SOURCE) $$d
