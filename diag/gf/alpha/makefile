# fbc/makefile -- for alpha FBC hardware

#new .h file, alpha FBC, old BPC:
FBCFLAGS = -DGFALPHA -D_GF1 -DDC2 -DUC2

#new .h file, alpha FBC, new BPC:	( make fbcld or fbcldb or fbcbddt )
#FBCFLAGS = -DGFALPHA -D_GF1 -DDC3 -DUC3

CC = cc68 -I. -I$$HDWR/include -DPM1

IOLIB = /og3/paul/iolib/iolib.a
DEVOBJS = char.b fbcload.b fbccmd.b fbcsub.b \
	devmacro.b ../ucode.b devrams.b
BETAOBJS = char.b fbcload.b fbccmd.b fbcsub.b header.b\
	devmacro.b betarams.b testall.b ../testpass.b ../remap.b \
	../expectint.b
FBCOBJS = fbinit.b fbinitmac.b fbtext.b ../testgsi.b gecmd.b ../testgso.b \
	../drawtests.b ../bustests.b bpctest.b initall.b fifotest.b testvec.b
GEOBJS = ../testmm1.b ../testmm2.b ../testmm3.b ../testmm4.b ../testcl1.b \
	../testcl2.b ../testcl3.b ../testcl4.b ../testcl5.b ../testcl6.b \
	../testsc1.b ../testsc2.b
TEST = ../../test

fbcld:	$(BETAOBJS) $(FBCOBJS) $(GEOBJS) betacode.b
	cc68 -s -r -o fbcld $(BETAOBJS) $(FBCOBJS) $(GEOBJS) betacode.b \
		$(IOLIB)
	cp fbcld $$BOOT/test

fbcbddt: $(BETAOBJS) $(FBCOBJS)
	cc68 -r -o fbcbddt $(BETAOBJS) $(FBCOBJS) -lddt -lsunc

fbcld.a: $(BETAOBJS) $(FBCOBJS) $(GEOBJS)
	ar rc fbcld.a $(BETAOBJS) $(FBCOBJS) $(GEOBJS)
	ranlib68 fbcld.a
	mv fbcld.a /oh4/hdwr/src

fbcldt: fbcld.a betacode.b
	cc68 -s -r -o fbcldt betacode.b fbcld.a fbcld.a $(IOLIB)


codewrite:	codemain.b codewrite.b betacode.b
	$(CC) -s -r -o codewrite codemain.b codewrite.b betacode.b

codewrite.dl:	codemain.b codewrite.b ../../ucode.b
	$(CC) -o codewrite codemain.b codewrite.b ../../ucode.b
	dl68 codewrite >codewrite.dl

codemain.b:	codemain.c
	$(CC) -c codemain.c

codewrite.b:	codewrite.c
	$(CC) -DGFALPHA -c codewrite.c

header.b:	header.c
	cc68 -c header.c

../expectint.b:	../expectint.c
	$(CC) -c -o ../expectint.b ../expectint.c

../remap.b:	../remap.c
	$(CC) -c -o ../remap.b ../remap.c

devrams.b:	../../devrams.c
	$(CC) -c $(FBCFLAGS) ../../devrams.c

gecmd.b:	../gecmd.c
	$(CC) -c $(FBCFLAGS) ../gecmd.c

fbcload.b:	../../devload.c
	$(CC) -c $(FBCFLAGS) -o fbcload.b ../../devload.c

devmacro.b:	../../devmacro.c
	$(CC) -c $(FBCFLAGS) ../../devmacro.c

fbccmd.b:	../../devcmd.c
	$(CC) -c $(FBCFLAGS) -o fbccmd.b ../../devcmd.c

fbcsub.b:	../../devcmdsb.c
	$(CC) -c -o fbcsub.b $(FBCFLAGS) ../../devcmdsb.c

ufile.b:	../../ufile.c
	$(CC) -c ../../ufile.c

../ucode.b:	../../ucode.c
	$(CC) -c -o ../ucode.b ../../ucode.c

betacode.b:	betacode.c
	$(CC) -c betacode.c

../testgso.b:	$(TEST)/testgso.c
	$(CC) -c -o ../testgso.b $(TEST)/testgso.c

../testgsi.b:	$(TEST)/testgsi.c
	$(CC) -c -o ../testgsi.b $(TEST)/testgsi.c

../testmm1.b:	$(TEST)/testmm1.c
	$(CC) -c -o ../testmm1.b $(TEST)/testmm1.c

../testmm2.b:	$(TEST)/testmm2.c
	$(CC) -c -o ../testmm2.b $(TEST)/testmm2.c

../testmm3.b:	$(TEST)/testmm3.c
	$(CC) -c -o ../testmm3.b $(TEST)/testmm3.c

../testmm4.b:	$(TEST)/testmm4.c
	$(CC) -c -o ../testmm4.b $(TEST)/testmm4.c

../testcl1.b:	$(TEST)/testcl1.c
	$(CC) -c -o ../testcl1.b $(TEST)/testcl1.c

../testcl2.b:	$(TEST)/testcl2.c
	$(CC) -c -o ../testcl2.b $(TEST)/testcl2.c

../testcl3.b:	$(TEST)/testcl3.c
	$(CC) -c -o ../testcl3.b $(TEST)/testcl3.c

../testcl4.b:	$(TEST)/testcl4.c
	$(CC) -c -o ../testcl4.b $(TEST)/testcl4.c

../testcl5.b:	$(TEST)/testcl5.c
	$(CC) -c -o ../testcl5.b $(TEST)/testcl5.c

../testcl6.b:	$(TEST)/testcl6.c
	$(CC) -c -o ../testcl6.b $(TEST)/testcl6.c

../testsc1.b:	$(TEST)/testsc1.c
	$(CC) -c -o ../testsc1.b $(TEST)/testsc1.c

../testsc2.b:	$(TEST)/testsc2.c
	$(CC) -c -o ../testsc2.b $(TEST)/testsc2.c

fbinit.b:	../fbinit.c
	$(CC) -c $(FBCFLAGS) ../fbinit.c

fbinitmac.b:	../fbinitmac.c
	$(CC) -c $(FBCFLAGS) ../fbinitmac.c

fbtext.b:	../fbtext.c
	$(CC) -c $(FBCFLAGS) ../fbtext.c

../drawtests.b:	../drawtests.c
	$(CC) -c -o ../drawtests.b ../drawtests.c

../bustests.b:	../bustests.c
	$(CC) -c -o ../bustests.b ../bustests.c

bpctest.b:	../bpctest.c
	$(CC) -c ../bpctest.c

initall.b:	../initall.c
	$(CC) -c $(FBCFLAGS) ../initall.c

testall.b:	../testall.c
	$(CC) -c $(FBCFLAGS) ../testall.c

testvec.b:	../testvec.c
	$(CC) -c $(FBCFLAGS) ../testvec.c

fifotest.b:	../fifotest.c
	$(CC) -c $(FBCFLAGS) ../fifotest.c

betarams.b:	../../betarams.c
	$(CC) -c $(FBCFLAGS) ../../betarams.c

char.b:	../../char.c
	cc68 -c -o char.b ../../char.c
