# makefile for adventure

include ${ROOT}/usr/include/make/commondefs

# $Source: /d2/3.7/src/games/adventure/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 15:43:07 $

CFILES	= done.c init.c io.c main.c save.c subr.c vocab.c wizard.c
OFILES	= done.o init.o io.o main.o save.o subr.o vocab.o wizard.o
IDBTAG  = -idb "std.sw.unix"

NODASHN	= --n

B	= /usr/games

all:	adventure

adventure:a.out
	rm -f $@ advent
	echo y | ./a.out
	mv advent adventure
	rm -f a.out

# Adventure musn't run pure or separate at present.
a.out:	$(OFILES)
	$(CC) $(NODASHN) $(OFILES) $(LDFLAGS) 

#save.o:save.c	# vax C optimizer bug - infinite loop
# 	$(CCF) -c save.c
save.o:
	$(CCF) -DW2_0 -c $*.c

## always recomple okplay
#okplay:okplay.c FRC
#	$(CCF) -o $@ $(LDFLAGS) okplay.c

install:all
	${INSTALL} -u games -g games -m 755 -F ${B} ${IDBTAG} adventure

#advsaver:advsaver.s
#	$(AS) -o $@ advsaver.s

clean:
	rm -f $(OFILES)
	
clobber:clean
	rm -f a.out advent adventure core errs

lint:
	lint $(CFILES) > lint.out 2>&1

.c.o:
	cc -O -c $<

FRC:
