#	checkmm make file

#
#
include ${ROOT}/usr/include/make/commondefs

OBJ	= chekl.o chekmain.o

all:	checkmm

install:all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin  \
	-idb "std.sw.unix mr" checkmm

checkmm:$(OBJ)
	$(CC) $(NFLAG) $(LDFLAGS) -o $@ $(OBJ) ${ROOT}/usr/lib/libl.a ${ROOT}/usr/lib/libPW.a

clean:
	  rm -f *.o lex.yy.c

clobber:  clean
	  rm -f checkmm
