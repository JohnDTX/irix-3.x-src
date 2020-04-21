#
#ident	"\%W\%"
#
# Notes:
#
# These files are dot patterns for halftone reproductions used be the drivers.
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
I_FLAGS	=-idb "std.sw.unix"
# configuration files that should be discarded if user has one already
NOUPDATE=-idb "std.sw.unix config(noupdate)"
# configuration files that should override, but save, existing version
UPDATE	=-idb "std.sw.unix config(update)"
SU_FLAGS=-m 04555 -u root -g sys $(I_FLAGS)
LP_FLAGS=-m 755 -u lp -g lp $(UPDATE)
DAT_FLAGS=-m 644 -u lp -g lp $(I_FLAGS)


PUTDIR	= /usr/lib/print/patterns
#
# Targets/Rules
#

default:    all

clean:
	
clobber:	clean

FRC:

#
# Specific Target/Rules follow
#
install:
	$(INSTALL) $(DAT_FLAGS) -F $(PUTDIR) "*.pat"

all:
