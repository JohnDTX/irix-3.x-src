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
LIB=	libinst.a

OBJS=	abspath.o \
	breakpath.o \
	breakword.o \
	byteio.o \
	cat.o \
	chksum.o \
	debug.o \
	derivedev.o \
	devnm.o \
	envsub.o \
	filematch.o \
	fluff.o \
	getspec.o \
	help.o \
	hist.o \
	io.o \
	machname.o \
	mkfs.o \
	mountfs.o \
	parsespec.o \
	prodread.o \
	prodwrite.o \
	rio.o \
	run.o \
	runline.o \
	runpipe.o \
	scan.o \
	split.o \
	umountfs.o \
	vline.o \
	waitpipe.o \
	words.o \
	xrsh.o

LCINCS=	-I../include -I$(ROOT)/usr/include/bsd

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
