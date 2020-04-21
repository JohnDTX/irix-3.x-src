#
#
include ${ROOT}/usr/include/make/commondefs


LCDEFS	= -DNMAIL

# Libraries for Yellow Pages support
LLDLIBS	= -lsun -lbsd -ldbm

all:	lpr lpd

install:all
	${INSTALL} -u root -g sys -m 4755 -F /bin \
	-idb "root upd" lpr
	${INSTALL} -u root -g sys -m 4755 -F /usr/lib \
	-idb "shared upd usr" lpd

clean:
	rm -f *.o

clobber:clean
	rm -f lpr vpr lpd lpd2 vpd vpd.pr

dingaling:lpr vpr lpd lpd2 vpd vpd.pr
spoolers:lpr vpr
daemons:lpd lpd2 tpd tpd.pr vpd vpd.pr

# lpr.c includes spool.c
lpr:	lpr.c spool.c
	$(CCF) -o $@ lpr.c $(LDFLAGS)

lpd:	lpd.o banner.o
	$(CCF) -o $@ lpd.o banner.o $(LDFLAGS)

# lpd.c includes daemon.c which includes daemon0.c
lpd.o:	lpd.c daemon.c daemon0.c

lpd2:	lpd2.o banner.o
	$(CCF) -o $@ lpd2.o banner.o $(LDFLAGS)

lpd2.o:	lpd2.c daemon.c daemon0.c

# vpr is just lpr with some -D things
# lpr.c includes spool.c
vpr:	lpr.o spool.o
	$(CCF) -DSPDIR='"/usr/spool/vpd"' -DDAEMON='"/usr/lib/vpd"' \
	  -DVPR -o $@ lpr.c $(LDFLAGS)

tpd:	vlpd.o daemon.o daemon0.o
	$(CCF) -DSPDIR='"/usr/spool/tpd"' -DLOCK='"/usr/spool/tpd/lock"' \
	  -DPRT='"/usr/lib/tpd.pr"' -o $@ vlpd.c $(LDFLAGS)

tpd.pr:	prt1200.o banner.o
	$(CCF) -o tpd.pr prt1200.o banner.o $(LDFLAGS)

vpd:	vlpd.o daemon.o daemon0.o
	$(CCF) -DSPDIR='"/usr/spool/vpd"' -DLOCK='"/usr/spool/vpd/lock"' \
	  -DPRT='"/usr/lib/vpd.pr"' -o vpd vlpd.o $(LDFLAGS)

vpd.pr:	vpd.pr.o banner.o
	$(CCF) -o vpd.pr vpd.pr.o banner.o $(LDFLAGS)
