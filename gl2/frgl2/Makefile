FFLAGS = -C -O -u -Nx400
CFLAGS = -O
LIBPRIM = $(ROOT)/usr/lib/$(GLHDWR)/lib.prim
#LIBPRIM = ../gl2/lib.prim

all:

install: lib.f

mandefs.fr lib.f: frlibgen.awk $(LIBPRIM)
	awk -f frlibgen.awk $(LIBPRIM)
# take care of comment lines that spill past column 72.
	awk 'length<72 {print;next} length >72 {print substr($$0,1,50); \
		print "C     ",substr($$0,51,length);}' lib.f > /tmp/lib.f
	mv /tmp/lib.f lib.f

clobber clean:
	rm -f lib.f mandefs.fr

