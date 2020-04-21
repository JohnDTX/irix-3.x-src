#
#ident	"$Header: "
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
LIB=	libidb.a
OBJS=	attr.o buffsize.o expr.o mapid.o memory.o paths.o read.o sopen.o \
	type.o write.o toblocks.o
LCINCS=	-I../include

#
# Targets/Rules
#

default: $(LIB)

clean:
	rm -f $(OBJS)

clobber:	clean
	rm -f $(LIB)

FRC:

#
# Specific Target/Rules follow
#
install: default

$(LIB): $(OBJS)
	$(AR) cr $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

$(OBJS): ../include/idb.h
