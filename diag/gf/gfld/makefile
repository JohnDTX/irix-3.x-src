# beta/makefile -- for GF1 in terminal configuration
#

INCLUDES = -I. -I../include -I$$HDWR/include
#  Look first for local include files.
FBCFLAGS = -DGFBETA -D_GF1 -DDC3 -DUC3
# _GF1 is a holdover from way back.  All _FBC options should be removed!

CC = cc68 $(INCLUDES) -DPM1
#  PM1 enables paul's charhandler stuff.

PROGRAM = gfld
IOLIB = /oh4/hdwr/diag/bpc/iolib/iolib.a
SRC =  ../src
TEST = ../test2

OBJS = char.b devload.b devcmd.b devcmdsb.b \
	expectint.b devmacro.b betacode.b \
	betarams.b testall.b fifotest.b \
	testpass.b fbinit.b fbinitmac.b fbtext.b \
	header.b drawtests.b bustests.b \
	initall.b testvec.b bpctest.b\
	ge2cmd.b gedraw.b realdraw.b reset.b

GE2TSTS =$(TEST)/testmm1.b $(TEST)/testmm2.b $(TEST)/testmm3.b \
	$(TEST)/testmm4.b $(TEST)/testcl1.b $(TEST)/testcl2.b \
	$(TEST)/testcl3.b $(TEST)/testcl4.b $(TEST)/testcl5.b \
	$(TEST)/testcl6.b $(TEST)/testsc1.b $(TEST)/testgsi.b \
	$(TEST)/testgso.b $(TEST)/test10sc1.b \
	$(TEST)/test10gso.b

$(PROGRAM):	$(OBJS) $(GE2TSTS)
	cd ../test2;make
	cc68 -s -r -o $(PROGRAM) $(OBJS) $(GE2TSTS) $(IOLIB)
	cp $(PROGRAM) $$BOOT/test

gfsmall:	$(OBJS)
	ld68 -s -r -o $(PROGRAM) $(OBJS) $(IOLIB)

header.b: header.c
	cc68 -c header.c

betacode.b:	betacode.c
	cc68 -c betacode.c

reset.b:	$(SRC)/reset.c
	$(CC) -c $(FBCFLAGS) $(SRC)/reset.c

realdraw.b:	$(SRC)/realdraw.c
	$(CC) -c $(SRC)/realdraw.c

gedraw.b:	$(SRC)/gedraw.c
	$(CC) -c $(FBCFLAGS) $(SRC)/gedraw.c

bpctest.b:	$(SRC)/bpctest.c
	$(CC) -c -I/oh4/hdwr/include $(FBCFLAGS) $(SRC)/bpctest.c

ge2cmd.b:	$(SRC)/ge2cmd.c
	$(CC) -c $(FBCFLAGS) $(SRC)/ge2cmd.c

testvec.b:	$(SRC)/testvec.c
	$(CC) -c $(FBCFLAGS) $(SRC)/testvec.c

initall.b:	$(SRC)/initall.c
	$(CC) -c $(FBCFLAGS) -I/oh4/hdwr/include $(SRC)/initall.c

bustests.b:	$(SRC)/bustests.c
	$(CC) -c $(SRC)/bustests.c

drawtests.b:	$(SRC)/drawtests.c
	$(CC) -c $(SRC)/drawtests.c

fbtext.b:	$(SRC)/fbtext.c
	$(CC) -c $(FBCFLAGS) $(SRC)/fbtext.c

fbinitmac.b:	$(SRC)/fbinitmac.c
	$(CC) -c $(FBCFLAGS) $(SRC)/fbinitmac.c

fbinit.b:	$(SRC)/fbinit.c
	$(CC) -c $(FBCFLAGS) $(SRC)/fbinit.c

testpass.b:	$(SRC)/testpass.c
	$(CC) -c $(SRC)/testpass.c

fifotest.b:	$(SRC)/fifotest.c
	$(CC) -c $(FBCFLAGS) $(SRC)/fifotest.c

testall.b:	$(SRC)/testall.c
	$(CC) -c $(FBCFLAGS) $(SRC)/testall.c

betarams.b:	$(SRC)/betarams.c
	$(CC) -c $(FBCFLAGS) $(SRC)/betarams.c

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
