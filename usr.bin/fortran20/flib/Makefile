
#
include ${ROOT}/usr/include/make/commondefs
.SUFFIXES:	.asm .o .obj .oh

HDIR=../flib.hardware
SDIR=../flib.software

SWLIB = libf20.a
HWLIB = libfhw20.a

HOBJS=flibinit.oh flibrec.oh flibwd.oh flibwr.oh fliblw.oh \
	cai.oh cmplxf.oh flibback.oh\
	flibchain.oh flibfmt.oh flibfr.oh\
	flibfrd.oh flibfrr.oh flibfw.oh \
	flibfwd.oh flibfwr.oh \
	flibinq.oh fliblr.oh \
	fliblwd.oh fliblwr.oh flibopen.oh\
	flibstop.oh flibu.oh \
	flibusr.oh flibusr2.oh \
	ftnchar.oh ufio.oh fmisc_hw.oh \
	userexit.oh

SOBJS=flibinit.o flibrec.o flibwd.o flibwr.o fliblw.o \
	cai.o cmplxf.o flibback.o\
	flibchain.o flibfmt.o flibfr.o\
	flibfrd.o flibfrr.o flibfw.o \
	flibfwd.o flibfwr.o \
	flibinq.o fliblr.o \
	fliblwd.o fliblwr.o flibopen.o\
	flibstop.o flibu.o \
	flibusr.o flibusr2.o \
	ftnchar.o ufio.o fmisc_sw.o \
	userexit.o

all:	../$(SWLIB) ../$(HWLIB) 

.asm.oh:
	../bin/asm68k -s $<
	/usr/lib/jlinker20 $*.obj -o$@

.asm.o:
	../bin/asm68k -s $<
	/usr/lib/jlinker20 $*.obj -o$@

flibfrd.oh:flibfrd.p
	CC_OPTS="${CC_OPTS} -ZJ" $(PC) $(PCFLAGS) -ZU -c -ZD $*.p -o $*.oh

.p.oh:
	CC_OPTS="${CC_OPTS} -ZJ" $(PC) $(PCFLAGS) -ZU -c -Zf $*.p -o $*.oh

.p.o:
	CC_OPTS="${CC_OPTS} -ZJ" $(PC) $(PCFLAGS) -ZU -c $*.p

.f.oh:
	CC_OPTS="${CC_OPTS} -ZJ" $(F77) $(F77FLAGS) -c -Zf -ZF+e $*.f -o $*.oh

.f.o:
	CC_OPTS="${CC_OPTS} -ZJ" $(F77) $(F77FLAGS) -c -ZF+e $*.f

flibwr.oh: flibwr.o
	cp flibwr.o flibwr.oh
	
flibwd.oh: flibwd.o
	cp flibwd.o flibwd.oh
	
cmplxf.oh:
	CC_OPTS="${CC_OPTS} -ZJ" $(F77) $(F77FLAGS) -c -ZF+e -Zf $*.f -o $*.oh

cmplxf.o:
	CC_OPTS="${CC_OPTS} -ZJ" $(F77) $(F77FLAGS) -c -ZF+e $*.f 

#.f.o:
#	f77 -c $(FFLAGS) $<

install:	all
	${INSTALL} -src "../${SWLIB}" -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix mr" "${SWLIB}"
	ranlib ${ROOT}/usr/lib/${SWLIB}
	${INSTALL} -src "../${HWLIB}" -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix mr" "${HWLIB}" 
	ranlib ${ROOT}/usr/lib/${HWLIB}
	if test "$$machine" = juniper;\
	then \
		${INSTALL} -ln /usr/lib/${SWLIB} -F /usr/lib \
		-idb "std.sw.unix mr" libf.a ;\
		${INSTALL} -ln /usr/lib/${HWLIB} -F /usr/lib \
		-idb "std.sw.unix mr" libfhw.a ;\
	fi


../$(SWLIB):	$(SOBJS)
	cp $(SOBJS) $(SDIR);\
	cd $(SDIR) ;\
	rm -f ../$(SWLIB);\
	ar rv ../$(SWLIB) $(SOBJS);\
	cd ../flib;\
	rm -f *.obj

../$(HWLIB):	$(HOBJS)
	for i in $(HOBJS); \
	do\
		cp $$i $(HDIR)/`basename $$i .oh`.o;\
	done
	cd $(HDIR) ;\
	rm -f ../$(HWLIB);\
	ar rv ../$(HWLIB) *.o;\
	cd ../flib;\
	rm -f *.obj

clean:
	rm -f $(HOBJS) $(SOBJS) *.obj *.j *.o *.a *.oh ;\
	cd $(HDIR);\
	rm -f *.o *.obj *.j *.oh *.a ;\
	cd $(SDIR) ;\
	rm -f *.o *.obj *.j *.oh *.a

clobber:	clean
	rm -f ../$(SWLIB) ;\
	rm -f ../$(HWLIB)
