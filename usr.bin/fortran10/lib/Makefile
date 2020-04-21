#
#
include ${ROOT}/usr/include/make/commondefs
 

.SUFFIXES:	.asm .o .oh

#
#	HDIR is the directory for hardware binaries
#	SDIR is the directory for software binaries
#
HDIR=../lib.hardware
SDIR=../lib.software

#
#	SVSOBJS are the objects that are used when loading the compiler
#	SGOBJS are the objects that are used

SVSOBJS=\
 	20muinit.o umem.o

SGOBJS=\
	g.umem.o g.20uinit.o  

OBJS= $(SGOBJS) \
	20umisc.o fpk.o nan.o pascheck.o pasdpwr10.o \
	pasmath.o pasmove.o paspwr10.o \
	passcomp.o passet.o\
	pastext.o pastring.o \
	ubuf.o ugotoxy.o uio.o truncjmp.o utexti.o utexto.o p2cstr.o

SOBJS= $(OBJS) \
	add.o cmp.o f13.o mul.o ipk.o pasrio.o div.o mod.o \
	fmax.o dmax.o \
	dadd.o dcmp.o d13.o dmul.o dpk.o dvmd.o dmod.o

HOBJS= $(OBJS) \
	sky.dcmp.oh sky.dpk.oh sky.dvmd.oh sky.f13.oh \
	sky.gmath.oh sky.ipk.oh pasrio.oh d13.oh

.asm.o:
	../bin/asm68k -s $<
	/usr/lib/jlinker10 $*.obj -o$@

.asm.oh:
	../bin/asm68k -s $<
	/usr/lib/jlinker10 $*.obj -o$@

.p.o:
	CC_OPTS="${CC_OPTS} -ZI" $(PC) $(PCFLAGS) -c -ZU $<

.p.oh:
	CC_OPTS="${CC_OPTS} -ZI" $(PC) $(PCFLAGS) -c -ZU -Zf $*.p -o $*.oh

.s.o:
	/bin/as20 -i $*.s -o $*.o

all:	libp10.a ../svslib.o libphw10.a

install:	all
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix mr" libp10.a
	ranlib ${ROOT}/usr/lib/libp10.a
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix mr" libphw10.a
	ranlib ${ROOT}/usr/lib/libphw10.a
	if test $$machine = iris;\
	then \
		${INSTALL} -ln /usr/lib/libp10.a  -F /usr/lib \
		-idb "std.sw.unix mr" libp.a ;\
		${INSTALL} -ln /usr/lib/libphw10.a  -F /usr/lib \
		-idb "std.sw.unix mr" libphw.a ;\
	fi

libp10.a:	$(SOBJS) 
	cp $(SOBJS) $(SDIR) ;\
	rm -f libp10.a;\
	ar cr libp10.a $(SOBJS) ;\
	cp libp10.a $(SDIR)

../svslib.o svslib: $(SVSOBJS)
	cp $(SVSOBJS) $(SDIR) ;\
	rm -f ../svslib.o;\
	ld -r -o ../svslib.o $(SVSOBJS)

libphw10.a:	$(HOBJS) 
	cp $(HOBJS) $(HDIR) ;\
	rm -f libphw10.a;\
	ar cr libphw10.a $(HOBJS) ;\
	cp libphw10.a $(HDIR)

clean:
	rm -f $(HOBJS) $(SOBJS) *.obj *.o *.j ;\
	cd $(HDIR);\
	rm -f *.o *.j *.oh *.a ;\
	cd $(SDIR);\
	rm -f *.o *.j *.oh *.a 

clobber:	clean
	rm -f libp10.a ;\
	rm -f libphw10.a ;\
	rm -f ../svslib.o
