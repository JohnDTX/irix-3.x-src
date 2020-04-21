
#
#
include ${ROOT}/usr/include/make/commondefs
LCDEFS = -DFsTYPE=2 -DEFS
LIBFSTOOLSDIR =	../toyfs
LIBFSTOOLS =	$(LIBFSTOOLSDIR)/libtoyfs.a
EFS_OBJS =	efs_fsck.o \
		efs_phase1.o efs_phase2.o efs_phase3.o \
		efs_phase4.o efs_phase5.o efs_phase6.o
CHECKERS=	fsck.efs fsck.bell

all:	fsck
	:

install: all
	${INSTALL} -u root -g sys -m 755 -F /etc \
	-idb "std.sw.unix mr" "${CHECKERS}"
	${INSTALL} -ln /etc/fsck.efs -F /etc \
	-idb "std.sw.unix mr" fsck

clean:
	rm -f *.o

clobber: clean
	rm -f $(CHECKERS)

.c.o:
	$(CC) -c $(CFLAGS) $(DEFINES) $*.c

$(EFS_OBJS): efs_fsck.h
	$(CC) -c $(CFLAGS) -DFsTYPE=1 $*.c

fsck.bell.o: fsck.c
	$(CC) -c $(CFLAGS) -DFsTYPE=2 fsck.c
	mv fsck.o $@

fsck1b.bell.o: fsck.c
	$(CC) -c $(CFLAGS) -DFsTYPE=1 fsck.c
	mv fsck.o $@

fsck:	$(CHECKERS)
	-rm -f $@
	ln fsck.efs $@

fsck.efs: $(EFS_OBJS) $(MAKEFILE) $(LIBFSTOOLS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(EFS_OBJS) $(LIBFSTOOLS)

fsck.bell: fsck.bell.o $(MAKEFILE) $(LIBFSTOOLS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ fsck.bell.o $(LIBFSTOOLS)

fsck1b.bell: fsck1b.bell.o $(MAKEFILE) $(LIBFSTOOLS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ fsck1b.bell.o $(LIBFSTOOLS)

$(LIBFSTOOLS):
	cd $(LIBFSTOOLSDIR); $(MAKE)
