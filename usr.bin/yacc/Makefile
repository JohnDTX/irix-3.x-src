#
#
include ${ROOT}/usr/include/make/commondefs


OFILES	= y1.o y2.o y3.o y4.o

HFILES	= dextern files

all:	yacc

install boot:all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin  \
	-idb "std.sw.unix mr" yacc
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib  \
	-idb "std.sw.unix mr" yaccpar

clean:
	rm -f *.o

clobber:clean
	rm -f yacc

yacc:	$(OFILES)
	$(CC) $(LDFLAGS) -o $@ $(OFILES)

$(OFILES):$(HFILES)
