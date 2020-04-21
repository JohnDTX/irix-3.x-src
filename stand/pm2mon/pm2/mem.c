#include "pmII.h"
#include "mem.h"


/*	
	memchk MUST be called in order to find memory and set 
	up the location of the multibus window page (mbw_page)

	NOTE this scribbles on the first few words of each
	half-megabyte of mem.  hopefully our stack, data, text,
	and bss won't be!
*/


# define NPATS	4
short chkpat[NPATS] = { 0x0000, 0xFFFF, 0xAAAA, 0x5555 };

int
memchk(verbose)
    int verbose;
{
    register short magicpage;
    register short *VAD;

    register i;
    register short cx;
    register int nhalfmegs;
    short oldcont[NPATS];

    magicpage = mapmagic();
    VAD = (short *)ptoa(magicpage);

    CONTEXT_REG = 0;			/* Context 0 */

    STATUS_REG = DIAG(5)|ENABLE_BOOT;	/* stat5, ~BOOT */

    if(verbose) printf("scanning memory:");

    nhalfmegs = 0;

    for( i = 0; i < MAX_MEM_CHUNKS; i++ )
    {
	/* map in first page of this mem chunk */
	mapmem(magicpage,i*MEM_CHUNK_PAGES,1);

	/* write test pattern */
	for( cx = 0; cx < NPATS; cx++ )
	{
	    oldcont[cx] = VAD[cx];	/* save old contents */
	    VAD[cx] = chkpat[cx];
	}

	/* see whether it acted like memory */
	for( cx = 0; cx < NPATS; cx++ )
	{
	    if( VAD[cx] != chkpat[cx] )
		break;
	    VAD[cx] = oldcont[cx];	/* restore old contents */
	}

	if( cx < NPATS )
	{
	    /* it didn't */
	    memfound[i] = 0;
	    if( verbose ) putchar('.');
	}
	else
	{
	    memfound[i]++;
	    nhalfmegs++;
	    if(verbose) putchar('X');
	}
    }

    unmapmagic();

#ifdef MULTI
    STATUS_REG = DIAG(6)|ENABLE_BOOT|ENABLE_MBUS/*|ENABLE_PARITY*/;	/**/
    STATUS_REG &= ~ENABLE_MBUS;
#else
    STATUS_REG = DIAG(7)|ENABLE_BOOT/*|ENABLE_PARITY*/; /* enable parity */
    STATUS_REG &= ~ENABLE_MBUS;
#endif

    if(verbose) newline();

    return nhalfmegs;
}


# ifdef NOTDEF
teerr()
{
	register i;
	register short *a;

	for(a = PAGEMAP, i = 0; a < PROTMAP; )	/* Map virt to phys */
		*a++ = i++;
	for( ; a < PROTEND; )		/* sup rwx, not multi */
		*a++ = PROT_LOCMEM|PROT_RWX___;
	CONTEXT_REG = 0;			/* Context 0 */

#ifdef MULTI
	STATUS_REG = DIAG(6)|ENABLE_BOOT|ENABLE_MBUS;	
#else
	STATUS_REG = DIAG(7)|ENABLE_BOOT	; 
#endif
}
# endif NOTDEF
