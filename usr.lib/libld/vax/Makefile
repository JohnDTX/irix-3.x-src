#	@(#)makefile	2.1	

#******************************************************************************
#									      *
#		COMMON LIBRARY (libld.a) Makefile			      *
#									      *
#******************************************************************************


COMI = ../../../head
CDIR = ../common
INSDIR = .

CFLAGS = -O -c
PPFLAGS = -I$(COMI)

LPT = dpr -mx

OFILES = ldaclose.o ldaopen.o ldclose.o ldahread.o ldlseek.o ldnlseek.o \
	ldnrseek.o ldnsseek.o ldnshread.o ldopen.o ldrseek.o ldlread.o \
	ldsseek.o ldshread.o ldfhread.o ldtbseek.o ldohseek.o ldtbread.o \
	ldtbindex.o allocldptr.o freeldptr.o vldldptr.o sgetl.o sputl.o

CFILES = $(CDIR)/ldaclose.c $(CDIR)/ldaopen.c $(CDIR)/ldclose.c\
	$(CDIR)/ldahread.c $(CDIR)/ldlseek.c $(CDIR)/ldnlseek.c\
	$(CDIR)/ldnrseek.c $(CDIR)/ldnsseek.c $(CDIR)/ldnshread.c\
	$(CDIR)/ldopen.c $(CDIR)/ldrseek.c $(CDIR)/ldlread.c\
	$(CDIR)/ldsseek.c $(CDIR)/ldshread.c $(CDIR)/ldfhread.c\
	$(CDIR)/ldtbseek.c $(CDIR)/ldohseek.c $(CDIR)/ldtbread.c\
	$(CDIR)/ldtbindex.c $(CDIR)/allocldptr.c $(CDIR)/freeldptr.c\
 	$(CDIR)/vldldptr.c $(CDIR)/sgetl.c $(CDIR)/sputl.c

HFILES = $(CDIR)/lddef.h $(COMI)/ldfcn.h

all:	libld.a

libld.a:  $(COMI)/ldfcn.h $(COMI)/filehdr.h $(COMI)/scnhdr.h \
	  $(COMI)/syms.h $(CDIR)/lddef.h $(OFILES)
	rm -f  libld.a
	ar qv libld.a $(OFILES)
	cp libld.a $(INSDIR)/libld.a

allocldptr.o:	$(COMI)/filehdr.h $(COMI)/ldfcn.h $(CDIR)/lddef.h \
		$(CDIR)/allocldptr.c
	$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/allocldptr.c

freeldptr.o:  $(COMI)/filehdr.h $(COMI)/ldfcn.h $(CDIR)/lddef.h \
	      $(CDIR)/freeldptr.c
	$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/freeldptr.c

vldldptr.o:  $(COMI)/filehdr.h $(COMI)/ldfcn.h $(CDIR)/lddef.h \
	     $(CDIR)/vldldptr.c
	$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/vldldptr.c

ldaclose.o:  $(COMI)/filehdr.h $(COMI)/ldfcn.h $(CDIR)/ldaclose.c
		$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/ldaclose.c 

ldaopen.o:  $(COMI)/filehdr.h $(COMI)/ldfcn.h $(CDIR)/ldaopen.c
		$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/ldaopen.c 

ldclose.o:  $(COMI)/filehdr.h $(COMI)/ldfcn.h $(CDIR)/ldclose.c
		$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/ldclose.c 

ldahread.o:  $(COMI)/filehdr.h $(COMI)/ldfcn.h $(CDIR)/ldahread.c
		$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/ldahread.c 

ldlseek.o:  $(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/ldfcn.h $(CDIR)/ldlseek.c
		$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/ldlseek.c 

ldnlseek.o:  $(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/ldfcn.h \
	     $(CDIR)/ldnlseek.c
		$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/ldnlseek.c 

ldnrseek.o:  $(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/ldfcn.h \
	     $(CDIR)/ldnrseek.c
		$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/ldnrseek.c 

ldnsseek.o:  $(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/ldfcn.h \
	     $(CDIR)/ldnsseek.c
		$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/ldnsseek.c 

ldnshread.o:  $(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/ldfcn.h \
	      $(CDIR)/ldnshread.c
		$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/ldnshread.c 

ldopen.o:  $(COMI)/filehdr.h $(COMI)/ldfcn.h $(CDIR)/ldopen.c
		$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/ldopen.c 

ldrseek.o:  $(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/ldfcn.h $(CDIR)/ldrseek.c
		$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/ldrseek.c 

ldtbread.o:  $(COMI)/filehdr.h $(COMI)/syms.h $(COMI)/storclass.h \
	     $(COMI)/ldfcn.h $(CDIR)/ldtbread.c
		$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/ldtbread.c 

ldsseek.o:  $(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/ldfcn.h $(CDIR)/ldsseek.c
		$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/ldsseek.c 

ldshread.o:  $(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/ldfcn.h \
	     $(CDIR)/ldshread.c
		$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/ldshread.c 

ldfhread.o:  $(COMI)/filehdr.h $(COMI)/ldfcn.h $(CDIR)/ldfhread.c
		$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/ldfhread.c 

ldtbseek.o:  $(COMI)/ldfcn.h $(COMI)/filehdr.h $(CDIR)/ldtbseek.c
		$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/ldtbseek.c

ldohseek.o:  $(COMI)/ldfcn.h $(COMI)/filehdr.h $(CDIR)/ldohseek.c
		$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/ldohseek.c

ldtbindex.o:  $(COMI)/ldfcn.h $(COMI)/filehdr.h $(COMI)/syms.h \
	      $(COMI)/storclass.h $(CDIR)/ldtbindex.c
	$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/ldtbindex.c

ldlread.o:  $(COMI)/ldfcn.h $(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/syms.h \
	    $(COMI)/linenum.h $(COMI)/storclass.h $(CDIR)/ldlread.c
	$(CC) $(CFLAGS) $(PPFLAGS) $(CDIR)/ldlread.c

sgetl.o: $(CDIR)/sgetl.c
	$(CC) $(CFLAGS) $(CDIR)/sgetl.c

sputl.o: $(CDIR)/sputl.c
	$(CC) $(CFLAGS) $(CDIR)/sputl.c

clean:
	rm -f *.o libld.a

listing:	
	pr -n makefile $(HFILES) $(CFILES) | $(LPT)
	@echo "files sent to lineprinter!"
