
#
#
include ${ROOT}/usr/include/make/commondefs

OBJS	= sed0.o sed1.o

all:	sed

install:all
	${INSTALL} -m 755 -g bin -u bin -F /bin \
		-idb "root upd" sed

clean:
	rm -f *.o

clobber:clean
	rm -f sed

sed:	$(OBJS)
	$(CCF) -o $@ $(OBJS) $(LDFLAGS)

