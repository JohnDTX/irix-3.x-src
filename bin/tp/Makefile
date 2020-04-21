
#
#
include ${ROOT}/usr/include/make/commondefs

OFILES	= tp0.o tp1.o tp2.o tp3.o

all:	tp

install:all
	${INSTALL} -g bin -u bin -m 755 -F /bin \
	-idb "root upd" tp

clean:
	rm -f $(OFILES)

clobber:clean
	rm -f tp

tp:	$(OFILES) 
	$(CC) $(LDFLAGS) -o $@ $(OFILES)

$(OFILES): tp.h
