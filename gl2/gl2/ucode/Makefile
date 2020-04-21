
include ${ROOT}/usr/include/make/commondefs
#

# Makefile for GF2 microcode
#

MAS      = ./mas
CODE	 = ./src
CODEP	 = $(CODE)/polygons
CODES	 = $(CODE)/scanconvert
SMALLMK  = Makefile.noz


LCINCS  = -I${ROOT}/usr/include 
SRCS =
HDRS = 
OBJS = gf2

GF2MICS = \
	$(CODE)/dispatch.o\
	$(CODE)/lowmem.o\
	$(CODE)/block.o\
	$(CODE)/buffcopy.o\
	$(CODE)/dbzline.o\
	$(CODE)/stm.o\
	$(CODE)/points.o\
	$(CODE)/attributes.o\
	$(CODE)/modes.o\
	$(CODE)/hitrept.o\
	$(CODE)/char.o\
	$(CODE)/chdraw.o\
	$(CODE)/diag.o \
	$(CODE)/cursor.o\
	$(CODE)/pixels.o\
	$(CODE)/mvppixels.o\
	$(CODE)/readpixels.o\
	$(CODE)/fixchar.o\
	$(CODE)/vectors.o\
	$(CODE)/divide.o\
	$(CODE)/ldivide.o\
	$(CODE)/viewport.o\
	$(CODE)/copypixels.o\
	$(CODE)/copyfont.o\
	$(CODE)/depthvec.o\
	$(CODE)/zvectors.o\
	$(CODE)/readrun.o\
	$(CODE)/runlen.o

POLYMICS = \
		$(CODEP)/addsentinels.o\
		$(CODEP)/advance.o\
		$(CODEP)/dbzfillline.o\
		$(CODEP)/dbzshadeline.o\
		$(CODEP)/bumpindex.o\
		$(CODEP)/checkmask.o\
		$(CODEP)/computeslope.o\
		$(CODEP)/filltrapzoid.o\
		$(CODEP)/finishside.o\
		$(CODEP)/getfrstsds.o\
		$(CODEP)/polyclose.o\
		$(CODEP)/polydraw.o\
		$(CODEP)/polyexit.o\
		$(CODEP)/shadeline.o\
		$(CODEP)/shadetrap.o\
		$(CODEP)/shfinishside.o\
		$(CODEP)/shgetfrstsds.o\
		$(CODEP)/shstartside.o\
		$(CODEP)/shtraploop.o\
		$(CODEP)/startside.o\
		$(CODEP)/swap.o\
		$(CODEP)/traploop.o\
		$(CODEP)/zfillline.o\
		$(CODEP)/zfilltrap.o\
		$(CODEP)/zfinishside.o\
		$(CODEP)/zgetfrstsds.o\
		$(CODEP)/zshadeline.o\
		$(CODEP)/zshadetrap.o\
		$(CODEP)/zstartside.o\
		$(CODEP)/ztraploop.o

SCANMICS = \
		$(CODES)/zscanline.o

MACHOBJS =	$(MAS)/main.o\
		$(MAS)/libh.o\
		$(MAS)/endstate.o\
		$(MAS)/defines.o\
		$(MAS)/symbols.o\
		$(MAS)/fbc.o\
		$(MAS)/version.o

WSOBJS	= $(MACHOBJS) $(MAS)/masws.o

VAXOBS = $(MACHOBJS) $(MAS)/masvax.o

default:
	@echo targets are ws, vax, smallws, smallvax, and new.

all:	ws

ws: subws ws.x
	./ws.x

install: all
	${INSTALL} -u root -g sys -m 644 -F /usr/lib/gl2 \
		-idb "std.sw.unix mr" ucode.o

subws:
	cd mas; make ws
	cd src; make
	cd $(CODEP); make
	cd $(CODES); make

ws.x: $(WSOBJS) $(GF2MICS) $(POLYMICS) $(SCANMICS)
	rm -f ws.x
	$(CCF) $(WSOBJS) $(GF2MICS) $(POLYMICS) $(SCANMICS) -o ws.x $(LDFLAGS) 

smallws: subsmallws sws.x
	sws.x

subsmallws:
	cd mas; make -f $(SMALLMK) ws
	cd src; make -f $(SMALLMK)
	cd $(CODEP); make -f $(SMALLMK)
	cd $(CODES); make

sws.x: $(WSOBJS) $(GF2MICS) $(POLYMICS) $(SCANMICS)
	rm -f sws.x
	$(CCF) $(WSOBJS) $(GF2MICS) $(POLYMICS) $(SCANMICS) -o sws.x $(LDFLAGS) 

vax: subvax vax.x
	vax.x

subvax:
	cd mas; make vax
	cd src; make
	cd $(CODEP); make
	cd $(CODES); make

vax.x: $(VAXOBJS) $(GF2MICS) $(POLYMICS) $(SCANMICS)
	rm -f vax.x
	$(CCF) $(VAXOBS) $(GF2MICS) $(POLYMICS) $(SCANMICS) -o vax.x $(LDFLAGS) 

smallvax: subsmallvax svax.x
	svax.x

subsmallvax:
	cd mas; make -f $(SMALLMK) vax
	cd src; make -f $(SMALLMK)
	cd $(CODEP); make -f $(SMALLMK)
	cd $(CODES); make

svax.x: $(VAXOBJS) $(GF2MICS) $(POLYMICS) $(SCANMICS)
	rm -f svax.x
	$(CCF) $(VAXOBS) $(GF2MICS) $(POLYMICS) $(SCANMICS) -o svax.x $(LDFLAGS) 

xcp:
	xcp ucode.o olympus:/oh4/hdwr/diag/gf/gf2pm2/betacode.b
	xcp ucode.sym olympus:/oh4/hdwr/diag/gf/gf2pm2/betacode.sym

new: vax xcp

# remove files that can be re-created

clean clobber:
	cd mas; make clean
	cd src; make clean
	cd $(CODEP); make clean
	cd $(CODES); make clean
	rm -f $(OBJS) *.out *.CKP *.BAK .emacs* *.o *.x *.b *.sym

