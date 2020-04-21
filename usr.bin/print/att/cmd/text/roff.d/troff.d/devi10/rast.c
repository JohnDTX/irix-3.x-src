/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 *
 * A few routines that read and process raster files supplied by Imagen.
 *
 * Impress, at least at the time these routines were written, only allowed
 * us to use families 0 to 95. That imposes a limit on the total number of
 * raster files that any one job can use, unless we do some kind of memory
 * management, and that in turn restricts the number of entries in array
 * fam_data[] to 96. Unless this limit is changed MAXFAMILY should never be
 * defined to be greater than 95.
 *
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ext.h"			/* external variable definitions */
#include "gen.h"			/* definitions used by everyone */
#include "rast.h"			/* raster file definitions */
#include "impcodes.h"			/* Impress 2.0 opcodes */


Rst		rst[] = RST_INIT;	/* describes raster file structure */

Rastdata	fam_data[MAXFAMILY+1];	/* data about raster files we've read */

int		next_fam = 0;		/* next free spot in fam_data[] */
int		cur_fam = 0;		/* family number we're using right now */
int		last_fam = -1;		/* last one we told printer about */
Rastdata	*fam = NULL;		/* &fam_data[cur_fam] */

int		rast_res = RAST_RES;	/* raster table resolution */


/*****************************************************************************/


getrastdata(f, s)


    char	*f;			/* use this font's raster file */
    int		s;			/* in this point size */


{


    int		i;			/* loop index */


/*
 *
 * Called to find the raster file info for font f, size s in fam_data[].
 * If it's not there it will be read in from the appropriate file in
 * *rastdir and all the required info will be added to location
 * next_fam in fam_data[].
 *
 * I'm assuming that each raster file contains at most 128 glyphs. If
 * that's not the case we'll need to change the way things are done
 * because Impress only allows 128 members in each family. Brian took
 * account of of larger raster files in di10.c and it wouldn't be all
 * that difficult to do it here too, but I don't think it's necessary.
 *
 */


    for ( i = 0; i < next_fam; i++ )	/* got any info for *rastdir/f.s */
	if ( fam_data[i].size == s  &&  strcmp(f, fam_data[i].name) == 0 )
	    break;

    cur_fam = i;			/* family we'll be using next */
    fam = &fam_data[i];			/* its entry in fam_data[] */

    if ( i >= next_fam )  {		/* didn't find it */
	if ( next_fam > MAXFAMILY )	/* only use families 0 to 95 */
	    error(FATAL, "job uses too many raster files");

	strncpy(fam->name, f, L_FNAME);	/* initialze some of the fields */
	fam->name[L_FNAME-1] = '\0';
	fam->size = s;
	fam->rst = NULL;
	fam->chused = (unsigned char *) rastalloc((MAXMEMBER+1)/BYTE, TRUE);
	fam->advance = (int *) rastalloc((MAXMEMBER+1) * sizeof(int), FALSE);

	next_fam++;			/* can only use it once */
    }   /* End if */

    if ( fam->rst == NULL )		/* need to read the whole raster file */
	readrastfile(f, s);

}   /* End of getrastdata */


/*****************************************************************************/


readrastfile(f, s)


    char	*f;			/* name of the raster file */
    int		s;			/* in this point size */


