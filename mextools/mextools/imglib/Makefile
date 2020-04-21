
include ${ROOT}/usr/include/make/commondefs

LCINCS  = -I../include -I$(ROOT)/usr/include 
OBJS = 	close.o filbuf.o flsbuf.o memimg.o name.o open.o \
	pix.o rdwr.o rle.o row.o scrsave.o
LCOPTS = -O 

all: libimage.a

clean:
	rm -f *.o
	rm -f libimage.a

install: all
	${INSTALL} -idb "std.sw.unix" -F /usr/lib  libimage.a 
	ranlib $(ROOT)/usr/lib/libimage.a

libimage.a: $(OBJS)
	ar cr libimage.a $(OBJS)
	ranlib libimage.a

.c.o:
	$(CCF) -c $<
