
# Makefile for filehand.  Should be callable from other makes. 
#  @(#)fhand.mk	1.2
FHAND = .
# Filehand directory.
STD = -c -O
# Compilation flags.
PARM =
# To invoke tracing of the routines, add -DTRACE to PARM.
# To create an object module that is compatible with code using xalloc and
#   xfree, you MUST use the -DXALLOC flag with PARM.
# To do this, use make -f $FHAND/makefile PARM="-DTRACE -DXALLOC"

filehand.o: $(FHAND)/filehand.c $(FHAND)/filehand.h
	cc $(STD) $(PARM) -DFHAND=\"$(FHAND) $(FHAND)/filehand.c

# Use the lint entry for debugging purposes.

lint: $(FHAND)/filehand.c $(FHAND)/filehand.h
	lint -DTRACE -DFHAND=\"$(FHAND) $(FHAND)/filehand.c

cref: $(FHAND)/filehand.c $(FHAND)/filehand.h
	cref -lc $(FHAND)/filehand.c $(FHAND)/filehand.h

love: $(FHAND)/filehand.c $(FHAND)/filehand.h $(FHAND)/makefile
	list $(FHAND)/filehand.c $(FHAND)/filehand.h \
	  $(FHAND)/makefile

