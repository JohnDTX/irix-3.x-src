# Construct /usr/include

# $Source: /d2/3.7/src/include/RCS/Makefile,v $
# $Revision: 1.1 $
# $Date: 89/03/27 16:11:09 $

include ${ROOT}/usr/include/make/commondefs

HEADERS = a.out.h alarm.h ar.h assert.h core.h ctype.h curses.h dbx.h dial.h \
	  dirent.h dumprestor.h errno.h execargs.h fatal.h fcntl.h filehdr.h \
	  fortfpe.h fperr.h fpopcodes.h fpregs.h fpsignal.h ftw.h grp.h  \
	  ibtab.h ldfcn.h limits.h linenum.h macros.h math.h memory.h mmu.h \
	  mon.h nan.h ndir.h nlist.h pascfpe.h pascfpedefs.h pwd.h ranlib.h \
	  regexp.h reloc.h rje.h scnhdr.h search.h setjmp.h sgimath.h signal.h \
	  stab.h stand.h stdio.h storclass.h string.h strings.h stropts.h \
	  symbol.h syms.h sysexits.h sysmacros.h termio.h time.h tp_defs.h \
	  unctrl.h ustat.h utmp.h values.h var.h varargs.h \
	  varvals.h x25lib.h

all default clean clobber:
	cd make; make $@

install:install_$(PROD)

install_1000:install_common

install_2000:install_common
	cd ../gl2/gl2/include; make Include

install_3000:install_common
	cd ../gl2/gl2/include; make Include

install_common:
	${INSTALL} -dir /usr/include  -u root -g sys -m 555 -idb "std.sw.unix" 
	${INSTALL} -u root -g sys -m 444 -F /usr/include \
		-idb "std.sw.unix" "${HEADERS}"
	cd make; make install
	cd ../bsd/include; make Include
	cd ../sun/include; make Include
	#
	cd ../sun/usr.lib; make Include
	cd ../sys; make Include
	cd ../mextools/mextools/include; make Include
	cd ${ROOT}/usr/include; rm -rf gl; ln -s gl2 gl
