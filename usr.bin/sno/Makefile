#
include ${ROOT}/usr/include/make/commondefs

OBJS	= sno1.o sno2.o sno3.o sno4.o

all:	sno

install:all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin \
	-idb "std.sw.unix mr" sno

clean:
	rm -f *.o

clobber:clean
	rm -f sno

sno:	$(OBJS)
	$(CC) -o sno $(LDFLAGS) $(OBJS)

$(OBJS):sno.h
