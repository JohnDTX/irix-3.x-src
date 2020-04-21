#/*	sa.mk 1.2 of 4/7/82	*/
# how to use this makefile
# to make sure all files  are up to date: make -f sa.mk all
#
# to force recompilation of all files: make -f sa.mk all FRC=FRC 
#
# to test new executables before installing in 
# /usr/lib/sa:	make -f sa.mk testbi
#
# to install just one file:	make -f sa.mk safile "INS=/etc/install"
#
# The sadc and sadp modules must be able to read /dev/kmem,
# which standardly has restricted read permission.
# They must have set-group-ID mode
# and have the same group as /dev/kmem.
# The chmod and chgrp commmands below ensure this.
#
TESTDIR = .
FRC =
INS = :
INSDIR = $$DEST/usr/lib/sa
CFLAGS = -O 
LDFLAGS = -s
FFLAG =
 

all:	sadc sar sa1 sa2 timex sag sadp


sadc:: sadc.c sa.h 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/sadc sadc.c 
sadc::
	$(INS) -o -n $(INSDIR) $(TESTDIR)/sadc $(INSDIR)
	-chmod 2775 $(INSDIR)/sadc
	-chown bin $(INSDIR)/sadc
	-chgrp sys $(INSDIR)/sadc
sar:: sar.c sa.h
	$(CC) $(FFLAG) $(CFLAGS)  -o $(TESTDIR)/sar sar.c
sar::
	$(INS) -n /usr/bin $(TESTDIR)/sar 
sa2:: sa2.sh
	cp sa2.sh sa2
sa2::
	$(INS) -n $(INSDIR) $(TESTDIR)/sa2 $(INSDIR)
sa1:: sa1.sh
	cp sa1.sh sa1
sa1::
	$(INS) -n $(INSDIR) $(TESTDIR)/sa1 $(INSDIR)
 
timex::	timex.c sa.h 
	$(CC) $(FFLAG) $(CFLAGS)  -o $(TESTDIR)/timex timex.c 
timex::
	$(INS) -n /usr/bin $(TESTDIR)/timex
sag::	saga.o sagb.o
	$(CC) $(FFLAG) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/sag saga.o sagb.o 
sag::
	$(INS) -n /usr/bin $(TESTDIR)/sag
saga.o:	saga.c saghdr.h
	$(CC) -c $(CFLAGS) saga.c
sagb.o:	sagb.c saghdr.h
	$(CC) -c $(CFLAGS) sagb.c
sadp:: sadp.c 
	$(CC) $(FFLAG) $(CFLAGS)  -o $(TESTDIR)/sadp sadp.c
sadp::
	$(INS) -n /usr/bin $(TESTDIR)/sadp 
	-chmod 2775 /usr/bin/sadp
	-chown bin /usr/bin/sadp
	-chgrp sys /usr/bin/sadp
test:		testai

testbi:		#test for before installing
	sh  $(TESTDIR)/runtest new /usr/src/cmd/sa

testai:		#test for after install
	sh $(TESTDIR)/runtest new

install:
	make -f sa.mk all FFLAG=$(FFLAG) "INS=$$DEST/etc/install" INSDIR=$(INSDIR)

clean:
	-rm -f *.o
 
clobber:	clean
		-rm -f sadc sar sa1 sa2 sag timex sadp

FRC:
