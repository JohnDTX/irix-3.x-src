#
# $Source: /d2/3.7/src/stand/lib/dev/RCS/Makefile,v $
# $Revision: 1.1 $
# $Date: 89/03/27 17:14:15 $
#

include ../Make.h

# Local definitions:



OFILES = \
	asm.o\
	bfs_fs.o\
	bootp.o \
	close.o \
	conf.o \
	cpio.o \
	duart.o \
	dk.o \
	efs_fs.o\
	enet.o \
	fs.o \
	gen.o \
	init.o \
	in_addr.o \
	in_ntoa.o \
	iph.o \
	keybd.o \
	keymap.o \
	mbmalloc.o \
	md.o \
	nx.o \
	open.o \
	perror.o \
	probe.o \
	prf.o \
	rdwr.o \
	rom.o \
	seq.o \
	spl.o \
	std.o \
	stt.o \
	subr.o \
	sys.o \
	splitspec.o \
	tcp.o \
	tod.o \
	trap.o \
	xns.o \
	ctype.o

default: all
	$(ECHO) done

all: $(OFILES)


clean:
	rm -f *.o

clobber:	clean


duart.o:
	$(CC) $(DFLAGS) -I$(INCL) -I${I} -OKS -c duart.c

spl.o:
	$(AS) -o $@ spl.s


# Header file dependancies
bell_subr.o: $(I)/sys/types.h dklabel.h $(I)/sys/inode.h $(I)/sys/ino.h $(I)/sys/dir.h $(INCL)/safs.h $(INCL)/dprintf.h

bellfs.o: $(INCL)/stand.h $(I)/sys/types.h dklabel.h $(INCL)/bell_fs.h $(INCL)/bell_inode.h $(INCL)/bell_ino.h $(INCL)/bell_dir.h $(INCL)/safs.h $(INCL)/dprintf.h

close.o:	$(INCL)/stand.h

conf.o:	$(INCL)/stand.h

duart.o:	$(I)/sys/types.h $(INCL)/cpureg.h $(INCL)/common.h

enet.o:	$(INCL)/net.h $(INCL)/cntrlr.h $(INCL)/dprintf.h

fs.o: $(INCL)/stand.h $(INCL)/dprintf.h

init.o:	$(I)/sys/types.h $(INCL)/cpureg.h $(INCL)/common.h

malloc.o: $(INCL)/dprintf.h

mbmalloc.o:	$(INCL)/stand.h $(INCL)/cpureg.h $(INCL)/common.h $(I)/sys/sysmacros.h $(INCL)/mbenv.h $(INCL)/dprintf.h

md.o: $(INCL)/stand.h $(INCL)/mbenv.h $(I)/sys/types.h dklabel.h md.h $(INCL)/dprintf.h

nx.o:	$(I)/errno.h $(INCL)/nx.h $(I)/xns/Xns.h $(INCL)/mbenv.h $(INCL)/cntrlr.h $(INCL)/dprintf.h

open.o:	$(INCL)/stand.h

perror.o: $(I)/errno.h

prf.o:	$(I)/ctype.h $(I)/sys/types.h $(INCL)/cpureg.h $(INCL)/common.h

probe.o:	$(I)/sys/types.h $(INCL)/cpureg.h $(INCL)/common.h $(I)/setjmp.h

rdwr.o:	$(INCL)/stand.h $(INCL)/dprintf.h

seq.o:	$(INCL)/net.h $(I)/errno.h $(INCL)/cntrlr.h $(INCL)/dprintf.h

splitspec.o:	$(I)/ctype.h

sys.o:	$(INCL)/stand.h

tod.o:	$(I)/sys/types.h $(INCL)/cpureg.h $(INCL)/tod.h

trap.o:	$(I)/sys/types.h $(INCL)/cpureg.h $(INCL)/common.h


xns.o:	$(INCL)/stand.h $(I)/xns/Xns.h $(INCL)/cntrlr.h $(INCL)/dprintf.h
