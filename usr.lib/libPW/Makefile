
#
#
include ${ROOT}/usr/include/make/commondefs
# makefile for libPW

# $Source: /d2/3.7/src/usr.lib/libPW/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 18:32:46 $

LORDER	= lorder
RANLIB	= ranlib
LIB	= libPW.a
C       = m68000

#	The member files.
MEMBERS	= \
	$(LIB)(abspath.o)	\
	$(LIB)(anystr.o)	\
	$(LIB)(bal.o)		\
	$(LIB)(curdir.o)	\
	$(LIB)(fdfopen.o)	\
	$(LIB)(giveup.o)	\
	$(LIB)(imatch.o)	\
	$(LIB)(index.o)		\
	$(LIB)(lockit.o)	\
	$(LIB)(logname.o)	\
	$(LIB)(move$C.o)	\
	$(LIB)(patoi.o)		\
	$(LIB)(patol.o)		\
	$(LIB)(regcmp.o)	\
	$(LIB)(regex.o)		\
	$(LIB)(rename.o)	\
	$(LIB)(repeat.o)	\
	$(LIB)(repl.o)		\
	$(LIB)(satoi.o)		\
	$(LIB)(setsig.o)	\
	$(LIB)(sname.o)		\
	$(LIB)(strend.o)	\
	$(LIB)(substr.o)	\
	$(LIB)(trnslat.o)	\
	$(LIB)(userdir.o)	\
	$(LIB)(username.o)	\
	$(LIB)(verify.o)	\
	$(LIB)(any.o)		\
	$(LIB)(xalloc.o)	\
	$(LIB)(xcreat.o)	\
	$(LIB)(xlink.o)		\
	$(LIB)(xopen.o)		\
	$(LIB)(xpipe.o)		\
	$(LIB)(xunlink.o)	\
	$(LIB)(xwrite.o)	\
	$(LIB)(xmsg.o)		\
	$(LIB)(alloca$C.o)	\
	$(LIB)(cat.o)		\
	$(LIB)(dname.o)		\
	$(LIB)(fatal.o)		\
	$(LIB)(clean.o)		\
	$(LIB)(userexit.o)	\
	$(LIB)(zero.o)		\
	$(LIB)(zeropad.o)	\
	$(NULL)

all:	$(LIB)

$(LIB): $(MEMBERS)
	$(RANLIB) $@

install:all
	${INSTALL} -g bin -u bin -u 644 -F /usr/lib \
	-idb "std.sw.unix mr" "${LIB}"
	ranlib ${ROOT}/usr/lib/${LIB}

clean:
	rm -f *.o
	
clobber:clean
	rm -f $(LIB)

.c.o:
	$(CC) $(CFLAGS) -c $< ; set +e;$(LD) -r -x $@;test $$? -le 1;mv a.out $@

.c.a:
	$(CC) $(CFLAGS) -c $< ; set +e;$(LD) -r -x $%;test $$? -le 1
	/bin/mv a.out $% ; ar rv $@ $% ; /bin/rm -f $%

.s.o:
	$(CC) $(CFLAGS) -c $< ; set +e;$(LD) -r -x $@;test $$? -le 1;mv a.out $@

.s.a:
	$(CC) $(CFLAGS) -c $< ; set +e;$(LD) -r -x $%;test $$? -le 1
	/bin/mv a.out $% ; ar rv $@ $% ; /bin/rm -f $%

.PRECIOUS:$(LIB)
