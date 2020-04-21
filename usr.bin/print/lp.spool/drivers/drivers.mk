#
#ident	"\%W\%"
#
# Notes:
#

#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs

#
# Compile Time Options
#

#
# Local Definitions
#
#
#
I_FLAGS	=-idb "std.sw.unix"
# configuration files that should be discarded if user has one already
NOUPDATE=-idb "std.sw.unix config(noupdate)"
# configuration files that should override, but save, existing version
UPDATE	=-idb "std.sw.unix config(update)"
SU_FLAGS=-m 04555 -u root -g sys $(I_FLAGS)
LP_FLAGS=-m 755 -u lp -g lp $(I_FLAGS)
DAT_FLAGS=-m 644 -u bin -g bin $(UPDATE)

ALL = mprint pprint tprint vprint sprint
#CFLAGS = -O  -I$(DEST)/usr/include/gl2
#LIBS = printer.o $(ROOT)/usr/lib/libimage.a
#LLDFLAGS = printer.o $(ROOT)/usr/lib/libimage.a -lbsd

LLDLIBS = -limage -lbsd
#
# Targets/Rules
#

default:    all

clean:
	rm -f *.o *.a

clobber:	clean
	rm -f $(ALL)

FRC:

#
# Specific Target/Rules follow
#
all: $(ALL)

install:	all
	$(INSTALL) $(LP_FLAGS) -F /usr/lib/print "$(ALL)"

sprint cprint mprint pprint: $$@.o printer.o
	$(CCF) $@.o printer.o $(LDFLAGS) -o $@ 

tprint: tprint.o tsubr.o printer.o
	$(CCF) $@.o tsubr.o printer.o $(LDFLAGS)-o $@

vprint: vprint.o vsubr.o printer.o
	$(CCF) $@.o vsubr.o printer.o $(LDFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $(CFLAGS) $<

.c.s:
	$(CC) $(CFLAGS) -c -S $(CFLAGS) $<
