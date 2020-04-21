#include "pmII.h"
#include "mem.h"
#include "common.h"
#include "IrisConf.h"
#include "Qglobals.h"

# define repeat		do
# define until(x)	while(!(x))

# undef  DEBUG do_debug
# include "dprintf.h"


extern unsigned long memory_summary();


struct region
{
    char *s, *e;
};

/*

	check memory and clear it if this is a power-up.

	returns true if memory needed to be cleared.

	NOTE this doesn't clear stack+bss; the startup routine is assumed
	to have done it.

*/
check_memory()
{
    register int verbose;

    /* if (!memory_trashed()) return 0; */

    verbose = VERBOSE(_commdat->config) != 0;

    memchk(verbose);
    memclr(verbose);

    /*
     * get a summarized copy of the new memory layout
     * and copy it into the stashed location
     */
    _commdat->membits = memory_summary();

    return 1;
}

struct region textr, stackr, commr;

extern char InProm;
extern char edata[], end[];

memclr(verbose)
    int verbose;
{
    register i;

    if( verbose ) printf("clearing memory:");

    bcopy(memfound,_commdat->memfound,sizeof _commdat->memfound);

    /*
     * get physical addresses of sacred areas.
     * assumptions:
     *		if !InProm, text < data and contiguous,
     *		            tsize + dsize <= 2*ROMSIZE,
     *		            text origin aligned on 2*ROMSIZE boundary;
     *		stack < bss and contiguous;
     *		currently using only the highest page of stack+bss.
     */
    if( !InProm )
	getregion(edata, &textr, 2*ROMSIZE);
    getregion(end, &stackr, ptoa(1));
    stackr.e = stackr.s + ptoa(1);
    commr.s = (char *)0; commr.e = _commend;

    /* zero the memory */
    for( i = 0; i < MAX_MEM_CHUNKS; i++ )
	if( memfound[i] )
	{
	    struct region m, seg;

	    if( verbose ) putchar('X');

	    seg.s = (char *)(i*MEM_CHUNK_SIZE);
	    seg.e = seg.s+MEM_CHUNK_SIZE;

	    /* map in this chunk */
	    mapmem(VIRT_FIRST_MAGIC_PAGE
		    ,atop((long)seg.s),MEM_CHUNK_PAGES);

	    /* clear this chunk, watching out for sacred areas */
	    for( m.s = seg.s; m.s < seg.e; m.s = m.e )
	    {
		m.e = seg.e;

		if( !InProm )
		    skipsacred(&m, &textr);
		skipsacred(&m, &stackr);
		skipsacred(&m, &commr);

dprintf((" clr $%x-$%x",m.s, m.e));
		if( m.s < m.e )
		    bzero(ptoa(VIRT_FIRST_MAGIC_PAGE)+(m.s-seg.s), m.e-m.s);
	    }
	}
	else
	{
	    if( verbose ) putchar('.');
	}


    /* clear to the end of the common area. */
    _commdat->memfound[MAX_MEM_CHUNKS] = 0;
    _commdat->pad = 0;
    _commdat->addr488 = 0;
    /*_commdat->dcconfig = 0;*/

    if( _commdat->reboot != MAGIC_REBOOT_VALUE )
    {
	_commdat->reboot = 0;	    
	_commdat->boottype = 0;
	bzero(_commdat->bootstr,_commend-_commdat->bootstr);
    }

    if( verbose ) newline();
}

skipsacred(tp, sp)
    register struct region *tp, *sp;
{
    /*
     * if no intersection, return.
     * otherwise, if target starts before sacred area,
     * change its end; if target starts within the sacred area,
     * change its beginning and end.
     */
    if( tp->e <= sp->s || sp->e <= tp->s )
	return;
    if( tp->s < sp->s )
	tp->e = sp->s;
    else
	tp->s = tp->e = sp->e;
}

getregion(e, rp, grain)
    register char *e;
    register struct region *rp;
    int grain;
{
    unsigned short ppage, pprot;
    register char *s;

dprintf((" getregion($%x,...$%x)",e,grain));
    s = e-1;				/* start is just before end */
    s -= (long)s % grain;		/* round down start */
    getmap(atop((long)s),&ppage,&pprot);/* get its physical page # */
    rp->s = (char *)ptoa(ppage);	/* convert to byte addr */
    rp->e = rp->s + (e - s);		/* remap end */
dprintf(("-->$%x-$%x\n",rp->s, rp->e));
}

short
find_multibus_window()
{	
	/* The multibus window is at zero for the master
	   processor. Slaves address must be determined
	   by a method to be taken up later. */
	if (BOOTENV(_commdat->config) == ENV_SLAVE) {
		_commdat->flags &= ~MASTER;
		_commdat->mbw = 0x200;
		_commdat->mblog = 0x210;
		_commdat->mbphys = 0;
		_commdat->nmbpages = (_commdat->membits > 1)?0xf0:0x70;
	}
	else {
		_commdat->flags |= MASTER;
		_commdat->mbw = 0;
		_commdat->mbphys = atop(MBMEMOFFSET);
		_commdat->mblog = atop(MBMEMOFFSET);
		_commdat->nmbpages = N_MBMEM_PAGES;
	}
	return(_commdat->mblog); 
}



/*

	beginning at physical 0x200, there is a small global communication
	area that is used to control the state of the world between resets.
	Part of this consists of TWO complete pieces of information 
	indicating where physical memory is located. These are compared, 
	and the results of this comparison used to indicate whether or
	not we have just been powered up.  memory_summary's job is to
	assemble the abbreviated copy from the other one.
	memory_trashed will compare the result of memory_summary to
	the current abbreviated copy. If the two copies match, a
	warm start is assumed and memory_trashed returns FALSE.  

*/

unsigned long
memory_summary()
{

	register int i;
	register unsigned long membits,bitmask;

	membits = 0; bitmask = 1;

	for( i = 0; i < MAX_MEM_CHUNKS; i++ )
	{
	    if( _commdat->memfound[i] )
		membits |= bitmask;
	    bitmask <<= 1;
	}

	return membits;
}

int
memory_trashed()
{
	return memory_summary() != _commdat->membits;
}
