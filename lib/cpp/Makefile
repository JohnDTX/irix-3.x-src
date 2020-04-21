#
#
include ${ROOT}/usr/include/make/commondefs

LCDEFS = -Dunix=1 -DPWB -Dsgi=1 -DSVR0=1 -O -DPASCAL 
STRIP=	strip


SRCS=	cpp.c cpy.y yylex.h

OFILES=	cpp.o cpy.o

all:	cpp

cpp:	$(OFILES)
	$(CC) -n -o cpp $(OFILES)

cpy.c:	cpy.y
	yacc cpy.y
	mv y.tab.c cpy.c

cpy.o:	yylex.h

clean:
	rm -f ${OFILES} a.out cpp y.tab.c cpy.c

clobber:
	rm -f ${OFILES} a.out cpp y.tab.c cpy.c cpp

install: all
	$(STRIP) cpp
	${INSTALL} -u bin -g bin -m 755 -F /lib \
	-idb "std.sw.unix mr" cpp

.c.o:
	$(CC) $(CFLAGS) -c $*.c

release :
	release $(SRCS) $(HDRS) Makefile

snapshot :
	snapshot $(SRCS) $(HDRS) Makefile
