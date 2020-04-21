/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/* EMACS_MODES: c, !fill */

#include <stdio.h>
#include "../glyph.h"

int debug = 0;

error(str)
char str[];
{
	printf("%s\n", str);
	exit(1);
}

readfh(fh)
struct Fontheader *fh;
{
	int cnt, c, tmp;
	
	cnt=scanf("magic: %o\n", &tmp);
	if (cnt == 0) error("out of phase: magic");
	fh->f_magic=tmp;
	cnt=scanf("size:  %d\n", &tmp);
	if (cnt == 0) error("out of phase: size");
	fh->f_size=tmp;
	cnt=scanf("max x: %d\n", &tmp);
	if (cnt == 0) error("out of phase: max x");
	fh->f_maxx=tmp;
	cnt=scanf("max y: %d", &tmp);
	if (cnt == 0) error("out of phase: max y");
	fh->f_maxy=tmp;
	c=getchar();
	if (c != '\n') error("out of phase: header too long");
}

readchp(chp, c, size)
struct Charparam *chp;
int *c;
int size;
{
	int cnt, ch, tmp, tmp1;
	
	ch=getchar();
	if (ch == EOF) return(1);
	if (ch != '\n') error("out of phase: nl between rasters");
	cnt=scanf("offset %d %x\n", &tmp, &tmp1);
	if (cnt == 0) error("out of phase: offset");
	if (tmp != size)  error("mismatching offsets");
	if (tmp != tmp1)  error("values don't match: offset");
	cnt=scanf("char:  %d\n", c);
	if (cnt == 0) error("out of phase: char");
	chp->c_addr=size;
	cnt=scanf("size:  %d %x\n", &tmp, &tmp1);
	if (tmp != tmp1)  error("values don't match: character size");
	if (cnt == 0) error("out of phase: character size");
	chp->c_size=tmp;
	cnt=scanf("up:    %d %x\n", &tmp, &tmp1);
	if (tmp != tmp1)  error("values don't match: up");
	if (cnt == 0) error("out of phase: up");
	chp->c_up=tmp;
	cnt=scanf("down:  %d %x\n", &tmp, &tmp1);
	if (tmp != tmp1)  error("values don't match: down");
	if (cnt == 0) error("out of phase: down");
	chp->c_down=tmp;
	cnt=scanf("left:  %d %x\n", &tmp, &tmp1);
	if (tmp != tmp1)  error("values don't match: left");
	if (cnt == 0) error("out of phase: left");
	chp->c_left=tmp;
	cnt=scanf("right: %d %x\n", &tmp, &tmp1);
	if (tmp != tmp1)  error("values don't match: right");
	if (cnt == 0) error("out of phase: right");
	chp->c_right=tmp;
	cnt=scanf("width: %d %x", &tmp, &tmp1);
	if (tmp != tmp1)  error("values don't match: width");
	chp->c_width=tmp;
	if (debug)
	{
		printf("char:  %d\n", *c);
		printf("size:  %d\n", chp->c_size);
		printf("up:    %d\n", chp->c_up);
		printf("down:  %d\n", chp->c_down);
		printf("left:  %d\n", chp->c_left);
		printf("right: %d\n", chp->c_right);
		printf("width: %d", chp->c_width);
	}
	return(0);
}

readcdp(cdp, chp, p)
char cdp[];
struct Charparam chp;
int p;
{
	int height, width;
 	int i, j, k;
	char work, c;

	height=(int)ckint(chp.c_up)+(int)ckint(chp.c_down)+1;
	width=(int)ckint(chp.c_left)+(int)ckint(chp.c_right)+1;
	c=getchar(); 
	if (debug) putchar(c);
	if (c != '\n') error("out of phase: before raster");
	for (i=0; i < height; i++)
	{
		k=0; work = 0;
		if (p == chp.c_addr+chp.c_size) break;
		for (j=0;  ; j++)
		{
			c=getchar(); 
			if (debug) putchar(c);
			if (c == '*') work |= 1;
			else if (c != ' ') error("out of phase: blank");
			k++; cdp[p]=work;
			if (j == width-1) break;
			if (k == 8) {k=0; ++p; work=0;}
			else work = work << 1;
		}
		c=getchar();
		if (debug) putchar(c);
		if (c != '\n') error("out of phase: after raster");
		cdp[p++]=work << (8-k);
	}
}

main(argc, argv)
int argc;
char *argv[];
{
	FILE *outfile;
	char filename[100];
	char fontname[100];
	char cdp[50000];
	struct Fontheader fh;
	struct Charparam chp [256];
	int r, size;
	
	if (argc != 2) error("fbuild: file?");
	strcpy(filename, argv[1]);
	outfile=fopen(filename, "w");
	if (outfile == NULL) 
	{
		printf("fdump: can't open %s", filename);
		error("");
	}

	scanf("font: %s\n", fontname);
	readfh(&fh);
	size=0;
	for (;;)
	{
		struct Charparam tempchp;
		int ch;
		
		r=readchp(&tempchp, &ch, size);
		if (r) break;
		if (tempchp.c_size == 0) continue;
		chp[ch]=tempchp;
		readcdp(cdp, chp[ch], size);
		size = size + (int)chp[ch].c_size;
	}

	if (fh.f_size != size)
	{
		printf("size differs: says %d   calculated %d\n",
			fh.f_size, size);
		fh.f_size=size;
	}
	
	fwrite(&fh, sizeof(struct Fontheader), 1, outfile);
	fwrite(&chp[0], sizeof(struct Charparam), 256, outfile);
	fwrite(cdp, fh.f_size, 1, outfile);
}


int ckint(n)
char n;
{
#if u3b || u3b15 || u3b5 || u3b2
	int i;
	if((int) n  > 0177 )
	{
		i = (int)n;
		i = (256 - i) * -1;
	}
	else
	{
	i = (int)n;
	}
	return(i);
#else
	return((int)n);
#endif
}
