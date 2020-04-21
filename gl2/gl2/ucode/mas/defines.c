/* defines.c
 */

#include <stdio.h>
#include "libh.h"

#define SCRATCHSIZE  4096
#define TRUE 1
#define FALSE 0

extern FILE *symf;
extern short filecounter;

short scratchptr = 1;		/* points to next available scratch loc */
short blocksize = 1;		/* size of next block */
char *installadr;		/* to save symbol string */
short nscratchsyms = 0;		/* number of sumbols defined */
short externflag = FALSE;	/* whether "external" appears in definition */
short declstate = 0;		/* declaration syntax state machine */

decl(name,nwds,extflag)
	char *name;
{
/*	EXISTS	EXTERN	ACTION
 *	  0	  0	define it
 *	  0	  1	define it - don't complain it's not defined
 *	  1	  0	try to install it - produce "REDEFINED" error
 *	  1	  1	do nothing
 */
    struct slist *_slookup();
    register struct slist *sp;

    sp = _slookup(name);
    if ((sp == NULL) || !extflag) {
	scratchinstall(name,scratchptr,0);
	++nscratchsyms;
	scratchptr += nwds;
	if (scratchptr >= SCRATCHSIZE) {
	    fprintf(symf,"Out of scratch space ");
	    printf("Out of scratch space ");
	    printstate();
	}
    }
    else sp->filedec = filecounter;	/* defined, extflag=1: update tag */
}


_declare(adr)
    char *adr;
{
    if (declstate>0) {
	if (*installadr==NULL) goto skipit;
	scratchinstall(installadr,scratchptr,externflag);
	if (externflag==FALSE) {
	    scratchptr += blocksize;
	    ++nscratchsyms;
	}
	if (scratchptr >= SCRATCHSIZE) {
		printf("Out of scratch space\n");
		fprintf(symf,"Out of scratch space\n");
		printstate();
	}
    }
skipit:
    blocksize = 1;
    externflag = FALSE;
    installadr = adr;	/* save til end of declare */
    declstate = 1;
}


_blok(siz)
    short siz;
{
    if (declstate != 1) {
	printf("Syntax errror in declaration: 'blok'\n");
	fprintf(symf,"Syntax errror in declaration: 'blok'\n");
    }
    blocksize = siz;
    declstate = 2;
}


_external()
{
    if (declstate != 1 && declstate != 2) {
	printf("Syntax error in declaration: 'external'\n");
	fprintf(symf,"Syntax error in declaration: 'external'\n");
    }
    externflag = TRUE;
    declstate = 3;
}


sreloc(n)
	short n;
{
    if ((n >= SCRATCHSIZE) || (n<0)) {
	printf("Scratchaddr(%d) out of bounds\n",n);
	fprintf(symf,"Scratchaddr(%d) out of bounds\n",n);
    }
    else scratchptr = n;
}
