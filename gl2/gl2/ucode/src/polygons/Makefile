#
# Makefile for GF2 microcode sources
#

include ${ROOT}/usr/include/make/commondefs

HDWR	 = ../../..
LCINCS  = -I../../include -I$(HDWR)/include

LCDEFS   = -DUC4 -DFBC 

SRCS     =	\
		addsentinels.c\
		dbzfillline.c\
		dbzshadeline.c\
		bumpindex.c\
		checkmask.c\
		filltrapzoid.c\
		finishside.c\
		shfinishside.c\
		getfrstsds.c\
		polydraw.c\
		polyclose.c\
		polyexit.c\
		startside.c\
		swap.c\
		traploop.c\
		advance.c\
		computeslope.c\
		shadeline.c\
		shadetrap.c\
		shgetfrstsds.c\
		shstartside.c\
		shtraploop.c\
		zfillline.c\
		zfilltrap.c\
		zgetfrstsds.c\
		zshadeline.c\
		zshadetrap.c\
		zfinishside.c\
		zstartside.c\
		ztraploop.c

MICS =		\
		addsentinels.o\
		bumpindex.o\
		checkmask.o\
		filltrapzoid.o\
		dbzfillline.o\
		dbzshadeline.o\
		finishside.o\
		shfinishside.o\
		getfrstsds.o\
		polydraw.o\
		polyclose.o\
		polyexit.o\
		startside.o\
		swap.o\
		traploop.o\
		advance.o\
		computeslope.o\
		shadeline.o\
		shadetrap.o\
		shgetfrstsds.o\
		shstartside.o\
		shtraploop.o\
		zfillline.o\
		zfilltrap.o\
		zgetfrstsds.o\
		zshadeline.o\
		zshadetrap.o\
		zfinishside.o\
		zstartside.o\
		ztraploop.o

all: $(MICS) 

clean:
	rm -f $(OBJS) *.out *.CKP *.BAK .emacs* *.o *.hex* *.cksum *.x *.b


.c.o:	polydefs.h
	$(CCF) -c $<

.c.e:	polydefs.h
	$(CCF) -E $< >$@
