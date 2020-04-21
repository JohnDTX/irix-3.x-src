# ws/makefile -- for PM2-based hardware

PROGRAM = gfpm2
INCLUDES = -I. -I../include -I$$HDWR/include
FBCFLAGS = -DGFBETA -D_GF1 -DDC3 -DUC3
SRC =  ../src
TEST = ../test2

CC = cc68 $(INCLUDES) -DPM2

OBJS = char.b devload.b devcmd.b devcmdsb.b expectint.b\
	devmacro.b ../beta/betacode.b betarams.b testall.b \
	fifotest.b testpass.b fbcscreen.b quiettest.b \
	font.b \
	fbinit.b fbinitmac.b fbtext.b header.b \
	drawtests.b bustests.b bpctest.b initall.b testvec.b \
	ge2cmd.b gedraw.b realdraw.b reset.b

GE2TSTS =$(TEST)/testmm1.b $(TEST)/testmm2.b $(TEST)/testmm3.b \
	$(TEST)/testmm4.b $(TEST)/testcl1.b $(TEST)/testcl2.b \
	$(TEST)/testcl3.b $(TEST)/testcl4.b $(TEST)/testcl5.b \
	$(TEST)/testcl6.b $(TEST)/testsc1.b $(TEST)/testgsi.b \
	$(TEST)/testgso.b $(TEST)/test10sc1.b \
	 $(TEST)/test10gso.b

DDT = /usr/local/68000/lib/libddt.a /usr/local/68000/lib/libsunc.a
INC = -i/ws/pmII/stand/crtstand.b
LIB = /ws/pmII/stand/libstand.a
WS  = /ws/vR1/usr/src/prom

$(PROGRAM):	$(OBJS) $(GE2TSTS)
	cd $(TEST);make
	cc68 -s -r -o $(PROGRAM) $(INC) $(OBJS) $(GE2TSTS) $(LIB)
	cp $(PROGRAM) $$BOOT/test

test:	pm1pm2.b
#	cc68 -o test.out pm1pm2.b /ws/gb/prom/libpm2.a
	cc68 -o test.out pm1pm2.b
#	nm68 test.out >test.sym
	rev68 test.out test

pm1pm2.b: pm1pm2.c
#	$(CC) -c -i/ws/gb/prom/pm2/crt0.b pm1pm2.c
	cc68 -c -I$$HDWR/include pm1pm2.c

pm2mem: pm2mem.b
	cc68 -o pm2mem.out pm2mem.b /ws/gb/prom/libpm2.a
	rev68 pm2mem.out pm2mem

pm2mem.b:	pm2mem.c
	$(CC) -c -i/ws/gb/prom/pm2/crt0.b pm2mem.c

quiettest.b:	quiettest.c
	$(CC) -c $(FBCFLAGS) quiettest.c

header.b:	header.c
	cc68 -c header.c

gfsmall:	$(OBJS)
	ld68 -s -r -o $(PROGRAM) $(OBJS) $(IOLIB)

betacode.b:	betacode.c
	cc68 -c betacode.c

reset.b:	$(SRC)/reset.c
	$(CC) -c $(SRC)/reset.c

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

screentest: screentest.c fbcscreen.b font.b
	cc68 -r -o screentest $(INC) fbcscreen.b font.b screentest.c $(LIB)

font.b:	$(WS)/sys/font.c
	cc68 -c $(WS)/sys/font.c

fbcscreen.b:	$(WS)/sys/fbcscreen.c
	cc68 -c -I$(WS)/include -DPM2 $(WS)/sys/fbcscreen.c



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
