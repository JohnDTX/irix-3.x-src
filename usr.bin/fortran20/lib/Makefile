#
#
include ${ROOT}/usr/include/make/commondefs
.SUFFIXES:	.s .asm .o .oh
LPCOPTS = -ZR /d/root

HDIR=../lib.hardware
SDIR=../lib.software

SWLIB = libp20.a
HWLIB = libphw20.a
SVSLIB = svslib.o

SVSOBJS=\
 	20muinit.o umem.o

SOBJS=\
	pasdpwr10.o paspwr10.o add.o cmp.o dadd.o dcmp.o\
	dmax.o fmax.o \
	div.o dmul.o dpk.o dvmd.o dmod.o fpk.o ipk.o mod.o mul.o\
	pasrio.o d13.o f13.o\
	ubuf.o ugotoxy.o uio.o truncjmp.o g.umem.o 20umisc.o utexti.o utexto.o\
	g.20uinit.o pascheck.o pasmath.o pasmove.o passcomp.o passet.o\
	pastext.o pastring.o nan.o p2cstr.o

HOBJS=\
	junpwr10.oh junflt.oh jundbl.oh juninit.oh jundmod.oh junfmod.oh \
	pasrio.oh d13h.oh f13h.oh f13ph.oh\
	ubuf.oh ugotoxy.oh uio.oh truncjmp.oh \
	g.umem.oh 20umisc.oh utexti.oh utexto.oh\
	g.20uinit.oh pascheck.oh pasmath.oh pasmove.oh passcomp.oh passet.oh\
	pastext.oh pastring.oh nan.oh p2cstr.o divide.oh

all:	../$(SWLIB) ../$(HWLIB) svslib ../swdiv.o

.asm.o:
	../bin/asm68k -s $<
	/usr/lib/jlinker20 $*.obj -o$@

.asm.oh:
	../bin/asm68k -s $<
	/usr/lib/jlinker20 $*.obj -o$@

.p.o:
	CC_OPTS="${CC_OPTS} -ZJ" $(PC) $(PCFLAGS) -c $<

.p.oh:
	CC_OPTS="${CC_OPTS} -ZJ" $(PC) $(PCFLAGS) -c -Zf $< -o $*.oh

.s.oh:
	CC_OPTS="${CC_OPTS} -ZJ" $(PC) $(PCFLAGS) -c -Zf $< -o $*.oh

.s.o:
	CC_OPTS="${CC_OPTS} -ZJ" $(PC) $(PCFLAGS) -c $< 

pasrio.oh: pasrio.o
	cp pasrio.o pasrio.oh

install:	all
	${INSTALL} -src ../${SWLIB} -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix mr" "${SWLIB}"
	ranlib ${ROOT}/usr/lib/${SWLIB}
	${INSTALL} -src ../${SWLIB} -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix mr" "${HWLIB}"
	ranlib ${ROOT}/usr/lib/${HWLIB}
	if test $$machine = juniper;\
	then \
		${INSTALL} -ln /usr/lib/${SWLIB} -F /usr/lib \
		-idb "std.sw.unix mr" libp.a ;\
		${INSTALL} -ln /usr/lib/${HWLIB} -F /usr/lib \
		-idb "std.sw.unix mr" libphw.a ;\
		${INSTALL} -src ../swdiv.o -m 644 -u bin -g bin \
		-F /usr/lib -idb "std.sw.unix mr" swdiv.o ;\
	fi

../$(SWLIB):	$(SOBJS)
	cp $(SOBJS) ../lib.software ;\
	cd $(SDIR) ;\
	rm -f ../$(SWLIB);\
	ar rv ../$(SWLIB) $(SOBJS)

../$(HWLIB):	$(HOBJS)
	for i in $(HOBJS); \
	do\
		cp $$i $(HDIR)/`basename $$i .oh`.o;\
	done
	cd $(HDIR) ;\
	rm -f ../$(HWLIB);\
	ar rv ../$(HWLIB) *.o

svslib: ../$(SVSLIB)

../$(SVSLIB): $(SVSOBJS)
	cp $(SVSOBJS) $(SDIR) ;\
	rm -f ../$(SVSLIB);\
	ld -r -o ../$(SVSLIB) $(SVSOBJS)

d13h.oh:	d13h.s
	/lib/cpp > d13h.cpp d13h.s -P -I../include
	/bin/as20 -o d13h.oh d13h.cpp

divide.oh:	divide.s
	/lib/cpp > divide.cpp divide.s -P -I../include
	/bin/as20 -o divide.oh divide.cpp

f13h.oh:	f13h.s
	/lib/cpp > f13h.cpp f13h.s  -I../include
	/bin/as20 -o f13h.oh f13h.cpp

../swdiv.o:	swdiv.p
	CC_OPTS="${CC_OPTS} -ZJ" $(PC) $(PCFLAGS) -c -Zf -ZD swdiv.p
	cp swdiv.o ..


dbg.o:	dbg.s
	/lib/cpp > dbg.cpp dbg.s  -I../include
	/bin/as20 -o dbg.o dbg.cpp

clean:
	rm -f *.o *.obj *.j *.oh *.a ;\
	cd $(SDIR) ;\
	rm -f *.o *.obj *.j *.oh *.a;\
	cd $(HDIR) ;\
	rm -f *.o *.obj *.j *.oh *.a
	rm -f d13h.cpp
	rm -f f13h.cpp
	rm -f divide.cpp

clobber:	clean
	rm -f ../$(SWLIB) ;\
	rm -f ../$(HWLIB) ;\
	rm -f ../$(SVSLIB)
