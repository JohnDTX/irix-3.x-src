#	@(#)cflow.mk	1.3
#	3.0 SID #	1.2
TESTDIR = .
CFLAGS = -O
FILES = Makefile README cflow.sh dag.c lpfx.c nmf.c flip.c
ALL = dag lpfx nmf flip
LINT = ../lint
MIP = ../cc/pcc/mip
BIN = /usr/bin
LIB = /usr/lib

all:	$(ALL)

dag:	dag.c
	$(CC) $(CFLAGS) -i dag.c -o $(TESTDIR)/dag

lpfx:	lpfx.c $(LINT)/lerror.h $(LINT)/lmanifest $(LINT)/lpass2.h \
		$(MIP)/manifest
	$(CC) $(CFLAGS) -I$(LINT) -I$(MIP) -i lpfx.c -o $(TESTDIR)/lpfx

nmf:	nmf.c
	$(CC) $(CFLAGS) nmf.c -o $(TESTDIR)/nmf

flip:	flip.c
	$(CC) $(CFLAGS) flip.c -o $(TESTDIR)/flip

install:	$(ALL)
	cp cflow.sh $(BIN)/cflow
	cp dag $(LIB)/dag
	cp lpfx $(LIB)/lpfx
	cp nmf $(LIB)/nmf
	cp flip $(LIB)/flip
	chgrp bin $(BIN)/cflow $(LIB)/dag $(LIB)/lpfx $(LIB)/nmf $(LIB)/flip
	chown bin $(BIN)/cflow $(LIB)/dag $(LIB)/lpfx $(LIB)/nmf $(LIB)/flip
	chmod 755 $(BIN)/cflow $(LIB)/dag $(LIB)/lpfx $(LIB)/nmf $(LIB)/flip

clean:
	-rm -f *.o *.out core

clobber:	clean
	-rm -f $(ALL)
