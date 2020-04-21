# STAR TREK GAME MAKEFILE
# $Source: /d2/3.7/src/games/trek/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 15:46:29 $

include ${ROOT}/usr/include/make/commondefs

B	= /usr/games
GRPOWN  = -u games -g games
IDBTAG  = -idb "std.sw.unix"
LLDLIBS =  ${ROOT}/usr/lib/libm.a

OBJS	= abandon.o adm.o attack.o checkcond.o compkl.o \
	  computer.o damage.o destruct.o dock.o dump.o \
	  events.o externs.o extra.o getcodi.o \
	  getpar.o help.o impulse.o input.o initquad.o kill.o \
	  klmove.o log.o lose.o lrscan.o main.o move.o \
	  nova.o phaser.o play.o query.o ram.o ranf.o \
	  report.o reset.o rest.o schedule.o score.o setup.o setwarp.o \
	  shell.o shield.o snova.o srscan.o ssradio.o torped.o visual.o \
	  warp.o win.o

all:	trek

install:all
	${INSTALL} ${GRPOWN} ${IDBTAG} -m 755 -F ${B} trek

clean:
	rm -f *.o

clobber:clean
	rm -f trek

trek:	$(OBJS)
	$(CCF) -o $@ $(OBJS) $(LDFLAGS)

$(OBJS):trek.h

lint:
	lint -abchx *.c
