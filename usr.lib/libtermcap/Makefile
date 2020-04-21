#
include ${ROOT}/usr/include/make/commondefs

# termcap Makefile
#
# if the header is changed one MUST do "make clobber"!!!
#
# @(#)$Header: /d2/3.7/src/usr.lib/libtermcap/RCS/Makefile,v 1.1 89/03/27 18:40:40 root Exp $
# $Log:	Makefile,v $
# Revision 1.1  89/03/27  18:40:40  root
# Initial check-in for 3.7
# 
# Revision 1.5  87/08/25  22:38:32  andre
# use commondefs more effectively
# 
# Revision 1.4  85/03/15  19:36:00  bob
# Merged Olympus & Jesse.
# 
#
HEADERS	= uparm.h

CFILES	= termcap.c tgoto.c tputs.c

# special termlib flags
LCDEFS	= -DCM_N -DCM_GT -DCM_B -DCM_D -v
LCINCS  = -I${ROOT}/usr/include
IDBTAG  = -idb "std.sw.unix"


#	The member files.
MEMBERS	= \
	$(LIB)(termcap.o)	\
	$(LIB)(tgoto.o)		\
	$(LIB)(tputs.o)		\
	$(NOOP)

L	= /usr/lib
LIB	= libtermcap.a
LIBP	= libtermcap_p.a
LINK	= libtermlib.a
LINKP	= libtermlib_p.a

all:	$(LIB) $(LIBP)

$(LIB):	$(MEMBERS)
	rm -f $(LINK) ; ln $(LIB) $(LINK)
	ranlib $@

$(LIBP):$(MEMBERS)
	$(MAKE) LIB=$(LIBP) LIBP=noop CFLAGS2=-p $(LIBP) LINK=$(LINKP)

install:all
	${INSTALL} -m 644 -g bin -u bin ${IDBTAG} -F /usr/lib "${LIB} ${LIBP}"
	ranlib ${ROOT}/usr/lib/$(LIB) 
	ranlib ${ROOT}/usr/lib/$(LIBP)
	${INSTALL} -ln /usr/lib/${LIB} ${IDBTAG} -F /usr/lib ${LINK}
	${INSTALL} -ln /usr/lib/${LIBP} ${IDBTAG} -F /usr/lib ${LINKP}

clean:
	rm -f *.o
	
clobber:clean
	rm -f $(LIB) $(LIBP) $(LINK) $(LINKP)

.c.o:
	$(CC) $(CFLAGS) -c $< ; $(LD) -r -x $@ \
	  ; mv a.out $@

.c.a:
	$(CC) $(CFLAGS) -c $< ; set +e ; $(LD) -r -x $% \
	  ; test $$? -le 1
	/bin/mv a.out $% ; ar rv $@ $% ; /bin/rm -f $%

.PRECIOUS:$(LIB)
