#
# $Source: /d2/3.7/src/stand/lib/gl/RCS/Makefile,v $
# $Revision: 1.1 $
# $Date: 89/03/27 17:15:07 $
#

# Local definitons

include ../Make.h

OFILES = \
	gl2gl.o \
	glbox.o \
	glchar.o \
	glclear.o \
	glcolor.o \
	glcurs.o \
	glfill.o \
	glinit.o \
	glxfont.o \
	screen.o \
	scrinit.o \
	term.o

default: all
	$(ECHO) done

all: $(OFILES)

clean:
	rm -f *.o

clobber:	clean


# Header file dependancies

gl2gl.o: $(INCL)/cpureg.h ./gl2dcdev.h ./gl2uc4.h ./gl2uctest.h ./gl2screen.h ./gl2gfdev.h ./gl2cmds.h ./glx.h

glbox.o: ./glx.h

glchar.o: ./glx.h

glclear.o: ./glx.h

glcolor.o: ./glx.h

glcurs.o: ./glx.h

glfill.o: ./glx.h

glinit.o: ./glx.h

glxfont.o: ./glx.h

screen.o: ./glx.h

scrinit.o: ./glx.h $(INCL)/cpureg.h $(INCL)/common.h

term.o: ./glx.h $(INCL)/dprintf.h