{


    char	name[100];		/* raster file pathname built up here */
    int		fd;			/* file descriptor for *name */
    struct stat buf;			/* used to get raster file size */


/*
 *
 * Reads the raster file for font f in size s from directory *rastdir
 * and initializes some of the required fields in fam_data[cur_fam].
 *
 * First we try to open the appropriate raster file. Then we determine
 * its size in bytes and try to allocate enough memory for the whole file.
 * After we've successfully read it we close the file and then initialize
 * the rest of the fields in fam_data[cur_fam].
 *
 * I've made allowances for two types of raster file naming conventions.
 * The most common is the one that troff's post-processor will be using.
 * The raster file names all begin with the string "font." and end with
 * the suffix size. For example the raster table for the Roman font in size
 * 10 would be called R.10. The other convention is the one you'll find
 * in all the raster files supplied directly from Imagen (as of 9/15 anyway).
 * In this case the file names all begin with the prefix string "*font.r"
 * and again end with the size. For example the raster file for font cmti
 * in size 12 would be cmti.r12. Since this routine is used by programs that
 * expect both conventions I've written it to try the two different raster
 * file names before giving up.
 *
 */


    sprintf(name, "%s/%s.%d", rastdir, f, s);
    if ( (fd = open(name, 0)) == -1 )  {
	sprintf(name, "%s/%s.r%d", rastdir, f, s);
	if ( (fd = open(name, 0)) == -1 )
	    error(FATAL, "can't open raster file for font %s, size %d", f, s);
    }	/* End if */

    if ( fstat(fd, &buf) == -1 )
	error(FATAL, "can't get file size for %s", name);

    fam->rst = rastalloc(buf.st_size, FALSE);

    if ( read(fd, fam->rst, buf.st_size) != buf.st_size )
	error(FATAL, "can't read raster file %s", name);

    close(fd);				/* all done with it - for now anyway */

    fam->glyphdir = getvalue(P_GLYDIR);
    fam->first = getvalue(P_FIRSTGLY);
    fam->last = getvalue(P_LASTGLY);

    if ( (fam->mag = getvalue(P_MAG)) <= 0 )
	fam->mag = 1000;

}   /* End of readrastfile */


/*****************************************************************************/


freerast(count)


    int		count;			/* max number of calls to free() */


{


    int		called;			/* number of calls made so far */
    int		i;			/* loop index */


/*
 *
 * Tries to free all the memory used to store raster file data for
 * at most count entires in array fam_data[]. The actual number of
 * calls to free() is returned to the caller. A return value of 0
 * obviously means there's no more memory to release. We don't want to
 * free the memory used to keep track of the characters we've downloaded
 * because we may want to use this raster file later on and we'll need to
 * know which glyphs have been downloaded.
 *
 * Although I don't expect to use this routine often, some of the
 * raster files are huge and there will undoubtedly be a few jobs
 * that won't be able to keep all the required raster data in memory
 * at the same time.
 *
 */


    for ( called = 0, i = 0; i < next_fam  &&  called < count; i++ )
	if ( fam_data[i].rst != NULL )  {
	    free(fam_data[i].rst);
	    fam_data[i].rst = NULL;
	    called++;
	}   /* End if */

    return(called);

}   /* End of freerast */


/*****************************************************************************/


resetrast()


{


    int		i;			/* loop index */


/*
 *
 * Called when we want to clear the fam_data[] array, free all the memory
 * associated with each raster file, and reset all the variables used to
 * access the raster file info. Not used when we're generating Impress
 * commands used to print files, but it may be useful when we build and
 * edit raster files and troff's ASCII font files - maybe it shouldn't
 * be in this file.
 *
 */


    for ( i = 0; i < next_fam; i++ )  {
	free(fam_data[i].chused);
	free(fam_data[i].rst);
	free(fam_data[i].advance);
	fam_data[i].name[0] = '\0';
	fam_data[i].size = 0;
    }	/* End for */

    next_fam = 0;
    cur_fam = 0;
    last_fam = -1;
    fam = NULL;

}   /* End of resetrast */


/*****************************************************************************/


char *rastalloc(size, init)


    unsigned	size;			/* allocate this many bytes */
    int		init;			/* initialize to zero if TRUE */


