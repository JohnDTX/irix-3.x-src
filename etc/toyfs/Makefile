#
#
include ${ROOT}/usr/include/make/commondefs
#
# makefile for toyfs.a

# relevant defines are
#	EFS	- include code for extent file system
#	BELL	- include code for bell file system
#	BKY	- include code for bky file system (unimplemented)
#


#----- parameters -----
ARCHIVE=	ark.a
CFLAGS=		-O -UDEBUG -DEFS -DBELL -UBKY
INCLUDES=	-ZN -I${ROOT}/usr/include -I.

MAKEFILE=	Makefile
L=		
#-----


#----- tool names -----
AR=		ar
AS=		as
CC=		cc
CPP=		cpp
LD=		ld
RANLIB=		ranlib
#-----


#----- abbreviations -----
C=		${CC} ${CFLAGS} ${INCLUDES}
E=		@echo mk
I=		@echo>>Install
M=		@echo link $@;$C ${LDFLAGS} -o $@ $@.o ${LIBS}
T=		@echo>>Targets
X=		ar x ${ARCHIVE}
#-----


#----- default rules -----
.SUFFIXES:
.SUFFIXES:	.c .i .o .s

default: all

.c.o:
	$C -URONLY -c $*.c
	@-mv -f `basename $*.o` $*.o 2> /dev/null
#	@${LD} -T 0 -o ${AOUT} -r -x $*.o;mv -f ${AOUT} $*.o
.c.ro:
	$c -DRONLY -c $*.c
	@mv -f `basename $*.o` $*.ro
#	@${LD} -T 0 -o ${AOUT} -r -x $*.ro;mv -f ${AOUT} $*.ro
.c.s:
	$C -S $*.c
.s.o:
	$C -c $*.s
.c.i:
	$C -E $*.c > $*.i
#-----


#----- required targets -----
all:	toytest libtoyfs.a
	:

clean:
	-rm -f ${TOYOBJS} *.o

clobber: clean
	-rm -f ${TOYPROGS} toytest libtoyfs.a

install: all

release:

#-----


#----- peculiar defines -----
TOYPROGS = toycat toycreat toyls toyrm
TOYLIB =	libtoyfs.a

BKY_OBJS =

BELL_OBJS = \
	bell/bell_dkalc.o \
	bell/bell_ialc.o \
	bell/bell_idump.o \
	bell/bell_imake.o \
	bell/bell_iread.o \
	bell/bell_iwr.o \
	bell/bell_mount.o \
	bell/bell_data.o \
	bell/bell_rbmap.o \
	bell/bell_rdir.o \
	bell/bell_readi.o \
	bell/bell_sync.o \
	bell/bell_umt.o \
	bell/bell_wbmap.o \
	bell/bell_wdir.o \
	bell/bell_wmt.o \
	bell/bell_wri.o

EFS_OBJS = \
	efs/efs_cksum.o \
	efs/efs_dkalc.o \
	efs/efs_ialc.o \
	efs/efs_idump.o \
	efs/efs_imake.o \
	efs/efs_iread.o \
	efs/efs_iwr.o \
	efs/efs_mount.o \
	efs/efs_data.o \
	efs/efs_rbmap.o \
	efs/efs_rdir.o \
	efs/efs_readi.o \
	efs/efs_sync.o \
	efs/efs_umt.o \
	efs/efs_wbmap.o \
	efs/efs_wdir.o \
	efs/efs_wmt.o \
	efs/efs_wri.o

LIB_OBJS = \
	lib/superblk.o \
	lib/_basename.o \
	lib/_cdate.o \
	lib/_cook.o \
	lib/_error.o \
	lib/_isnum.o \
	lib/_newstr.o

TOYSYS_OBJS = \
	toysys/toy_buf.o \
	toysys/toy_close.o \
	toysys/toy_clfs.o \
	toysys/toy_creat.o \
	toysys/toy_dir.o \
	toysys/toy_error.o \
	toysys/toy_fsalc.o \
	toysys/toy_fsfree.o \
	toysys/toy_idump.o \
	toysys/toy_imake.o \
	toysys/toy_init.o \
	toysys/toy_inode.o \
	toysys/toy_iwr.o \
	toysys/toy_link.o \
	toysys/toy_malc.o \
	toysys/toy_mknod.o \
	toysys/toy_namei.o \
	toysys/toy_nulfn.o \
	toysys/toy_open.o \
	toysys/toy_openfs.o \
	toysys/toy_openi.o \
	toysys/toy_read.o \
	toysys/toy_rofs.o \
	toysys/toy_seek.o \
	toysys/toy_stdio.o \
	toysys/toy_sync.o \
	toysys/toy_unlk.o \
	toysys/toy_wofs.o \
	toysys/toy_write.o

TOYOBJS=	${TOYSYS_OBJS} ${LIB_OBJS} \
		${EFS_OBJS} ${BELL_OBJS} ${BKY_OBJS}
#-----


#----- peculiar targets -----
${TOYOBJS}: toyfs.h toyops.h

${EFS_OBJS}: efs_toyfs.h

${BELL_OBJS}: bell_toyfs.h

${TOYLIB}: ${TOYOBJS} ${MAKEFILE}
	@-rm -f $@
	@echo ${AR}
	@${AR} r $@ ${TOYOBJS}
	${RANLIB} $@
toytest: toytest.o ${TOYLIB} ${MAKEFILE}
	@-rm -f $@
	$C -o $@ toytest.o ${TOYLIB}
toytest.o: toytest.c
	$C -URONLY -c $?
${TOYPROGS}: toytest
	-rm -f $@
	ln toytest $@
#-----
