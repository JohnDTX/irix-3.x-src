
include ${ROOT}/usr/include/make/commondefs

MEXIMGE = Makefile cat.bw cube2.di logo.bw randcubes.di sepcube.di trip5.di 

MEXIMGLIB = close.c filbuf.c flsbuf.c memimg.c \
	    name.c open.c pix.c rdwr.c rle.c row.c scrsave.c 

MEXIMGTOOLS = capture.c clip.c dither.c \
	hist.c imged.c ipaste.c iset.c istat.c loadmap.c mapimg.c melt.c \
	movie.c readimage.c rle.c savemap.c showci.c shrink.c snap.c \
	tobw.c verbatim.c 

MEXINCLUDE = image.h parsetab.h port.h rect.h 

MEXRCS = Makefile tools.mexrc*

MEXPORT = colormod.c fontlib.c gamma.c getapixel.c getset.c ipc.c parse.c \
	percent.c rectlib.c redraw.c subview.c turtle.c 


MEXTOOLS = blanktime.c cbal.c cedit.c clock.c curved.c fade.c fed.c gamcal.c \
	gamma.c gexec.c ical.c interp.c ismex.c keyboard.c loadav.c \
	loadfont.c mag.c makemap.c mouse.c mousemon.c mousewarp.c paint.c \
	palette.c randmap.c savedesktop.c scribble.c showmap.c showpie.c \
	showramp.c spiral.c stars.c startup* sunflower.c texback.c \
	textcolors.c vis.c worms.c zoing.c 

all:
	cd mextools; make

clobber clean:
	cd mextools; make clean

boot:
	cd mextools/imglib; make install

install:
	cd mextools; make install
	${INSTALL} -idb "gifts" -F /usr/people/gifts/mextools \
		-src mextools/Makefile.gifts Makefile
	cd mextools; ${INSTALL} -idb "gifts" -F /usr/people/gifts/mextools \
		"README Notice "
	cd mextools/images; ${INSTALL} -idb "gifts" \
		-F /usr/people/gifts/mextools/images  "${MEXIMGE}"
	cd mextools/imglib; ${INSTALL} -idb "gifts" \
		-F /usr/people/gifts/mextools/imglib  "${MEXIMGLIB}"
	cd mextools/imglib; ${INSTALL} -idb "gifts" -src Makefile.gifts \
		-F /usr/people/gifts/mextools/imglib  Makefile
	cd mextools/imgtools; ${INSTALL} -idb "gifts" \
		-F /usr/people/gifts/mextools/imgtools  "${MEXIMGTOOLS}"
	if test '$(machine)' = iris ;\
	    then  ${INSTALL} -idb "gifts" -src mextools/imgtools/Makefile.PM2 \
		-F /usr/people/gifts/mextools/imgtools Makefile ;\
	    else  ${INSTALL} -idb "gifts" -src mextools/imgtools/Makefile.IP2 \
		-F /usr/people/gifts/mextools/imgtools Makefile ; fi
	cd mextools/include; ${INSTALL} -idb "gifts" \
		-F /usr/people/gifts/mextools/include  "${MEXINCLUDE}"
	cd mextools/include; ${INSTALL} -idb "gifts" -src Makefile.gifts \
		-F /usr/people/gifts/mextools/include  Makefile
	cd mextools/mexrcs; ${INSTALL} -idb "gifts" \
		-F /usr/people/gifts/mextools/mexrcs  "${MEXRCS}"
	cd mextools/portlib; ${INSTALL} -idb "gifts" \
		-F /usr/people/gifts/mextools/portlib  "${MEXPORT}"
	if test '$(machine)' = iris ;\
	    then  ${INSTALL} -idb "gifts" -src mextools/portlib/Makefile.PM2 \
		-F /usr/people/gifts/mextools/portlib Makefile ;\
	    else  ${INSTALL} -idb "gifts" -src mextools/portlib/Makefile.IP2 \
		-F /usr/people/gifts/mextools/portlib Makefile ; fi
	cd mextools/tools; ${INSTALL} -idb "gifts" \
		-F /usr/people/gifts/mextools/tools  "${MEXTOOLS}"
	if test '$(machine)' = iris ;\
	    then  ${INSTALL} -idb "gifts" -src mextools/tools/Makefile.PM2 \
		-F /usr/people/gifts/mextools/tools Makefile ;\
	    else  ${INSTALL} -idb "gifts" -src mextools/tools/Makefile.IP2 \
		-F /usr/people/gifts/mextools/tools Makefile ; fi
