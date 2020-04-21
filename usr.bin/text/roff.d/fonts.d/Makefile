#	troff font tables make file

#
#
include ${ROOT}/usr/include/make/commondefs
#	SCCS:  @(#)re-constructed from fonts.mk	1.15
# $Source: /d2/3.7/src/usr.bin/text/roff.d/fonts.d/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 18:27:56 $

L	= /usr/lib/font
FFILES	= chars.c ftB.c ftBC.c ftC.c ftCE.c ftCI.c ftCK.c ftCS.c ftCW.c \
	 ftG.c ftGI.c ftGM.c ftGR.c ftI.c ftL.c ftLI.c ftPA.c ftPB.c \
	 ftPI.c ftR.c ftS.c ftSB.c ftSI.c ftSM.c ftUD.c
PFILES	= makefonts.c
MKFILES	= mkfont.c mkfont1.c
FILES	= ftB ftBC ftC ftCE ftCI ftCK ftCS ftCW ftG ftGI ftGM ftGR ftI \
	ftL ftLI ftPA ftPB ftPI ftR ftS ftSB ftSI ftSM ftUD

all:	$(FILES)

$(FILES):makefonts
	./makefonts
	chmod 644 $(FILES)

makefonts:$(PFILES) $(MKFILES) $(PFILES)
	cc -I${ROOT}/usr/include -O -s -o $@ $@.c		

install:all $L
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib/font \
	-idb "std.sw.unix mr" "${FILES}"
	${INSTALL} -ln /usr/lib/font/ftG -F /usr/lib/font \
	-idb "std.sw.unix mr" ftH
	${INSTALL} -ln /usr/lib/font/ftGI -F /usr/lib/font \
	-idb "std.sw.unix mr" ftHI
	${INSTALL} -ln /usr/lib/font/ftGM -F /usr/lib/font \
	-idb "std.sw.unix mr" ftHM
	${INSTALL} -ln /usr/lib/font/ftUD -F /usr/lib/font \
	-idb "std.sw.unix mr" ftFD

mkfont:  $$@.c mkfont1.c
	$(CC) $(LDFLAGS) $(NFLAG) $@.c

$L:
	${INSTALL} -dir $L -u bin -g bin -m 755   \
	-idb "std.sw.unix mr" 

clean:
	rm -f makefonts a.out *.o

clobber:clean
	rm -f $(FILES) mkfont
