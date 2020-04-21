# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	7.12 (Berkeley) 9/7/85
#
VS=3.7
#
# Ex skeletal makefile for VAX VM/Unix 4.2BSD
#
# NB: This makefile doesn't indicate any dependencies on header files.
#
# Ex is very large - this version will not fit on PDP-11's without overlay
# software.  Things that can be turned off to save
# space include LISPCODE (-l flag, showmatch and lisp options), UCVISUAL
# (visual \ nonsense on upper case only terminals), CHDIR (the undocumented
# chdir command.)  CRYPT includes the code to edit encrypted files (the -x
# option, like ed.)  VMUNIX makes ex considerably larger, raising many limits
# and improving speed and simplicity of maintenance.  It is suitable only
# for a VAX or other large machine, and then probably only in a paged system.
#
# Don't define VFORK unless your system has the VFORK system call,
# which is like fork but the two processes have only one data space until the
# child execs. This speeds up ex by saving the memory copy.
#
# If your system expands tabs to 4 spaces you should -DTABS=4 below
#
#

include ${ROOT}/usr/include/make/commondefs
INSTCMD=${INSTALL} -idb "root upd" 

EDITLINKS = edit ex view
BD=	/bin
LB=	/usr/lib
FOLD=	fold
CTAGS=	ctags
XSTR=	xstr
DEBUGFLAGS=	-DTRACE -g
NONDEBUGFLAGS=	-O
DEB=	${NONDEBUGFLAGS}	# or ${DEBUGFLAGS} to to debug
#LCOPTS= -DCRYPT -DLISPCODE -DCHDIR -DUCVISUAL -DVFORK -DVMUNIX -DFASTTAG
LCOPTS	= -DLISPCODE -DCHDIR -DUSG3TTY -DVMUNIX -DFASTTAG
LCINCS=	-I${ROOT}/usr/include
LCDEFS=	-DTABS=8 ${DEB} 
LDFLAGS=	-z		# or -i or -n
TERMLIB=	-ltermcap
MKSTR=	mkstr
CXREF=	cxref
PR=	pr
OBJS=	ex.o ex_addr.o ex_cmds.o ex_cmds2.o ex_cmdsub.o \
	ex_data.o ex_extern.o ex_get.o ex_io.o ex_put.o ex_re.o \
	ex_set.o ex_subr.o ex_tagio.o ex_temp.o ex_tty.o ex_unix.o \
	ex_v.o ex_vadj.o ex_vget.o ex_vmain.o ex_voper.o \
	ex_vops.o ex_vops2.o ex_vops3.o ex_vput.o ex_vwind.o \
	printf.o bcopy.o strings.o
HDRS=	ex.h ex_argv.h ex_re.h ex_temp.h ex_tty.h ex_tune.h ex_vars.h ex_vis.h
SRC1=	ex.c ex_addr.c ex_cmds.c ex_cmds2.c ex_cmdsub.c
SRC2=	ex_data.c ex_get.c ex_io.c ex_put.c ex_re.c
SRC3=	ex_set.c ex_subr.c ex_tagio.c ex_temp.c ex_tty.c ex_unix.c
SRC4=	ex_v.c ex_vadj.c ex_vget.c ex_vmain.c ex_voper.c
SRC5=	ex_vops.c ex_vops2.c ex_vops3.c ex_vput.c ex_vwind.c
SRC6=	printf.c bcopy.c expreserve.c exrecover.c
MISC=	makefile READ_ME rofix
VGRIND=	csh /usr/ucb/vgrind
VHDR=	"Ex Version ${VS}"

.c.o:
# ifdef VMUNIX
	${CC} -E ${CFLAGS} $*.c | ${XSTR} -c -
# else
#	${MKSTR} - ex${VS}strings x $*.c
#	${CC} -E ${CFLAGS} x$*.c | ${XSTR} -c -
#	rm -f x$*.c
# endif
	${CC} ${CFLAGS} -c x.c 
	mv x.o $*.o

all default:vi ${EDITLINKS} exrecover expreserve

vi:	${OBJS}
	${CCF} -o $@ ${OBJS} ${TERMLIB} ${LDFLAGS}

${EDITLINKS}:vi
	rm -f $@; ln $? $@

tags:	/tmp
	${CTAGS} -w ex.[hc] ex_*.[hc]

${OBJS}: ${HDRS}

ex_vars.h:
	csh makeoptions ${CFLAGS}

bcopy.o:	bcopy.c
	${CC} -c ${CFLAGS} bcopy.c

# xstr: hands off!
strings.o: strings
	${XSTR}
	${CC} -c -S xs.c
	ed - <rofix xs.s
	${AS} -o strings.o xs.s
	rm xs.s
	
exrecover: exrecover.c ex_extern.o 
	${CCF} exrecover.c ex_extern.o -o exrecover

expreserve: expreserve.c
	${CCF} expreserve.c -o expreserve

clean:
#	If we dont have ex we cant make it so don't rm ex_vars.h
	rm -rf a.out strings core errs trace
	rm -rf *.o x*.[cs]

clobber: clean
	rm -f exrecover expreserve vi ${EDITLINKS}


# install in standard place (/usr/ucb)
install:default
	${INSTCMD} -u bin -g bin -m 755 -F ${BD} vi
	${INSTCMD} -ln ${BD}/vi -F ${BD} "${EDITLINKS}"
	${INSTCMD} -m 4755 -src exrecover -F ${LB} ex${VS}recover
	${INSTCMD} -m 4755 -src expreserve -F ${LB} ex${VS}preserve

lint:
	lint ${CFLAGS} ex.c ex_?*.c
	lint ${CFLAGS} -u exrecover.c
	lint ${CFLAGS} expreserve.c

print:
	@${PR} READ* BUGS
	@${PR} makefile*
	@${PR} /etc/termcap
	@(size -l vi ; size *.o) | ${PR} -h sizes
	@${PR} -h errno.h ${INCLUDE}/errno.h
	@${PR} -h setjmp.h ${INCLUDE}/setjmp.h
	@${PR} -h sgtty.h ${INCLUDE}/sgtty.h
	@${PR} -h signal.h ${INCLUDE}/signal.h
	@${PR} -h sys/stat.h ${INCLUDE}/sys/stat.h
	@${PR} -h sys/types.h ${INCLUDE}/sys/types.h
	@ls -ls | ${PR}
	@${CXREF} *.c | ${PR} -h XREF
	@${PR} *.h *.c
vgrind:
	tee index < /dev/null
	${VGRIND} -h ${VHDR} ${HDRS}
	${VGRIND} -h ${VHDR} ${SRC1}
	${VGRIND} -h ${VHDR} ${SRC2}
	${VGRIND} -h ${VHDR} ${SRC3}
	${VGRIND} -h ${VHDR} ${SRC4}
	${VGRIND} -h ${VHDR} ${SRC5}
	${VGRIND} -h ${VHDR} ${SRC6}
	${VGRIND} -n -h ${VHDR} ${MISC}
	${VGRIND} -i -h ${VHDR} index
