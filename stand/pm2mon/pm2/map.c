#include "pmII.h"
#include "common.h"

# define PROMSTATIC

/*
 *
 *	map.c - routines to set the page and protection maps on the pm2.1
 *
 *	two facilities are provided:
 *
 * 	    setprot sets the protection of N logical(virtual) pages beginning
 *		at a specified page number L, to a given protection value.
 *
 *	    setmap sets the map of N logical pages beginning
 *		at a specified page number L, to contiguous physical pages
 *		beginning at a specified page number P
 *
 */

setprot(startpg,npages,prt)
unsigned short startpg,npages,prt;
{
	/* set the protection of logical pages startpg -> (startpg+npages-1)
	   to protection prt */

	register int i;
	register short *protptr = &PROTMAP[startpg];

	for (i=0;i<npages;i++) 
		*protptr++ = prt;
}

setmap(start_log,npages,start_phys)
unsigned short start_phys,npages,start_log;
{
	/* map logical pages start_log to (start_log+npages-1) to 
	   physical pages contiguously starting at start_phys */
	register int i;
	register short *pageptr = &PAGEMAP[start_log];

	for (i=0;i<npages;i++)
		*pageptr++ = start_phys++;
}

getmap(pageno,_phys,_prot)
    register unsigned short pageno;
    short *(_phys),*(_prot);
{
    *_phys = PAGEMAP[pageno];
    *_prot = PROTMAP[pageno];
}

setmbmap(start_mblog,npages,start_phys)
unsigned short start_phys,npages,start_mblog;
{
	register int i;
	register short *cp,proto;
	register int virtpage,physpage;
	short oldphys,oldprot;

	/* set the multibus page map to map npages of memory
	   starting at the indicated logical page visible to
	   the multibus to the given physical page */

	/* first, set the off-board pagemap */
	/* 1. map it in */
	virtpage = mapmagic();
	setprot(virtpage,1,PROT_MBMEM|PROT_RWX___);

	STATUS_REG &= ~(EN0|EN1);	/* and enable multibus memory access*/

	/* 2. set it up */
	physpage = start_mblog+atop((long)MBMAPOFFSET);
	cp = (short *)ptoa(virtpage);
	proto = start_phys;
	for( i = 0; i < npages; i++ )
	{
	    setmap(virtpage,1,physpage++);
	    *cp = proto++;
	}

	unmapmagic();
}

mapin(virtpage,physpage,proto,npages)
    register unsigned short virtpage;
    unsigned short physpage;
    unsigned short proto;
    short npages;
{
    while( --npages >= 0 )
    {
	PAGEMAP[virtpage] = physpage++;
	PROTMAP[virtpage] = proto;
	virtpage++;
    }
}

mapmem(virtpage,physpage,npages)
    unsigned short virtpage,physpage;
    short npages;
{
    mapin(virtpage,physpage,PROT_LOCMEM|PROT_RWX___,npages);
}
