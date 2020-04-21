/*
 * Font stuff.  Mostly stolen from paul.
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/fntload.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:22 $
 */
#include "tf.h"
#include "gl.h"

#define	FONTLIB		"/usr/lib/gl2/fonts"
#define FONTMAGIC	4345

/*
 * Try open a file name on a given path
 */
static int
pathopen(name, how)
	char *name;
	int how;
{
	register char *cp;
	register char *path;
	char fn[1000];
	extern char *getenv();
	extern char *strchr();
	int fd;

	path = getenv("GFXPATH");
	if (!path || (*path == '\0'))
		return (open(name, how));

	cp = path;
	while (cp) {
		if (cp = strchr(path, ':'))
			*cp = 0;
		if ((cp && (cp != path)) || (!cp && *path)) {
			/*
			 * Only prepend the path prefix if there is one
			 * and its of non-zero length.  Null length paths
			 * hit the other case below.
			 */
			strcpy(fn, path);
			strcat(fn, "/");
			strcat(fn, name);
		} else
			strcpy(fn, name);
		if ((fd = open(fn, how)) >= 0)
			return (fd);
		path = cp + 1;
	}
	return (-1);
}

/*
 * Read a font in, from the given file descriptor
 */
static int
readfont(file, ht, nc, chars, nr, raster)
	int file;
	short *ht, *nc, *nr;
	Fontchar chars[];
	short raster[];
{
	long magic;

	lseek(file, 0L, 0);
	read(file, &magic, sizeof(long));
	if (magic != FONTMAGIC)
		return (1);
	read(file, &magic, sizeof(long));
	read(file, ht, sizeof(short));
	read(file, nc, sizeof(short));
	read(file, nr, sizeof(short));
	read(file, chars, *nc * sizeof(Fontchar));
	read(file, raster, *nr *sizeof(short));
	return (0);
}

int
fntload(filename, n)
	char *filename;
	short n;
{
	short ht, nc, nr;
	Fontchar *chars;
	short *raster;
	int file;

	if ((file = pathopen(filename, 0)) < 0) {
		char buf[1000];

		strcpy(buf, FONTLIB);
		strcat(buf, "/");
		strcat(buf, filename);
		if ((file = open(buf, 0)) < 0)
			return (1);
	}

	chars = MALLOC(Fontchar *, 256 * sizeof(Fontchar));
	raster = MALLOC(short *, 256 * 256);
	if ((chars == 0) || (raster == 0)) {
		close(file);
		return (1);
	}
	readfont(file, &ht, &nc, chars, &nr, raster);
	defrasterfont(n, ht, nc, chars, nr, raster);
	FREE(chars);
	FREE(raster);
	close(file);
	return (0);
}
