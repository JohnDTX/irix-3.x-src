#
# Makefile for GF2 microcode sources
#

include ${ROOT}/usr/include/make/commondefs

LCINCS  = -I../include -I${ROOT}/usr/include
INCLUDES = ../include/mas.h \
	   ../include/fbcparams.h \
	   ../include/fbc.h \
	   ../include/fbc.mic.h\
	   ../include/consts.h

LCDEFS   = -DUC4 -DFBC 

.SUFFIXES : .E

MICS =		\
		attributes.o \
		lowmem.o\
		dispatch.o\
		vectors.o\
		block.o\
		buffcopy.o\
		dbzline.o\
		pixels.o\
		mvppixels.o\
		readpixels.o\
		cursor.o\
		copypixels.o\
		copyfont.o\
		viewport.o\
		char.o\
		chdraw.o\
		runlen.o\
		readrun.o\
		fixchar.o\
		diag.o\
		divide.o\
		stm.o\
		hitrept.o\
		modes.o\
		ldivide.o\
		points.o\
		depthvec.o\
		zvectors.o


all: $(INCLUDES) $(MICS) 

modes.o: ../include/version.h

.c.o: 
	$(CCF) -c $< 

.c.E: 
	$(CCF) -E $<  >$*.E

# remove files that can be re-created

clean :
	rm -f $(OBJS) *.out *.CKP *.BAK .emacs* *.o *.hex* *.cksum gf1 fbc