{


    char	*ptr;			/* block of memory we just got */
    int		i;			/* loop index for initialization */


/*
 *
 * Tries to allocate a block of memory that's large enough to be used for
 * storage of size bytes. If init is TRUE the memory will be initialized
 * to all 0's. It's assumed that the memory is going to be used to store
 * a raster file or other related data. We'll keep trying to get enough
 * memory as long as some is being used for other raster files.
 *
 */


    while ( (ptr = (char *) malloc(size)) == NULL )
	if ( freerast(1) == 0 )
	    error(FATAL, "can't allocate memory for raster file");

    if ( init == TRUE )			/* want to set it to zeros first */
	for ( i = 0; i < size; i++ )
	    *(ptr + i) = 0;

    return(ptr);

}   /* End of rastalloc */


/*****************************************************************************/


printchar(c, font, size, advance, fp)


    int		c;			/* print glyph for this character */
    char	*font;			/* in this font */
    int		size;			/* and this point size */
    int		advance;		/* using this as the advance width */
    FILE	*fp;			/* Impress commands go here */


{


/*
 *
 * Does everything needed to get character c printed using the raster file
 * for *font in point size 'size' and using advance as the character advance.
 * Right now it's only used by the program that dumps raster files to the
 * printer. The troff post-processor doesn't use this routine. It handles
 * the downloading, positioning, and printing of glyphs on its own.
 *
 */


    if ( fam == NULL || fam->size != size || strcmp(font, fam->name) != 0 || fam->rst == NULL )
	getrastdata(font, size);

    putc(download(c, advance, fp), fp);

}   /* End of printchar */


/*****************************************************************************/


download(c, advance, fp)


    int		c;			/* download this glyph? */
    int		advance;		/* using this as the advance width */
    FILE	*fp;			/* Impress commands written here */


{


    int		width;			/* raster width for c's bitmap */
    int		height;			/* of the bitmap in rasters */
    int		map_size;		/* bytes needed to store the bitmap */
    char	*bptr;			/* where the bitmap really starts */
    int		i;			/* loop index for downloading glyph */


/*
 *
 * Downloads glyph number c, using the raster file fam->rst and advance
 * as the character advance. All the Impress commands are written to FILE
 * *fp. If advance is greater than or equal to zero we'll assume the caller
 * wants to use the value as the character advance for the glyph, otherwise
 * we'll get the value from the raster file. Returns the member number in
 * the current family that corresponds to glyph c.
 *
 * Does much of the stuff handled by xychar() in di10.c except for the
 * positioning of the reference point and printing the member. I've tried
 * to make things a little more general so I'm assuming that whoever calls
 * it has done the appropriate positioning and will take care of printing
 * the glyph.
 *
 */


    if ( c < fam->first  ||  c > fam->last  ||  c > MAXMEMBER )
	error(FATAL, "glyph %d out of range for font %s", c, fam->name);

    if ( cur_fam != last_fam )  {	/* will be using a new family */
	putc(ASF, fp);
	putc(last_fam = cur_fam, fp);
    }	/* End if */

    if ( ! checkbit(fam->chused, c) )  {	/* first use of this glyph */
	setbit(fam->chused, c);		/* don't download it again */

	width = getvalue(G_WIDTH, c);	/* we'll need these values later */
	height = getvalue(G_HEIGHT, c);
	bptr = fam->rst + getvalue(G_BPTR, c);
	fam->advance[c] = (advance < 0 ) ? PIXEL_WIDTH(getvalue(G_CHWIDTH, c), rast_res) : advance;

	if ( (map_size = height * ((width + BYTE - 1) / BYTE)) > 0 )  {
	    putc(ABGLY, fp);		/* define the glyph */
	    putint((cur_fam << 7) | c, fp);
	    putint(fam->advance[c], fp);
	    putint(width, fp);
	    putint((int) getvalue(G_XREF, c), fp);
	    putint(height, fp);
	    putint((int) getvalue(G_YREF, c), fp);

	    for ( i = map_size; i--; )	/* followed by its bitmap */
		putc(*bptr++, fp);
	} else error(NON_FATAL, "Bad glyph - font %s, glyph %d", fam->name, c);
    }	/* End if */

    return(c);

}   /* End of download */


