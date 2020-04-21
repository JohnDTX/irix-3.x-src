
include ${ROOT}/usr/include/make/commondefs

LCDEFS  = -DIBM -DNEWSTDIO -DV -DPM2 -DDC4 -DUC4
LCOPTS  = -OBPS -N 
LCINCS  = -I$(ROOT)/usr/include/$(GLHDWR)  -I$(ROOT)/usr/include/vkernel
LIBS = $(ROOT)/usr/lib/libVgl2.a $(ROOT)/usr/lib/libV.a 
VKERNEL = $(ROOT)/usr/lib/$(GLHDWR)/vkernel

all: trivial

clean:
	rm -f trivial *.o *.r *.dsk *.68

install: all

trivial.r: trivial.o
	cc --n -e _TeamRoot -T 40000 trivial.o -o trivial.r $(LIBS)

trivial: trivial.r
	makeboot trivial.r temp.boot -l 40000 -h 3FFE0
	glueboot $(VKERNEL) temp.boot -o trivial
	rm temp.boot

syms: 
	nm trivial.68 -n > syms

.c.o:	
	$(CC) $(CFLAGS) -c $*.c

.c.ls:	
	$(CC) -L $(CFLAGS) -c $*.c

.c.s:	
	$(CC) -S $(CFLAGS) -c $*.c
