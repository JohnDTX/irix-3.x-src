# Makefile for pm2 proms

#
#
include ${ROOT}/usr/include/make/commondefs
#
# .do files have text moved to data;
# they are used to balance PROM0 (text) and PROM1 (data)
#
.SUFFIXES: .c .do .no .o .r .s


#----- tool names -----
LD =		./segld
RANLIB =	ranlib
NM =		nm
STRIP =		strip
REV =		rev
MAKEDBOOT =	./makedboot
NPROM =		./nprom


#----- parameters -----
LCINCS  =	-I./include
LCDEFS  =	-DMC68000 -DPM2 -DQPROM
LCOPTS  =	-O 
MAKEFILE =	Makefile
RCSDIR =	RCS
CIFLAGS =	-N$$RCSVERSION -q -m.
PROMTYPE =	AM27128A
#PROMTYPE =	AM27128
#PROMTYPE =	i27128
GLXVERSION =	V3.9 February 12, 1986


#----- peculiar defines -----
MBIOBASE =	F70000
PROMORG =	F80000
DATAORG =	F90000
BSSORG =	F6F800
# TESTORG must be a multiple of 0x10000
TESTORG =	 40000
HITESTORG =	140000
RHTESTORG =	130000
PM2LIB =	libpm2prom.a
GLXLIB =	libglxprom.a
MONLIB =	libmon.a
STANDLIB =	stand/libstand.a
STARTUP =	pm2start.o
GLXLIBS =	$(MONLIB) $(GLXLIB) $(PM2LIB)
SRCS =		pm2start.s promcvt.c qprom.c
GLXOBJS =	$(STARTUP) versglx.do qprom.do
RHGLXOBJS =	$(STARTUP) versglx.do rhqprom.do rhdiskboot.do
BOOST =		./boost


#----- default rules -----
default: all
	:
.c.o:
	$(CC) -c $(CFLAGS) $*.c
.c.do:
	$(CC) -S $(CFLAGS) $*.c 
	mv $*.s temp
	sed 's/\.text/.data/' <temp > $*.s
	$(AS) $*.s -o $*.o
	-rm -f $*.s temp
	mv $*.o $*.do
.c.no:
	$(CC) -c $(DEFINES) $(INCLUDES) $*.c
	mv $*.o $*.no
.c.s:
	$(CC) -S $(CFLAGS) $*.c
.s.o:	
	$(AS) $*.s -o $*.o
.s.do:
	$(AS) $*.s -o $*.o
	mv $*.o $*.do

pm2start:
	oas pm2start.s -o pm2start.o

#----- required targets -----
all:	pm2start glx
	:
new:	clean all
	:
clobber: clean
	-rm -f *.a *.r versglx.c
clean:
	rm -f *.o *.do *.no *.r
	rm -f a.* *.nm
	rm -f promcvt promsplit nprom prom.out
	cd gl; make $@
	cd gen; make $@
	cd dev; make $@
	cd pm2; make $@
	cd mon; make $@
	cd stand; make $@

install: ${BOOST} pm2start libpm2prom libglxprom hiqprom.r rhqprom.r
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
		-idb "std.sw.unix mr" "libpm2prom.a libglxprom.a "
	ranlib ${ROOT}/usr/lib/libpm2prom.a 
	ranlib ${ROOT}/usr/lib/libglxprom.a
	${INSTALL} -u root -g sys -m 644 -F /stand \
		-idb "std.sw.unix mr" "hiqprom.r rhqprom.r "

release:
snapshot:
	cd include; make $@
	cd gl; make $@
	cd gen; make $@
	cd dev; make $@
	cd pm2; make $@
	cd mon; make $@
	$@ $(SRCS) $(MAKEFILE)


#----- exceptional rules -----
versglx.do: $(MAKEFILE)
	echo 'char *promversion = "'$(GLXVERSION)'";'>versglx.c
	echo 'char *compdate ="'`date`'";'>>versglx.c
	$(CC) -c versglx.c
	mv versglx.o versglx.do
#	touch versglx.c


