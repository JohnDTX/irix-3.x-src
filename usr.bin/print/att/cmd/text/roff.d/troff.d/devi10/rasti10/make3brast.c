/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/* This program translates a Imagen binary font in VAX format to 3b20 format.
It should be run on a 3b20 with vax fonts as input.
Input file is indicated by argument 1
Standard output is the converted file
Option "-f" forces conversion despite incorrect magic number for VAX
*/
#include <stdio.h>

FILE	*fd;		/*file descriptor for input file*/

main(argc,argv)
int argc;
char **argv;
{
#ifndef vax
	int i,j; /*counters */
	unsigned int x[2]; /*holds two chars */
	union u_mag{
		char ch[2];
		short s;
	} magic;
	int forcesw = 0;

	if (strcmp(argv[1], "-f") == 0) {
		forcesw++;
		argv++;  argc--;
	}
	if (argc < 2)
	{
		fprintf(stderr,"usage -- missing font name\n");
		exit(1);
		/*no font name provided */
	}
	if((fd=fopen(argv[1],"r")) == 0)
	{
		fprintf(stderr," font file does not exist %s \n",argv[1]);
		exit(0);
	}
	magic.ch[0] = getc(fd);
	magic.ch[1] = getc(fd);
	if (magic.s != 7681) {			/*magic number for a VAX file*/
		if (magic.s == 286)		/*magic number for a 3B file*/
			fprintf(stderr, "Input file is already made for 3B\n");
		else
			fprintf(stderr, "Unrecognized magic number\n");
		if (forcesw == 0)
			exit(2);
	}
	putchar(magic.ch[1]);
	putchar(magic.ch[0]);
	for(i=0;i<4;i++)
	{
		flipbyte(); /*the next eight bytes are swabed,
				correcting the global header */
	}
		fillbyte(2); /*two fill bytes are inserted */
	for(j=0;j<256;j++) /*this involves corecting an array of 256 structures
			      which describe each character */
	{
		flipbyte(); /* swab the bytes */
		flipbyte();
		for(i=0;i<6;i++)
		{
			putbyte(); /* put out six chars */
		}
		fillbyte(2); /* put out two fill bytes */
	}
	while((x[0]=getc(fd)) != EOF) /* read and then write out the bit map 1
				byte at a time */
	{
		putchar(x[0]);
	}
	fclose(fd);
}
flipbyte() /* swabs a short int */
{
	unsigned int y[2];
	y[1]=getc(fd);
	y[0]=getc(fd);
	putchar(y[0]);
	putchar(y[1]);
}
putbyte() /*puts out a byte */
{
	unsigned int y;
	y=getc(fd);
	putchar(y);
}
fillbyte(n) /*puts out N null bytes */
int n;
{
	unsigned int x = '\0';
	int i;
	for(i=0;i<n;i++)
	{
		putchar(x);
	}
#endif
}	
