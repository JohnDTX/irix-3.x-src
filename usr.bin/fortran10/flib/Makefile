#
#
include ${ROOT}/usr/include/make/commondefs


.SUFFIXES:	.asm .o .obj .oh

OBJS=flibinit.o flibrec.o flibwd.o flibwr.o fliblw.o \
	cai.o cmplxf.o flibback.o\
	flibchain.o flibfmt.o flibfr.o\
	flibfrd.o flibfrr.o flibfw.o \
	flibfwd.o flibfwr.o \
	flibinq.o fliblr.o \
	fliblwd.o fliblwr.o flibopen.o\
	flibstop.o flibu.o \
	flibusr.o flibusr2.o \
	ftnchar.o ufio.o ftnmisc.o \
	userexit.o

HOBJS=flibinit.oh flibrec.oh flibwd.oh flibwr.oh fliblw.oh \
	cai.o cmplxf.oh flibback.oh\
	flibchain.oh flibfmt.oh flibfr.oh\
	flibfrd.oh flibfrr.oh flibfw.oh \
	flibfwd.oh flibfwr.oh \
	flibinq.oh fliblr.oh \
	fliblwd.oh fliblwr.oh flibopen.oh\
	flibstop.oh flibu.oh \
	flibusr.oh flibusr2.oh \
	ftnchar.o ufio.o ftnmisc.o \
	userexit.oh

all: libf10.a libfhw10.a

.asm.o:
	../bin/asm68k -s $<
	/usr/lib/jlinker10 $*.obj -o$@

.p.o:
	CC_OPTS="${CC_OPTS} -ZI" $(PC) $(PCFLAGS) -ZU -c $<

.f.o:
	CC_OPTS="${CC_OPTS} -ZI" $(F77) $(F77FLAGS) -c $*.f

.p.oh:
	CC_OPTS="${CC_OPTS} -ZI" $(PC) $(PCFLAGS) -c -ZU -Zf $*.p -o $*.oh

.f.oh:
	CC_OPTS="${CC_OPTS} -ZI" $(F77) $(F77FLAGS) -c -Zf $*.f -o $*.oh

cmplxf.oh:
	CC_OPTS="${CC_OPTS} -ZI" $(F77) $(F77FLAGS) -c -ZF+e -Zf $*.f -o $*.oh

cmplxf.o:
	CC_OPTS="${CC_OPTS} -ZI" $(F77) $(F77FLAGS) -c -ZF+e $*.f 

install:	all
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix mr" "libf10.a libfhw10.a"
	ranlib ${ROOT}/usr/lib/libf10.a
	ranlib ${ROOT}/usr/lib/libfhw10.a
	if test $$machine = iris;\
	then \
		${INSTALL} -ln /usr/lib/libf10.a -F /usr/lib \
		-idb "std.sw.unix mr" libf.a ;\
		${INSTALL} -ln /usr/lib/libfhw10.a -F /usr/lib \
		-idb "std.sw.unix mr" libfhw.a ;\
	fi


libf10.a:	$(OBJS)
	rm -f libf10.a
	ar cr libf10.a $(OBJS)

libfhw10.a:	$(HOBJS)
	rm -f libfhw10.a
	ar cr libfhw10.a $(HOBJS)


clean:
	rm -f $(OBJS) $(HOBJS) *.obj *.o *.j ;

clobber:	clean
	rm -f libf10.a ;\
	rm -f libfhw10.a
