/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/* EMACS_MODES: c, !fill */
/* this program will list out any binary font for imagen and will
thus allow the user to check any font for corectness
*/
#include <stdio.h>
#include "../glyph.h"

printfh(fh)
struct Fontheader fh;
{
	printf("magic: %o\n", fh.f_magic);
	printf("size:  %d\n", fh.f_size);
	printf("max x: %d\n", fh.f_maxx);
	printf("max y: %d\n", fh.f_maxy);
}

printchp(chp, c)
struct Charparam chp;
int c;
{
	printf("\n");
	printf("offset %d %x\n", chp.c_addr,chp.c_addr);
	printf("char:  %d\n", c);
	printf("size:  %d %x\n", chp.c_size, chp.c_size);
	printf("up:    %d %x\n", chp.c_up, chp.c_up);
	printf("down:  %d %x\n", chp.c_down, chp.c_down);
	printf("left:  %d %x\n", chp.c_left, chp.c_left);
	printf("right: %d %x\n", chp.c_right, chp.c_right);
	printf("width: %d %x\n", chp.c_width, chp.c_width);
}

printcdp(cdp, chp)
char cdp[];
struct Charparam chp;
{
	int height, width;
 	int i, j, k, p;
	char work;
	height=(int)ckint(chp.c_up)+(int)ckint(chp.c_down)+1;
	width=(int)ckint(chp.c_left)+(int)ckint(chp.c_right)+1;
	p=chp.c_addr-1;
	for (i=0; i < height; i++)
	{
		k=0; work = cdp[++p];
		if (p == chp.c_addr+chp.c_size) break;
		for (j=0;  ; j++)
		{
			if ((work & 0200) != 0) putchar('*');
			else putchar(' ');
			k++;
			if (j == width-1) break;
			if (k == 8) {k=0; work = cdp[++p];}
			else work = work << 1;
		}
		putchar('\n');
	}
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
main(argc, argv)
int argc;
char *argv[];
{
	FILE *infile;
	char filename[100];
	char cdp[50000];
	struct Fontheader fh;
	struct Charparam chp [256];
	int i;
	
	if (argc != 2) 
	{
		printf("fdump: file\n");
		exit(0);
	}
	strcpy(filename, argv[1]);
	infile=fopen(filename, "r");
	if (infile == NULL) 
	{
		printf("fdump: can't open %s\n", filename);
		exit(0);
	}
	fread(&fh, sizeof(struct Fontheader), 1, infile); 
	fread(&chp[0], sizeof(struct Charparam), 256, infile);
	fread(cdp, fh.f_size, 1, infile);

	printf("font: %s\n", argv[1]);
	printfh(fh);
	for (i=0; i<256; i++)
	{
		if (chp[i].c_size == 0) continue;
		printchp(chp[i], i);
		printcdp(cdp, chp[i]);
	}
}
