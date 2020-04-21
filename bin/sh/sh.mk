#	@(#)sh.mk	1.4
#	3.0 SID #	1.2
TESTDIR = .
FRC =
INS = /etc/install -n /bin
INSDIR =
CFLAGS = -O 
LDFLAGS = -n -s
OFILES = setbrk.o builtin.o blok.o stak.o cmd.o fault.o main.o word.o string.o\
name.o args.o xec.o service.o error.o io.o print.o macro.o expand.o\
ctype.o msg.o test.o defs.o

all: sh

sh: $(SFILES) $(OFILES)
	$(CC) $(LDFLAGS) $(SFILES) $(OFILES) -o $(TESTDIR)/sh

$(OFILES):	defs.h $(FRC)

ctype.o:	ctype.h
	 	sh ./:fix ctype

xec.o:		xec.c
	set +e; if u370;\
	then \
	$(CC) $(CFLAGS) -b1,0 -c xec.c;\
	else \
	$(CC) $(CFLAGS) -c xec.c;\
	fi
blok.o fault.o main.o stak.o:	brkincr.h

msg.o:		msg.c $(FRC)
	  	sh ./:fix msg

test:
	  rtest $(TESTDIR)/sh

install:  all
	  $(INS) -o $(TESTDIR)/sh $(INSDIR)
	  mv /bin/rsh /bin/OLDrsh
	  ln /bin/sh /bin/rsh
	  chown bin /bin/sh
	   sync

clean:
	  -rm -f *.o

clobber:  clean
	  -rm -f $(TESTDIR)/sh
	  -rm -f /bin/OLDrsh

FRC:
