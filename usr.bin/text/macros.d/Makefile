#	text Development Support Library (DSL) macros make file

#
#
include ${ROOT}/usr/include/make/commondefs

# @(#)$Header: /d2/3.7/src/usr.bin/text/macros.d/RCS/Makefile,v 1.1 89/03/27 18:26:09 root Exp $
# $Log:	Makefile,v $
# Revision 1.1  89/03/27  18:26:09  root
# Initial check-in for 3.7
# 
# Revision 1.7  87/08/25  22:27:55  andre
# we don't chmod anymore with new install script
# 
# Revision 1.6  87/05/29  16:34:31  andre
# cd ..
# 
# Revision 1.5  87/05/04  11:08:15  andre
# 3.6 Product, New 5.3 Style Installs Added, Native Installs done
# 
# Revision 1.2  86/02/13  13:41:20  root
# - donl
# 
# Revision 1.1  86/02/13  13:29:16  root
# Initial revision
# 
# Revision 1.6  84/12/12  13:35:30  bob
# Fixed so that $(MACS) depends on $(MACS).src.
# 

M	= /usr/lib/macros
T	= /usr/lib/tmac
E	= /usr/lib/me
TMACS	= tmac.an tmac.an6 tmac.e tmac.m tmac.osd tmac.ptx tmac.s tmac.scover \
	  tmac.sdisp tmac.skeep tmac.srefs tmac.v tmac.vcat

MACS	= an an6 mmn mmt osdd vmca

EMACS	= acm.me chars.me deltext.me eqn.me float.me footnote.me index.me \
	  local.me null.me sh.me tbl.me thesis.me revisions

all:	$(TMACS) $(MACS) $(EMACS)

install:all $M $T $E
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib/macros \
	-idb "std.sw.unix mr" "${MACS}"
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib/tmac \
	-idb "std.sw.unix mr" "${TMACS}"
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib/me \
	-idb "std.sw.unix mr" "${EMACS}"

clean:
	rm -f ntar ntar.o

clobber:clean
	rm -f $(MACS)

$M $T $E:
	${INSTALL} -dir ${M} -u bin -g bin -m 755 \
	-idb "std.sw.unix mr" 
	${INSTALL} -dir ${T} -u bin -g bin -m 755 \
	-idb "std.sw.unix mr" 
	${INSTALL} -dir ${E} -u bin -g bin -m 755 \
	-idb "std.sw.unix mr" 

#	Compile on Source machine to run as part of compilation process
ntar:	ntar.c
	cc -n -O -s -I${ROOT}/usr/include -o $@ ntar.c

$(MACS):ntar $$@.src
	cp macrunch macrunch.bak
	chmod +x macrunch.bak
	./macrunch.bak  $@
