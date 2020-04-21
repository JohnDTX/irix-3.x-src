#	make file for STYLE and DICTION
#
include ${ROOT}/usr/include/make/commondefs
#
#	@(#)Makefile	4.5	(Berkeley)	83/06/30
# @(#)$Header: /d2/3.7/src/usr.bin/diction/RCS/Makefile,v 1.1 89/03/27 17:45:11 root Exp $
# $Log:	Makefile,v $
# Revision 1.1  89/03/27  17:45:11  root
# Initial check-in for 3.7
# 
# Revision 1.9  87/05/22  15:30:51  andre
# changed CC to CCF to use internal CFLAGS
# 
# Revision 1.8  87/05/04  10:41:59  andre
# 3.6 Product, New 5.3 Style Installs Added, Native Installs done
# 
# Revision 1.7  87/05/04  00:31:11  andre
# 3.6 Product, New 5.3 Style Installs Added, Native Installs done
# 
# Revision 1.5  86/03/06  19:38:04  root
# eliminated calls to install program.
# 
# Revision 1.4  85/07/05  10:02:35  bob
# Added LDFLAGS to cc -o line.
# 
# Revision 1.3  85/03/11  13:24:10  bob
# *** empty log message ***
# 
#
#	BIN is the directory for storing the object code
#		for the 3 programs that make up style
#	DICT is the full pathname of the file dict.d, the
#		dictionary file used by DICTION
#
SRCS	= \
	abbrev.c conp.h dict.c dict.d \
	diction.c diction.sh edict.c ehash.c \
	explain.d \
	end.l getopt.c names.h nhash.c \
	nwords.l outp.c part.l pscan.c \
	style.h style.sh explain.sh ydict.c \
	README

DICT	= -DDICT=\"/usr/lib/dict.d\"
INSTALLS = style1 style2 style3 dprog dict.d explain.d

all:	diction style

style:	style1 style2 style3

install:style diction style.sh diction.sh explain.sh dict.d explain.d
	${INSTALL} -u bin -g bin -m 755 -F /usr/lib  \
	-idb "std.sw.unix mr" "${INSTALLS}"
	${INSTALL} -src style.sh -u bin -g bin -m 755 -F /usr/bin  \
	-idb "std.sw.unix mr" style
	${INSTALL} -src diction.sh -u bin -g bin -m 755 -F /usr/bin  \
	-idb "std.sw.unix mr" diction
	${INSTALL} -src explain.sh -u bin -g bin -m 755 -F /usr/bin  \
	-idb "std.sw.unix mr" explain

clean:
	rm -f *.o
	rm -f prtlex.c wdslex.c endlex.c style1 style2 style3 dprog

clobber:clean
	rm -f diction style style1 style2 style3

style3:	prtlex.o pscan.o outp.o
	$(CCF) prtlex.o pscan.o outp.o -ll -o $@ $(LDFLAGS) 

prtlex.c:part.l
	lex part.l
	mv lex.yy.c prtlex.c

prtlex.o:names.h prtlex.c conp.h style.h names.h
	$(CCF) -c prtlex.c

pscan.o:names.h conp.h pscan.c
	$(CCF) -c pscan.c

outp.o:	names.h conp.h style.h outp.c
	$(CCF) -c outp.c

style1:	wdslex.o
	$(CCF) wdslex.o -ll -o $@ $(LDFLAGS) 

wdslex.o:wdslex.c nhash.c dict.c ydict.c names.h abbrev.c
	$(CCF) -c wdslex.c

wdslex.c:nwords.l
	lex nwords.l
	mv lex.yy.c wdslex.c

style2:	endlex.o
	$(CCF) endlex.o -ll -o $@ $(LDFLAGS) 

endlex.o:names.h endlex.c ehash.c edict.c
	$(CCF) -c endlex.c

endlex.c:end.l
	lex end.l
	mv lex.yy.c endlex.c

diction:dprog

dprog:	diction.c
	$(CCF) $(DICT) diction.c -o $@ $(LDFLAGS) 

srcs:	sources

sources:$(SRCS)

$(SRCS):
	get $@
