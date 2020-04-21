##########################################################################
#									 #
# 		 Copyright (C) 1985, Silicon Graphics, Inc.		 #
#									 #
#  These coded instructions, statements, and computer programs  contain  #
#  unpublished  proprietary  information of Silicon Graphics, Inc., and  #
#  are protected by Federal copyright law.  They  may  not be disclosed  #
#  to  third  parties  or copied or duplicated in any form, in whole or  #
#  in part, without the prior written consent of Silicon Graphics, Inc.  #
#									 #
##########################################################################

include ${ROOT}/usr/include/make/commondefs

#
# If TCP is defined, wsiris will try both IP/TCP and XNS when making
# Ethernet connections.  Define TCP only if your machine has the 
# IP/TCP option installed.
#
# $MACHINE and $SYSTEM must not be used (this only runs on an IRIS anyway)
#

LCINCS  = -I${ROOT}/usr/include/bsd
LCDEFS = -D$(GLHDWR) -DTCP
IDBTAG = -idb "std.sw.unix"
GRPOWN = -g bin -u bin
LCOPTS = -O 

XLDFLAGS = 
SYSLIBS = -lsun  -lbsd -ldbm -lgl2 -lm -lxns -loldmalloc

LIBPRIM = ./lib.prim
STRGEN  = strgen.awk
DISPGEN = dispgen.awk
NAMEGEN = namegen.awk
DOCGEN  = docgen.awk
RET	= ret.c

CSRCS = term.c readhost.c writehost.c hostio.c irisio.c local.c globals.c \
        ttymodes.c dispatch.c special.c pipecmd.c util.c errorm.c ftp.c \
	tklib.c rstcmap.c

SRCS  = $(CSRCS) $(NAMEGEN)

OBJS  = term.o readhost.o writehost.o hostio.o irisio.o local.o globals.o \
        ttymodes.o dispatch.o special.o pipecmd.o util.o errorm.o ftp.o \
        ret.o tklib.o rstcmap.o

HDRS = term.h hostio.h 

ALL  = wsiris

all: $(ALL)

install: all
	${INSTALL} ${IDBTAG} ${GRPOWN} -m 755 -F /bin wsiris 

wsiris: $(OBJS)
	$(CC) $(XLDFLAGS) $(OBJS) -o wsiris $(LDFLAGS) $(SYSLIBS)

dispatch.o: term.h hostio.h dispatch.h
globals.o: term.h
hostio.o: term.h hostio.h
irisio.o: rpc.h term.h hostio.h
pipecmd.o: term.h
readhost.o: term.h hostio.h rpc.h 4010.h
special.o: term.h
term.o: term.h hostio.h
tklib.o: 4010.h term.h hostio.h
ttymodes.o: term.h
util.o: term.h
writehost.o: term.h hostio.h 4010.h

ret.o: $(RET) term.h hostio.h
	: $${machine:?}
	if [ $(machine) = juniper ] ; then \
	    $(CC) -DUNIX -DIP2 $(CFLAGS) -I../term -c $(RET) ; \
	else \
	    $(CC) -DUNIX -DPM2 $(CFLAGS) -I../term -c $(RET) ; \
	fi

dispatch.h: $(LIBPRIM) $(STRGEN) $(DISPGEN) $(NAMEGEN) Makefile
	sed -e 's/:ginit(/:xginit(/g' \
	    -e 's/:gexit(/:xgexit(/g' \
	    -e 's/:gbegin(/:xgbegin(/g' \
	    -e 's/:getpor(/:xgetpor(/g' \
	    -e 's/:getport(/:xgetport(/g' \
	    -e 's/:winopen(/:xwinopen(/g' \
	    -e 's/:winope(/:xwinope(/g' \
	    -e 's/:tpoff(/:xtpoff(/g' \
	    -e 's/:tpon(/:xtpon(/g' \
	    -e 's/:setfastcom(/:xsetfastcom(/g' \
	    -e 's/:setslowcom(/:xsetslowcom(/g' \
	    	$(LIBPRIM) > temp.prim	
	awk -f $(STRGEN) temp.prim > tempstruct.h
	awk -f $(DISPGEN) tempstruct.h > tempdisp.h 
	awk -f $(NAMEGEN) tempstruct.h > tempname.h
	cat tempdisp.h tempstruct.h tempname.h > dispatch.h
	rm -f tempdisp.h tempstruct.h temp.prim tempname.h

tags: $(HDRS) $(CSRCS)
	ctags $(HDRS) $(CSRCS)

lint:
	lint -x $(DEFINES) $(CSRCS) $(RET) $(SYSLIBS) -lc > lint.out

clobber clean:
	rm -f $(ALL) *.[osE] *.lst *.BAK *.CKP .[BC]* dispatch.h \
	      temp*.* LOGFILE.* make.out lint.out

.SUFFIXES: .o .lst .c .s .E

.c.lst:	
	$(CC) -L $(CFLAGS) -c $*.c

.c.s:	
	$(CC) -S $(CFLAGS) -c $*.c

.c.E:	
	$(CC) -E $(CFLAGS) -c $*.c > $@


