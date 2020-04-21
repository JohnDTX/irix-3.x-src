# devel/makefile -- for microcode development system
#
# first version - uses local gfdev.h - new gfinit.c

INCLUDES = -I. -I../include -I$$HDWR/include
#  Look first for local include files.
FBCFLAGS = -DGFALPHA -DDC3 -DUC3
# this tells .c files to use ALPHA options.  May want to change selectively.
# GE-related code that isn't #ifdef'd should work fine (on GF1 board).
# _GF1 is a holdover from way back.  All _FBC options should be removed!

CC = cc68 $(INCLUDES) -DPM1 -DDEVEL
#  PM1 enables paul's charhandler stuff.
#  DEVEL invokes special options in ge2cmd.c, initall.c

IOLIB = /og3/paul/iolib/iolib.a
SRC =  ../src
TEST = ../test2

OBJS = char.b devload.b devcmd.b devcmdsb.b \
	expectint.b devmacro.b betacode.b \
	gl2rams.b gl2testall.b gl2fifotest.b \
	testpass.b fbinit.b fbinitmac.b fbtext.b \
	header.b remap.b gl2draws.b bustests.b \
	initall.b testvec.b gfinit.b bpctest.b \
	ge2cmd.b gedraw.b realdraw.b reset.b

GE2TSTS =$(TEST)/testmm1.b $(TEST)/testmm2.b $(TEST)/testmm3.b \
	$(TEST)/testmm4.b $(TEST)/testcl1.b $(TEST)/testcl2.b \
	$(TEST)/testcl3.b $(TEST)/testcl4.b $(TEST)/testcl5.b \
	$(TEST)/testcl6.b $(TEST)/testsc1.b $(TEST)/testgsi.b \
	$(TEST)/testgso.b $(TEST)/test10sc1.b \
	$(TEST)/test10gso.b

gfdevel:	$(OBJS) $(GE2TSTS)
	cd ../test2;make
	cc68 -s -r -o gfdevel $(OBJS) $(GE2TSTS) $(IOLIB)
	cp gfdevel $$BOOT/test

gfsmall:	$(OBJS)
	ld68 -s -r -o gfdevel $(OBJS) $(IOLIB)

header.b: header.c
	cc68 -c header.c

gfinit.b:	gfinit.c
	cc68 -I$$HDWR/include -c -DGFBETA gfinit.c

remap.b:	$(SRC)/remap.c
	$(CC) -c $(SRC)/remap.c

reset.b:	$(SRC)/reset.c
	cc68 -I$$HDWR/include -c $(SRC)/reset.c

realdraw.b:	$(SRC)/realdraw.c
	$(CC) -c $(SRC)/realdraw.c

gedraw.b:	$(SRC)/gedraw.c
	$(CC) -c $(FBCFLAGS) $(SRC)/gedraw.c

ge2cmd.b:	$(SRC)/ge2cmd.c
	$(CC) -c $(FBCFLAGS) $(SRC)/ge2cmd.c

testvec.b:	$(SRC)/testvec.c
	$(CC) -c $(FBCFLAGS) $(SRC)/testvec.c

initall.b:	$(SRC)/initall.c
	$(CC) -c -DDC3 -DUC4 $(SRC)/initall.c

bustests.b:	$(SRC)/bustests.c
	$(CC) -c $(SRC)/bustests.c

gl2draws.b:	$(SRC)/gl2draws.c
	$(CC) -c -DGFBETA -DDEVEL $(SRC)/gl2draws.c

fbtext.b:	$(SRC)/fbtext.c
	$(CC) -c -DGFALPHA -DUC4 $(SRC)/fbtext.c

bpctest.b:	$(SRC)/bpctest.c
	$(CC) -c -DGFALPHA -DUC4 -DDC3 $(SRC)/bpctest.c

fbinitmac.b:	$(SRC)/fbinitmac.c
	$(CC) -c $(FBCFLAGS) $(SRC)/fbinitmac.c

fbinit.b:	$(SRC)/fbinit.c
	$(CC) -c -DUC4 $(SRC)/fbinit.c

testpass.b:	$(SRC)/testpass.c
	$(CC) -c $(SRC)/testpass.c

fifotest.b:	$(SRC)/fifotest.c
	$(CC) -c $(FBCFLAGS) $(SRC)/fifotest.c

gl2fifotest.b:	$(SRC)/gl2fifotest.c
	$(CC) -c $(FBCFLAGS) $(SRC)/gl2fifotest.c

testall.b:	$(SRC)/testall.c
	$(CC) -c $(FBCFLAGS) $(SRC)/testall.c

gl2testall.b:	$(SRC)/gl2testall.c
	$(CC) -c $(FBCFLAGS) $(SRC)/gl2testall.c

betarams.b:	$(SRC)/betarams.c
	$(CC) -c $(FBCFLAGS) $(SRC)/betarams.c

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

expectint.b:	$(SRC)/expectint.c
	$(CC) -c $(FBCFLAGS) $(SRC)/expectint.c


ci:
	foreach i(*.c *.h *akefile)
	    ci -u -n$(VERSION) $i
	end

clean:
	rm -f *CKP *BAK *.b *.r *.br *.o *.v .emacs*

.SUFFIXES:
.SUFFIXES: .s .ls .e .b .o .c .r .v .br


.c.b:
	$(CC) -c $(FBCFLAGS) $<
