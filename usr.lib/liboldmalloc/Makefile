include ${ROOT}/usr/include/make/commondefs

LCINCS= -I/usr/include/gl2
LIB   = liboldmalloc.a
FILES = getmem.o malloc.o calloc.o

all:	${LIB}

${LIB}:	${FILES}
	${AR} rv ${LIB} ${FILES}
	strip $@

install: all
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib -idb "upd root" ${LIB}
	ranlib ${ROOT}/usr/lib/${LIB}

.c.o:
	${CCF} -c $<

clean:

clobber:clean
	rm -f *.o *.a
