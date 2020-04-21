include ${ROOT}/usr/include/make/commondefs

.SUFFIXES: .o .p
OBJS= asm68k.o 

all:	asm68k

install:	all
	cp asm68k ../bin
	chmod 755 ../bin/asm68k

asm68k:	$(OBJS)
	echo "/usr/lib/libp.a required to build this "
	CC_OPTS="${CC_OPTS} -ZH" $(PC) $(PCFLAGS) -o asm68k $(OBJS) 

.p.o:
	CC_OPTS="${CC_OPTS} -ZH " $(PC) $(PCFLAGS) -c $< 


clean:
	rm -f *.o *.obj *.j

clobber:	clean
	rm -f asm68k
