#
#
include ${ROOT}/usr/include/make/commondefs
INSCMD	=${INSTALL} -idb "std.sw.unix" 

LCDEFS	= -Zf

SRCS = asin.c atan.c erf.c floor.c fmod.c \
	gamma.c hypot.c jn.c j0.c j1.c pow.c log.c \
	sin.c sqrt.c tan.c tanh.c sinh.c exp.c up_i.c
OBJS = $(SRCS:.c=.o)

# hack to do something only for 2000's
all:	hall$(machine)
hall halljuniper:
halliris:libm.a

install:hinstall$(machine)
hinstall hinstalljuniper:
hinstalliris: all
	${INSCMD} -u bin -g bin -m 644 -src libm.a -F /usr/lib libmsky.a
	ranlib ${ROOT}/usr/lib/libmsky.a
	${INSCMD} -ln /usr/lib/libmsky.a -F /usr/lib libmhw.a

libm.a: $(OBJS)
	rm -f libm.a
	$(AR) r libm.a `lorder $(OBJS) | tsort`
#	$(AR) r libm.a $(OBJS)
	strip libm.a

.c.o:
	$(CCF) $(CFLAGS2) -c $<
	set +e;$(LD) -r -x $@;test $$? -le 1
	/bin/mv a.out $@

.c.s:
	$(CCF) $(CFLAGS2) -S $<

clobber clean:
	rm -f *.o libmsky.a libm.a

release:
	release $(SRCS) Makefile

snapshot:
	snapshot $(SRCS) Makefile

nostrict:
	rcs -U $(SRCS) $(HDRS) Makefile