/*****************************************************************************/


unsigned getvalue(field, ch)


    int			field;		/* position in rst[] */
    int			ch;		/* character number - for glyphs */


{


    register int	n;		/* number of bytes in the field */
    register char	*p;		/* start of the field */
    register unsigned	value;		/* result after decoding field */


/*
 *
 * Called to figure out the value field in the current family's raster file.
 * Right now things don't work for strings, but they're not really important.
 *
 * You'll see quite a few calls to this routine without the parameter ch.
 * It's only used when we're accessing the glyph directory so I've omitted
 * passing a value when we're looking at the preamble.
 *
 */


    n = rst[field].size;		/* number of bytes we want to grab */

    p = fam->rst + rst[field].offset;	/* right except for glyph directory */

    if ( rst[field].glyphdir == TRUE )	/* it's really part of the directory */
	p += GLYPH_PTR(ch);

    value = (rst[field].type == INTEGER && (*p & 0200)) ? ~0 : 0;

    for ( ; n > 0; n--, p++ )
	value = (value << BYTE) | (*p & BMASK);

    return(value);

}   /* End of getvalue */


/*****************************************************************************/


checkbit(p, n)


    char	*p;			/* start of the bitmap */
    int		n;			/* see if this bit is set */


{


/*
 *
 * Returns the value (0 or 1) set in the n-th bit in the bitmap that
 * starts at address p. Primarily used to keep track of which glyphs
 * have already been downloaded, but it's also needed in the routine
 * that dumps bitmaps out to files. It probably should be defined as
 * a macro.
 *
 */


    return(((p[n / BYTE] >> ((BYTE - 1) - n % BYTE)) & 01));

}   /* End of checkbit */


/*****************************************************************************/


setbit(p, n)


    char	*p;			/* start of the bitmap - again */
    int		n;			/* bit that we want to set in p[] */


{


/*
 *
 * Turns on the n-th bit in the bitmap that starts at address p. This
 * routine is really only used to keep track of the glyphs that have
 * already been downloaded. Again it could easily be defined as a macro.
 *
 */


    p[n / BYTE] |= 01 << ((BYTE - 1) - n % BYTE);

}   /* End of setbit */


/*****************************************************************************/


dump_glyph(n, fp)


    int		n;			/* glyph number we want to display */
    FILE	*fp;			/* everything's written here */


{


    int		height;			/* pixel height of the bitmap */
    int		width;			/* width of the rows in pixels */
    int		xref;			/* x coordinate of reference point */
    int		yref;			/* same but for y coordinate */
    int		chwidth;		/* width of the character */
    int		rwid;			/* number of bytes per bitmap row */
    int		i, j;			/* loop indices for the bitmap */
    char	*bptr;			/* start of the glyph's bitmap */


/*
 *
 * Really just a debugging routine that's used to dump the bitmap and
 * other glyph data for character n out to FILE *fp.
 *
 */


    height = getvalue(G_HEIGHT, n);
    width = getvalue(G_WIDTH, n);
    xref = getvalue(G_XREF, n);
    yref = getvalue(G_YREF, n);
    chwidth = getvalue(G_CHWIDTH, n);
    bptr = fam->rst + getvalue(G_BPTR, n);

    rwid = (width + BYTE - 1) / BYTE;

    fprintf(fp, "GLYPH NUMBER %d  Character Width = %d\n", n, PIXEL_WIDTH(chwidth, rast_res));
    fprintf(fp, " Raster Data: height = %d, width = %d, xref = %d, yref = %d, chwidth = %d\n",height, width, xref, yref, chwidth);

    for ( j = 0; j < height; j++ )  {
	for ( i = 0; i < width; i++ )
	    if ( checkbit(bptr, j * rwid * BYTE + i) )
		putc('X', fp);
	    else putc('.', fp);
	putc('\n', fp);
    }   /* End for */

}   /* End of dump_glyph */


/*****************************************************************************/


