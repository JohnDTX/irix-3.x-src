static char ID[] = "@(#)instr.c	1.1";

/*
 *	Prints instructions on the terminal
 */

#include <stdio.h>
#define EXPFILE "/usr/games/mmhow"

instr()
{
	FILE *expfile;
	register int c;

	if ((expfile = fopen (EXPFILE, "r")) == NULL) {
		printf ("I couldn't get the explanation file %s\n", EXPFILE);
		return;
	}
	while ((c = getc (expfile)) != EOF)
		putchar (c);
	if (fclose (expfile) == EOF)
		printf ("I couldn't close the explanation file\n");
}
