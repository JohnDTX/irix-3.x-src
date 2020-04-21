
include ${ROOT}/usr/include/make/commondefs

LCINCS = -I../include -I${ROOT}/usr/include -I$(ROOT)/usr/include/gl2
LCOPTS  = -O -D$$CPUBOARD -DUNIX
LLDLIBS = -lgl -lm
LIBS = ../imglib/libimage.a ../portlib/libport.a 
IDBTAG = -idb "std.sw.unix"
GRPOWN = -g bin -u bin

ALL = 	blanktime cbal cedit clock curved fade fed gamcal gamma \
	gexec ical interp ismex keyboard loadav loadfont mag makemap mouse \
	mousemon mousewarp paint palette randmap savedesktop scribble showmap \
	showramp showpie spiral stars sunflower texback textcolors vis worms \
	zoing scribble

INSTALL_LIST =	blanktime loadfont textcolors

all: $(ALL)

clean:
	rm -f $(ALL) *.o

install: all
	${INSTALL} ${GRPOWN} -m 755 ${IDBTAG} -F /usr/bin "${INSTALL_LIST}"

.c:
	$(CCF) -o $@ $< ${LIBS} $(LDFLAGS)
