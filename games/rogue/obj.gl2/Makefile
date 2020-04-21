
include ${ROOT}/usr/include/make/commondefs

LCDEFS 	= -DCurses -Dregister= -DCOLOR -DWIZD -DCORE -Diris -D$(GLHDWR) 
LCINCS  = -I$(ROOT)/usr/include/$(GLHDWR) 
IDBTAG  = -idb "std.sw.unix"
GRPOWN  = -u games -g games
SGRPOWN = -u root  -g sys


SOURCE  = Curses.h armor.c chase.c command.c daemon.c daemons.c fight.c \
	init.c io.c list.c mach_dep.h main.c monsters.c move.c \
	new_level.c options.c pack.c passages.c potions.c rings.c rip.c \
	rogue.h rooms.c save.c things.c use.c vers.c wands.c weapons.c

OBJS	= vers.o main.o rooms.o passages.o command.o move.o fight.o \
	chase.o init.o new_level.o things.o use.o list.o io.o rip.o \
	daemon.o daemons.o weapons.o monsters.o save.o options.o armor.o \
	pack.o potions.o rings.o wands.o

LLDLIBS	= ../libCurses/obj.$(GLHDWR)/libCurses.a \
	  $(ROOT)/usr/lib/libtermlib.a \
	  $(ROOT)/usr/lib/lib$(GLHDWR).a \
	  $(ROOT)/usr/lib/libm.a \
	  $(ROOT)/usr/lib/libsun.a \
	  $(ROOT)/usr/lib/libbsd.a \
	  $(ROOT)/usr/lib/libdbm.a

include Idest

all:	rogue

install:all rogue_gal
	strip rogue
	${INSTALL} ${GRPOWN} ${IDBTAG} -m 600 -F /usr/games/lib  rogue_gal
	${INSTALL} ${SGRPOWN} ${IDBTAG} -m 4755 -F /usr/games  rogue

.c.o:
	$(CCF) -c $<

rogue:	$(OBJS) $(LIBS)
	$(CCF) $(OBJS) $(LDFLAGS) -o $@

/usr/games \
/usr/games/lib \
/usr/games/rogue:
	${INSTALL} -dir $@ -u guest -g guest -m 755 ${IDBTAG} 


$(OBJS):	$(SOURCE)

rogue_gal:
	cp /dev/null $@

clean:
	rm -f rogue rogue_gal *.o $(SOURCE)

$(SOURCE):
	d=`pwd` ; cd .. ; ln $(SOURCE) $$d
