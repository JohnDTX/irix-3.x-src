include ${ROOT}/usr/include/make/commondefs

LCINCS = -I../include -I${ROOT}/usr/include -I${ROOT}/usr/include/gl2
LCOPTS = -O -D$$CPUBOARD -DUNIX
LLDLIBS = -lgl -lm
LIBS = ../imglib/libimage.a ../portlib/libport.a 
ALL = capture clip dither hist imged ipaste iset istat loadmap mapimg \
	melt movie readimage rle savemap showci shrink snap tobw verbatim

all: $(ALL)

clean:
	rm -f $(ALL) *.o

install: all

.c:
	${CCF} -o $@ $< $(LIBS) ${LDFLAGS}
