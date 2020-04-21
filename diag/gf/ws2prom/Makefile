# ws2prom/makefile -- for non-interactive confidence test in oven
#

HDWR = $$HDWR

INCLUDES = -I. -I../include -I$(HDWR)/include -I/oh4/hdwr/include
#  Look first for local include files.
FBCFLAGS = -DGF2 -DDC4 -DUC4 -DEPROM

CC = cc68 $(INCLUDES) -DPM2 -DPROMALONE -O
#  PM1 enables paul's charhandler stuff.

PROGRAM = gf2prom
SRC =  ../src
TEST = ../test2
GA   = ../testga
INC = -i/ws/pmII/stand/crtstand.b
LIB = /ws/pmII/stand/libstand.a
WS  = /ws/vR1/usr/src/prom


OBJS = gl2rams.b testall.b \
	header.b gl2initall.b gl2bpctest.b \
	reset.b \
	mch.b betacode.b gl2bustests.b

$(PROGRAM):	$(OBJS) main.b
	cc68 -s -r -o $(PROGRAM) main.b $(INC) $(OBJS) $(LIB)
	rm -f $$BOOT/test/$(PROGRAM)
	cp $(PROGRAM) $$BOOT/test

objs: $(OBJS)

main.b: main.c
	cc68 -c main.c

header.b: header.c
	cc68 -c header.c

reset.b:	$(SRC)/reset.c
	$(CC) -c -I. -I$(HDWR)/include -c $(SRC)/reset.c

gl2bpctest.b:	$(SRC)/gl2bpctest.c
	$(CC) -c $(FBCFLAGS) $(SRC)/gl2bpctest.c

gl2bustests.b:	$(SRC)/gl2bustests.c
	$(CC) -c $(FBCFLAGS) $(SRC)/gl2bustests.c

gfinit.b:	$(SRC)/gfinit.c
	$(CC) -c $(FBCFLAGS) $(SRC)/gfinit.c

inittables.b:		$(SRC)/inittables.c
	$(CC) -c $(FBCFLAGS) $(SRC)/inittables.c

swizzletab.b:		$(SRC)/swizzletab.c
	$(CC) -c $(FBCFLAGS) $(SRC)/swizzletab.c

bitmask.b:		$(SRC)/bitmask.c
	$(CC) -c $(FBCFLAGS) $(SRC)/bitmask.c

gl2initall.b:	$(SRC)/gl2initall.c
	$(CC) -c $(FBCFLAGS) $(SRC)/gl2initall.c

fbinit.b:	$(SRC)/fbinit.c
	$(CC) -c $(FBCFLAGS) $(SRC)/fbinit.c

mch.b: $(SRC)/mch.s
	cc68 -c $(SRC)/mch.s



clean:
	rm -f *CKP *BAK *.b *.r *.br *.o *.v .emacs*

.SUFFIXES:
.SUFFIXES: .s .ls .e .b .o .c .r .v .br


.c.b:
	$(CC) -c $(FBCFLAGS) $<
