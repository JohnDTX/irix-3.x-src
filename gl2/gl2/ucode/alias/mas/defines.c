/* defines.c
 */

#define SCRATCHSIZE  4096
#define TRUE 1
#define FALSE 0
#define NULL  0

short scratchptr = 1;		/* points to next available scratch loc */
short blocksize = 1;		/* size of next block */
char *installadr;		/* to save symbol string */
short nscratchsyms = 0;		/* number of sumbols defined */
short externflag = FALSE;	/* whether "external" appears in definition */
short declstate = 0;		/* declaration syntax state machine */

decl(name,nwds,externflag)
	char *name;
{
	struct slist *_slookup();

	if (_slookup(name) == NULL)
		if (!externflag) {
			printf("scratch symbol %s redefined ",name);
			printstate();
			return(0);
		}
	scratchinstall(name,scratchptr,0);
	++nscratchsyms;
	scratchptr += nwds;
	if (scratchptr >= SCRATCHSIZE) {
		printf("Out of scratch space ");
		printstate();
	}
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
    if (declstate != 1)
	printf("Syntax errror in declaration: 'blok'\n");
    blocksize = siz;
    declstate = 2;
}


_external()
{
    if (declstate != 1 && declstate != 2)
	printf("Syntax error in declaration: 'external'\n");
    externflag = TRUE;
    declstate = 3;
}


sreloc(n)
	short n;
{
    if ((n >= SCRATCHSIZE) || (n<0))
	printf("Scratchaddr(%d) out of bounds\n",n);
    else scratchptr = n;
}
