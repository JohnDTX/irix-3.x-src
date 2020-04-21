#ifdef notdef
/*
 * Font stuff.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/font.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:45:52 $
 */
#include "gsh.h"
#include "gl.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#ifdef notdef
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
	if ((fd = fontopen(flag_font, 0)) < 0) {
		return (0);
	}

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
	defrasterfont(1, 0, 0, 0, 0, 0);
	readfont(fd, &ht, &nc, chars, &nr, raster);
	defrasterfont(1, ht, nc, chars, nr, raster);
	font(1);

	/*
	 * Figure out size of font
	 */
	charheight = ht;
	charwidth = gl_findwidth(chars, nc);
	chardescender = gl_finddescender(chars, nc);
	return (1);
}
#endif

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
	strcpy(name, filename);
	strcat(name, ".fnt");
	if ((fd = open(name, how)) >= 0)
		return (fd);

	/*
	 * Try looking in the library
	 */
	strcpy(name, fontlib);
	strcat(name, "/");
	strcat(name, filename);
	if ((fd = open(name, how)) >= 0)
		return (fd);

	/*
	 * Try looking in the library AND appending ".fnt" to the name
	 */
	strcpy(name, fontlib);
	strcat(name, "/");
	strcat(name, filename);
	strcat(name, ".fnt");
	return (open(name, how));
}

setfont(linep)
char	*linep;
{
	char	*lp;
	
	lp = strtok(NULL, " \t\n");
	flag_font = (char *) malloc(strlen(lp) + 1);
	strcpy(flag_font, lp);
}
int
setup_font()
{
    int fd, rv;

    /*
     * Try to open up the font file. 
     */
    if ((fd = fontopen(flag_font, O_RDONLY)) < 0) {
	return (0);
    }

    /*
     * Read the font in from the file and tell the gl about it.  GL doesn't
     * allow use to redefine font 0. 
     */
    if (loadfont(fd, 1, &charheight, &charwidth, &chardescender)) {
	fprintf(stderr, "loadfont failed\n");
	rv = 0;
    }
    else {
	font(1);
	rv = 1;
    }
#if 0
    errorm('w', "ht=%d wid=%d dcr=%d", font_height, font_width, font_descender);
#endif
    close(fd);
    return (rv);
}

#ifdef notdef
#define FONTMAGIC	4345

/* stolen from paul */
int
loadfont(file, font_nr, ht, wid, dsndr)
int file, font_nr;
short *ht, *wid, *dsndr;
{
    long magic, dummy;
    short nc, nr;
    Fontchar *chars;
    short *raster;

    lseek(file, 0L, 0);
    read(file, (char *) &magic, sizeof(long));
    if (magic != FONTMAGIC)
	return (1);		/* oops */
    read(file, (char *) &dummy, sizeof(long));
    read(file, (char *) ht, sizeof(short));
    read(file, (char *) &nc, sizeof(short));
    read(file, (char *) &nr, sizeof(short));
    /*
     * Allocate memory for the font. 
     */
    if ((chars = (Fontchar *) malloc(nc * sizeof(Fontchar))) == (Fontchar *) 0)
	return (1);
    if ((raster = (short *) malloc(nr * sizeof(short))) == (short *) 0) {
	free((char *) chars);
	return (1);
    }
    read(file, (char *) chars, (unsigned) (nc * sizeof(Fontchar)));
    read(file, (char *) raster, (unsigned) (nr * sizeof(short)));
    defrasterfont(font_nr, *ht, nc, chars, nr, raster);
    /*
     * Figure out size of font 
     */
    *wid = gl_findwidth(chars, nc);
    *dsndr = gl_finddescender(chars, nc);
    free((char *) raster);
    free((char *) chars);
    return (0);
}
#endif
#endif
