AR = ar

#
#
include ${ROOT}/usr/include/make/commondefs
FILES =	asin.o atan.o erf.o floor.o fmod.o \
	gamma.o hypot.o jn.o j0.o j1.o pow.o _lpow.o log.o \
	sin.o sqrt.o tan.o tanh.o sinh.o exp.o \
	gftn.o gpasc.o gur.o gtl10.o pruntimes.o

SRCS = asin.c atan.c erf.c floor.c fmod.c \
	gamma.c hypot.c jn.c j0.c j1.c pow.c _lpow.c log.c sin.c \
	sqrt.c tan.c tanh.c sinh.c exp.c \
	gftn.p gftn.s gpasc.p gpasc.s gur.p gur.s gtl10.p gtl10.s \
	pruntimes.c

HDRS = gdcl.h gftn.h gpasc.h gur.h gtl10.h

all:	libm.a 

libm.a: $(FILES)
	rm -f libm.a
	$(AR) rv libm.a `lorder $(FILES) | tsort`
#	$(AR) rv libm.a $(FILES)

install: all
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix" libm.a
	ranlib ${ROOT}/usr/lib/libm.a

.c.o:
	$(CC) $(CFLAGS) -c $<
	set +e;$(LD) -r -x $@;test $$? -le 1
	/bin/mv a.out $@

.c.s:
	$(CC) -c $(CFLAGS) $<

clobber clean:
	rm -f *.o libm.a

gftn.o: gftn.s
	as20 -i -o $*.o $*.s

gpasc.o: gpasc.s
	as20 -i -o $*.o $*.s

gtl10.o: gtl10.s
	as20 -i -o $*.o $*.s

gur.o: gur.s
	as20 -i -o $*.o $*.s

release:
	release $(SRCS) $(HDRS) Makefile

snapshot:
	snapshot $(SRCS) $(HDRS) Makefile

nostrict:
	rcs -U $(SRCS) $(HDRS) Makefile
