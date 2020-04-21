#
#
include ${ROOT}/usr/include/make/commondefs

ALL = mprint pprint sprint tprint vprint #cprint
CFLAGS = -O  -I./ -I$(DEST)/usr/include/gl2
LIBS = printer.o $(DEST)/usr/lib/libimage.a

all: $(ALL)

clean:
	rm -f *.o *.a $(ALL)
clobber: clean

install: all
	${INSTALL} -F /usr/lib/print  \
	-idb "std.sw.unix mr" "${ALL}"

cprint: cprint.o printer.o
	cc cprint.o -o cprint $(LIBS)

mprint: mprint.o printer.o
	cc mprint.o -o mprint $(LIBS)

pprint: pprint.o printer.o
	cc pprint.o -o pprint $(LIBS) 

sprint: sprint.o printer.o
	cc sprint.o -o sprint $(LIBS) 

tprint: tprint.o tsubr.o printer.o
	cc tprint.o tsubr.o -o tprint $(LIBS) 

vprint: vprint.o vsubr.o printer.o
	cc vprint.o vsubr.o -o vprint $(LIBS) 

.c.o:
	cc -c $(CFLAGS) $<

.c.s:
	cc -c -S $(CFLAGS) $<