#----- peculiar targets -----
glx: libglxprom libpm2prom libmon qprom.r hiqprom.r rhqprom.r prom
	:
libs: libglxprom libpm2prom libmon libstand
	:
libglxprom $(GLXLIB): 
	-rm -f $(GLXLIB)
	cd gl; make
	ln $(GLXLIB) stand
	$(RANLIB) $(GLXLIB)
libpm2prom $(PM2LIB):
	-rm -f $(PM2LIB)
	cd gen; make
	cd dev; make
	cd pm2; make
	ln $(PM2LIB) stand
	$(RANLIB) $(PM2LIB)
libmon $(MONLIB):
	-rm -f $(MONLIB)
	cd mon; make
	$(RANLIB) $(MONLIB)
libstand $(STANDLIB):
	cd stand; make
	ln $(STANDLIB) .
	$(RANLIB) $(STANDLIB)

qprom.r: $(GLXOBJS) $(GLXLIBS) $(BOOST) $(MAKEFILE)
# download-able prom program - for debugging
# entered at the reset vector - hence the adb step
	$(RANLIB) $(PM2LIB)
	$(LD) -o $@ -LT $(TESTORG) -LC $(BSSORG) \
		$(GLXOBJS) $(GLXLIBS) -lc
	size $@
	$(MAKEDBOOT) $(BOOST) $@ ,$@ ; mv ,$@ $@

hiqprom.r: $(GLXOBJS) $(GLXLIBS) $(BOOST) $(MAKEFILE)
# download-able prom program - for debugging
# entered at the reset vector - hence the adb step
	$(RANLIB) $(PM2LIB)
	$(LD) -o $@ -LT $(HITESTORG) -LC $(BSSORG) \
		$(GLXOBJS) $(GLXLIBS) -lc
	size $@
	$(MAKEDBOOT) $(BOOST) $@ ,$@ ; mv ,$@ $@

rhqprom.r: $(RHGLXOBJS) $(GLXLIBS) $(BOOST) $(MAKEFILE)
# download-able prom program - with roothack
# entered at the reset vector - hence the adb step
	$(RANLIB) $(PM2LIB)
	$(LD) -o $@ -LT $(RHTESTORG) -LC $(BSSORG) \
		$(RHGLXOBJS) $(GLXLIBS) -lc
	size $@
	$(MAKEDBOOT) $(BOOST) $@ ,$@ ; mv ,$@ $@
rhdiskboot.do: dev/diskboot.c
	$(CC) -S -DROOTHACK $(CFLAGS) dev/diskboot.c
	mv diskboot.s temp
	sed 's/\.text/.data/' <temp > diskboot.s
	$(CC) -c diskboot.s
	-rm -f diskboot.s temp
	mv diskboot.o $@
rhqprom.do: qprom.c
	$(CC) -S -DROOTHACK $(CFLAGS) qprom.c
	mv qprom.s temp
	sed 's/\.text/.data/' <temp > qprom.s
	$(CC) -c qprom.s
	-rm -f qprom.s temp
	mv qprom.o $@

prom: $(GLXOBJS) $(GLXLIBS) $(MAKEFILE) promcvt
# prom image - to be loaded into the prom burner's ram
# promcvt removes the header and pads the text segment
	$(RANLIB) $(PM2LIB)
	$(LD) -o $@.out -LT $(PROMORG) -LD $(DATAORG) -LC $(BSSORG) \
		$(GLXOBJS) $(GLXLIBS) -lc
	size $@.out
	$(NM) -ng $@.out > $@.nm
	$(STRIP) $@.out
#	$(REV) $@.out $@.out.r
	-rm -f a.$@
	promcvt -o a.$@ -b 8000 -d $(DATAORG) $@.out -v
	@echo "Use 'make burn' to burn the proms."

burn: nprom
	$(NPROM) -t$(PROMTYPE) -S2 -v <a.prom

promcvt: promcvt.do
	$(CC) -o $@ promcvt.do

promsplit: promsplit.do
	$(CC) -o $@ promsplit.do

$(BOOST): boost.o $(MAKEFILE)
	$(LD) -o $@ boost.o
