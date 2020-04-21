#	@(#)m4.mk	1.2
TESTDIR = .
YACCRM=-rm
FRC =
INS = /etc/install -n /usr/lbin
INSDIR =
CFLAGS = -O

all: m4

m4:	m4.o m4ext.o m4macs.o m4y.o
	$(CC) $(LDFLAGS) -i -s -o $(TESTDIR)/m4 m4.o m4ext.o m4macs.o m4y.o $$DEST/usr/lib/liby.a

m4.o:	m4.c $(FRC)
m4ext.o: m4ext.c $(FRC)
m4macs.o: m4macs.c $(FRC)
m4y.o:	m4y.y $(FRC)

test:
	rtest $(TESTDIR)/m4

install: all
	$(INS) $(TESTDIR)/m4 $(INSDIR)

clean:
	-rm -f *.o
	$(YACCRM) -f y.tab.c 

clobber: clean
	-rm -f $(TESTDIR)/m4

FRC:
