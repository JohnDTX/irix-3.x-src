#
# Makefile for GF2 microcode sources
#

include ${ROOT}/usr/include/make/commondefs

LCINCS  = -I../../include -I${ROOT}/usr/include/${GLHDWR}

LCDEFS   = -DUC4 -DFBC

SRCS     =	zscanline.c

MICS =		zscanline.o

all: $(MICS) 

clean:
	rm -f $(OBJS) *.out *.CKP *.BAK .emacs* *.o *.hex* *.cksum *.x *.b


.c.o:	scandefs.h
	$(CCF) -c $<

.c.e:	scandefs.h
	$(CCF) -E $< >$@
