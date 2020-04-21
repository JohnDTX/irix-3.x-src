# gf2/makefile -- for GF2 in terminal configuration
#

HDWR = $$HDWR

INCLUDES = -I. -I../include -I$(HDWR)/include -I/oh4/hdwr/include
#  Look first for local include files.
FBCFLAGS = -DGF2 -DDC3 -DUC4

CC = cc68 $(INCLUDES) -DPM1
#  PM1 enables paul's charhandler stuff.

PROGRAM = gf2ld
IOLIB = /og3/paul/iolib/iolib.a
SRC =  ../src
TEST = ../test2
GA   = ../testga

OBJS = char.b devload.b devcmd.b devcmdsb.b \
	devmacro.b betacode.b breakpt.b \
	gl2rams.b gl2testall.b gl2fifotest.b \
	testpass.b fbinit.b fbinitmac.b fbtext.b \
	header.b gl2draws.b gl2bustests.b \
	initall.b testvec.b bpctest.b gf2features.b \
	ge2cmd.b gedraw.b realdraw.b reset.b gepa.b \
	gfinit.b swizzletab.b inittables.b bitmask.b

MOBJS = main.b microwrite.b betacode.b

GE2TSTS =$(TEST)/testmm1.b $(TEST)/testmm2.b $(TEST)/testmm3.b \
	$(TEST)/testmm4.b $(TEST)/testcl1.b $(TEST)/testcl2.b \
	$(TEST)/testcl3.b $(TEST)/testcl4.b $(TEST)/testcl5.b \
	$(TEST)/testcl6.b $(TEST)/testsc1.b $(TEST)/testgsi.b \
	$(TEST)/testgso.b $(TEST)/test10sc1.b \
	$(TEST)/test10gso.b

GATESTS = $(GA)/headin0.b $(GA)/headin1.b $(GA)/headin2.b $(GA)/headin3.b \
	$(GA)/headin4.b $(GA)/headin5.b $(GA)/headin6.b $(GA)/headin7.b \
	$(GA)/headin8.b $(GA)/headin9.b \
	$(GA)/headout0.b $(GA)/headout1.b $(GA)/headout2.b $(GA)/headout3.b \
	$(GA)/headout4.b $(GA)/headout5.b $(GA)/headout6.b $(GA)/headout7.b \
	$(GA)/headout8.b $(GA)/headout9.b \
	$(GA)/tailin0.b $(GA)/tailin1.b $(GA)/tailin2.b $(GA)/tailin3.b \
	$(GA)/tailin4.b \
	$(GA)/tailout0.b $(GA)/tailout1.b $(GA)/tailout2.b $(GA)/tailout3.b \
	$(GA)/tailout4.b

$(PROGRAM):	$(OBJS)
	cd ../testga;make
	cd ../test2;make
	cc68 -s -r -o $(PROGRAM) $(OBJS) $(GATESTS) $(GE2TSTS) $(IOLIB)
#	nm68 -xn $(PROGRAM) >$(PROGRAM).sym
#	rev68 $(PROGRAM) >$(PROGRAM).r
	cp $(PROGRAM) $$BOOT/test/gf2ld

small:	$(OBJS)
	ld68 -s -r -o $(PROGRAM) $(OBJS) $(IOLIB)

microload: $(MOBJS)
	cc68 -s -r -o microload $(MOBJS) $(IOLIB)
	cp microload $$BOOT/test


main.b: main.c
	cc68 -c main.c

header.b: header.c
	cc68 -c header.c

microwrite.b: $(SRC)/microwrite.c
	cc68 -I. -I$(HDWR)/include -c $(SRC)/microwrite.c


breakpt.b: $(SRC)/breakpt.c
	$(CC) -c $(FBCFLAGS) $(SRC)/breakpt.c

reset.b:	$(SRC)/reset.c
	cc68 -I. -I$(HDWR)/include -c $(SRC)/reset.c

