/* reset.c
 *
 *	findge(verbose)
 *	smartconfigure(table)
 *	justconfigure(table)
 */

#define GFBETA
#include "gfdev.h"
#include "m68000.h"

#define buzz(n)		{short _count = n; while (--_count > 0);}
#define outlit(n)	GEdata = (n); buzz(100)
#define FBCfeedback	0x25
#define FBCsend(d)	FBCdata = (d); \
			GFdisabvert(GERESET3|ENABTRAPINT_BIT_,CYCINDEBUG); \
			GFdisabvert(GERESET3|ENABTRAPINT_BIT_,RUNDEBUG);

short GEfound,GEmask;
extern short printwd;

poutshort(dat)
	short dat;
{
outlit(dat);
if (printwd) printf(" %x ",dat);
}


findge(verb)	/* returns (short) mask of GE chips present , no. present */
	int verb;
{		/* in GEmask, GEfound 					*/
		/* NOTE *** must be modified for ALPHA use.		*/

	short i,j,temp,trapmask;
	short headGA = 0;

	hardinit();
	if (verb) printf("findge ");
#ifdef GF2
	FBCsend(FBCfeedback);
#endif
	FBCflags = RUNMODE;
	intlevel(7);
	GEmask = 0;		/* assemble mask of chips present */
	GEfound = 0;		/* count no. chips present */
	GEflags = GERESET1;
	if ((FBCflags & FITRAP_BIT_)
	    && !(GEflags & (LOWATER_BIT + HIWATER_BIT))) {
		GEmask = 2;
		headGA = 1;
		if (verb) printf(" head GA ");
	}

	trapmask = GETRAP_BIT(1);
				/* which trap bit expected if GE not there */
	for (i=1; i<13; i++)
	    {
		reset_GE();
				/* reset pipe -- no fifo ints allowed */
		poutshort(0x3a00);
				/* config infifo, no lowater allowed */
		if (headGA) poutshort(0xff01);
		for (j=GEfound; j; --j)
		    {				/* Configure GEs already */
			poutshort(0xff38);	/* found as wires. */
		    }
		poutshort(0xff09);			/* configure GE to probe  */
		poutshort(2);			/* send an illegal command */
		if (GEflags&trapmask) {++GEmask; ++GEfound;}
					/* if trap, GE is there - set a bit */
		if (verb) putchar('.');
		GEmask <<= 1;
		trapmask <<= 1;
	    }
	    if (headGA) ++GEfound;
	    GEflags = GERESET1;
	    if ((FBCflags & FOTRAP_BIT_) &&
	        !(GEflags & (LOWATER_BIT + HIWATER_BIT))) {
			GEmask |= 1;
			++GEfound;
			if (verb) printf(" tail GA ");
	    }
	    reset_GE();
}


smartconfigure(table)
	short *table;
{
	/* configure GEs */
	reset_GE();
	reset_GE();

	poutshort( 0x3a02 );	/* FIFO chip with high water mark at 63 */
				/* and low water mark at 2?		*/
	if (GEmask & 0x2000) poutshort( (short) ((GEfound-1)<<8) +1 );
	justconfigure(table);
	poutshort( 0x3a02 );	/* FIFO chip at end of pipe    */
	if (GEmask & 1) poutshort(0xff00 | table[13]);
}


justconfigure(table)
	short *table;
{
    register found;
    register short temp,j,mask;

	mask = GEmask << 3;
	found = GEfound;
	if (GEmask & 0x2000) --found;	/* if head GA present, found too big*/
	for (temp=1; temp<13; temp++ )	/* for each chip position... */
		if ((mask <<= 1) < 0)
		   {
			j = (--found<<8) | *(table+temp) ;
			poutshort(j);
		   }
}

reset_GE()
{
	short head;

	GEflags = GERESET1;
	buzz(100);
	head = FBCflags & FITRAP_BIT_;
	GEflags = GERESET3;
	return(head);
}
