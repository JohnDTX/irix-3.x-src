/*
 * Font stuff.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/gsh/RCS/font.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:46:29 $
 */
#include "gsh.h"
#include "gl.h"
#include "string.h"

#define FONTMAGIC	4345

/* stolen from paul */
int
readfont(file, ht, nc, chars, nr, raster)
	int file;
	short *ht, *nc, *nr;
	Fontchar chars[];
	short raster[];
{
	long magic;

	lseek(file, 0, 0);
	read(file, &magic, sizeof(long));
	if (magic != FONTMAGIC)
		return (1);			/* oops */
	read(file, &magic, sizeof(long));
	read(file, ht, sizeof(short));
	read(file, nc, sizeof(short));
	read(file, nr, sizeof(short));
	read(file, chars, *nc * sizeof(Fontchar));
	read(file, raster, *nr * sizeof(short));
	return (0);
}

/*
 * Load the given font named in flag_font
 */
int
setup_font()
{
	static Fontchar *chars;
	static short *raster;
	short ht, nc, nr;
	int fd;

	/*
	 * Try to open up the font file.
	 */
	if ((fd = fontopen(flag_font, 0)) < 0)
		return (0);

	/*
	 * Allocate memory for the font.
	 */
	if (chars == NULL)
		chars = (Fontchar *) malloc(256*sizeof(Fontchar));
	if (raster == NULL)
		raster = (short *) malloc(256*256);
	if ((chars == NULL) || (raster == NULL))
		return (0);

	/*
	 * Delete the old font #1, then read the font in from the
	 * file and then tell the gl about it.
	 */
	if (readfont(fd, &ht, &nc, chars, &nr, raster))
		return (0);
	defrasterfont(1, 0, 0, (Fontchar *)0, 0, (short *)0);
	defrasterfont(1, ht, nc, chars, nr, raster);
	font(1);

	/*
	 * Figure out size of font
	 */
	font_height = ht;
	font_width = gl_findwidth(chars, nc);
	font_descender = gl_finddescender(chars, nc);
	return (1);
}

/*
 * Open up a font file.  This knows about FONTLIB
 */
int
fontopen(filename, how)
	char *filename;
	int how;
{
	char name[1000];
	int fd;

	/*
	 * First see if user gave a legitimit file name.
	 */
	if ((fd = open(filename, how)) >= 0)
		return (fd);

	/*
	 * Try appending ".fnt" to the name
	 */
	(void) strcpy(name, filename);
	(void) strcat(name, ".fnt");
	if ((fd = open(name, how)) >= 0)
		return (fd);

	/*
	 * Try looking in the library
	 */
	(void) strcpy(name, fontlib);
	(void) strcat(name, "/");
	(void) strcat(name, filename);
	if ((fd = open(name, how)) >= 0)
		return (fd);

	/*
	 * Try looking in the library AND appending ".fnt" to the name
	 */
	(void) strcpy(name, fontlib);
	(void) strcat(name, "/");
	(void) strcat(name, filename);
	(void) strcat(name, ".fnt");
	return (open(name, how));
}

#ifdef	R2300
gl_finddescender(chars, nc)
Fontchar *chars;
long nc;
{
    int i,descender;

    descender = 0;
    for (i = 0; i < nc; i++) {
	if (descender > chars->yoff)
	    descender = chars->yoff;
	chars++;
    }
    return(-descender);
}


gl_findwidth(chars, nc)
Fontchar *chars;
long nc;
{
    short i,width;

    width = 0;
    for (i = 0; i < nc; i++) {
	if (width < chars->width )
	    width = chars->width;
	chars++;
    }
    return(width);
}
#endif