realdraw.b:	$(SRC)/realdraw.c
	$(CC) -c $(SRC)/realdraw.c

gedraw.b:	$(SRC)/gedraw.c
	$(CC) -c $(FBCFLAGS) $(SRC)/gedraw.c

bpctest.b:	$(SRC)/bpctest.c
	$(CC) -c -I/oh4/hdwr/include $(FBCFLAGS) $(SRC)/bpctest.c

ge2cmd.b:	$(SRC)/ge2cmd.c
	$(CC) -c -O $(FBCFLAGS) $(SRC)/ge2cmd.c

gepa.b:		$(SRC)/gepa.c
	$(CC) -c $(FBCFLAGS) $(SRC)/gepa.c

gfinit.b:	$(SRC)/gfinit.c
	$(CC) -c $(FBCFLAGS) $(SRC)/gfinit.c

inittables.b:		$(SRC)/inittables.c
	$(CC) -c $(FBCFLAGS) $(SRC)/inittables.c

swizzletab.b:		$(SRC)/swizzletab.c
	$(CC) -c $(FBCFLAGS) $(SRC)/swizzletab.c

bitmask.b:		$(SRC)/bitmask.c
	$(CC) -c $(FBCFLAGS) $(SRC)/bitmask.c

gf2features.b:	$(SRC)/gf2features.c
	$(CC) -c $(FBCFLAGS) $(SRC)/gf2features.c

testvec.b:	$(SRC)/testvec.c
	$(CC) -c $(FBCFLAGS) $(SRC)/testvec.c

initall.b:	$(SRC)/initall.c
	$(CC) -c $(FBCFLAGS) $(SRC)/initall.c

gl2bustests.b:	$(SRC)/gl2bustests.c
	$(CC) -c $(FBCFLAGS) $(SRC)/gl2bustests.c

gl2draws.b:	$(SRC)/gl2draws.c
	$(CC) -c $(SRC)/gl2draws.c

fbtext.b:	$(SRC)/fbtext.c
	$(CC) -c $(FBCFLAGS) $(SRC)/fbtext.c

fbinitmac.b:	$(SRC)/fbinitmac.c
	$(CC) -c $(FBCFLAGS) $(SRC)/fbinitmac.c

fbinit.b:	$(SRC)/fbinit.c
	$(CC) -c $(FBCFLAGS) $(SRC)/fbinit.c

testpass.b:	$(SRC)/testpass.c
	$(CC) -c $(SRC)/testpass.c

gl2fifotest.b:	$(SRC)/gl2fifotest.c
	$(CC) -c $(FBCFLAGS) $(SRC)/gl2fifotest.c

gl2testall.b:	$(SRC)/gl2testall.c
	$(CC) -c $(FBCFLAGS) $(SRC)/gl2testall.c

gl2rams.b:	$(SRC)/gl2rams.c
	$(CC) -c $(FBCFLAGS) $(SRC)/gl2rams.c

devmacro.b:	$(SRC)/devmacro.c
	$(CC) -c $(FBCFLAGS) $(SRC)/devmacro.c

devcmdsb.b:	$(SRC)/devcmdsb.c
	$(CC) -c $(FBCFLAGS) $(SRC)/devcmdsb.c

devcmd.b:	$(SRC)/devcmd.c
	$(CC) -c $(FBCFLAGS) $(SRC)/devcmd.c

devload.b:	$(SRC)/devload.c
	$(CC) -c $(FBCFLAGS) $(SRC)/devload.c

char.b:	$(SRC)/char.c
	$(CC) -c $(SRC)/char.c


# this doesn't work. must run under csh.
ci:
	foreach i (*.c *.h *akefile)
	    ci -u -n$(VERSION) $i
	end

clean:
	rm -f *CKP *BAK *.b *.r *.br *.o *.v .emacs*

.SUFFIXES:
.SUFFIXES: .s .ls .e .b .o .c .r .v .br


.c.b:
	$(CC) -c $(FBCFLAGS) $<
