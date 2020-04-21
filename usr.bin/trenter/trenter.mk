#	@(#)trenter.mk	1.3
ROOT=
INS=	/etc/install -n /usr/bin
INSDIR=	/usr/bin
SPOOL=	/usr/spool/trenter

TRESRC=	trenter.c obj.c util.c

TREOBJ=	trenter.o obj.o util.o

LDFLAGS=-s

TRE=	trenter

all:
	@if pdp11; \
	then \
		$(MAKE) -f trenter.mk CC=pcc CFLAGS="-O -i" $(TRE); \
	else \
		$(MAKE) -f trenter.mk CC=cc CFLAGS="-O" $(TRE); \
	fi

install: all
	$(INS) $(TRE) $(INSDIR)
	@if [ ! -d $(SPOOL) ]; \
	then \
		mkdir $(SPOOL); \
		chown bin $(SPOOL); \
		chmod 777 $(SPOOL); \
		mkdir $(SPOOL)/send; \
		chown bin $(SPOOL)/send; \
		chmod 777 $(SPOOL)/send; \
	fi


$(TRE):		$(TREOBJ)
		$(CC) $(LDFLAGS) $(TREOBJ) -o $(TRE)

.s.o:
		$(CC) $(CFLAGS) -c $<
.c.o:
		$(CC) $(CFLAGS) -c $<

clean:
		-rm -f $(TREOBJ)

clobber:	clean
		-rm -f $(TRE)
