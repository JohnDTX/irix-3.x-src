/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/mbmalloc.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:38 $
 */

#include	"stand.h"
#include	"cpureg.h"
#include	"common.h"
#include	"sysmacros.h"
#include	"mbenv.h"
#include	"dprintf.h"

extern char *malloc();

/*
** mbmalloc
**   allocate multibus-visible memory. Multibus memory is visible through
**   segment 4.  It is also mapped to virtual address MBmallocVA
*/
char *
mbmalloc( nbytes )
register unsigned int	nbytes;
{
	char *ptr;

	ptr = malloc(nbytes);

	if ( ptr > 0 ) 
		bzero( ptr, nbytes );

	dprintf(("mbmalloc returning 0x%x\n",ptr));
	return (ptr);
}

/*
 * mbmap() -	map the virtual address into multibus address space (size bytes)
 *		return the physical multibus address of the buffer
 *		Set the external MBmappedVA to the base of the virtual page
 *		that we are passed.
 */
char *
mbmap(vaddr,size)
char *vaddr;
int size;
{
	register u_short *mbmapentry;

	long mbaddr;	/* physical multibus address	*/
	long addr;
	int page;	/* physical page	*/
	int npages, i;

	/* if in multibus address space you don't need to do anything	*/
	if ( (vaddr >= MBmemVA)  && ( (vaddr+size) < (MBmemVA+MBmallocSZ)) )
		return( vaddr);

	addr = ( (long)vaddr & ~(NBPG-1) );
	mbmapentry = (u_short *)( (int)_commdat->c_mbmapadr +
						(int)MBmappedstart);

	npages = size/NBPG + 2;		/* +2 for slop	*/
	for ( i = 0; i <npages; i++ ) {
		page = vtopage(addr);
		*mbmapentry = page;
		mbmapentry = (u_short *)((int)mbmapentry + NBPG);
		addr += NBPG;
	}
	MBmappedVA = (char *)( (long)vaddr & ~(NBPG-1) );
	return( MBmappedphys + ((long)vaddr & (NBPG-1)) );
}

vtopage(vaddr)
long vaddr;
{
	register u_short base;
	register u_short vpn;
	register struct pte *ppte;
	
	switch ( vaddr & SEG_MSK ) {
	    case SEG_TD:
		base = *TDBASE_REG;
		break;

	    case SEG_OS:
		base = (u_short)(*OS_BASE) << 8;
		break;

	    default:
		printf("bad segment in vtopage (0x%x)\n",vaddr);
		return(-1);
	}

	vpn = (vaddr & ~SEG_MSK) >> PGSHIFT;
	ppte = (struct pte *)PTMAP_BASE + (vpn | base);

	return( ppte->pg_page);
	
}

/* A very simple first fit memory allocator			*/

/* the structure is header,data					*/

#define NULL 0
#define ALLOCFLAG	0xabcd
#define FREEFLAG	0xdcba

struct minfo {
	struct minfo *m_next;	/* forward pointer */
	struct minfo *m_last;	/* back pointer	*/
	int m_size;		/* size		*/
	int m_flag;		/* flag for later use	*/
};

struct minfo mfirst;


char *
malloc(size)
{
	register struct minfo *mp, *nmp;

	dprintf(("malloc %d\n",size));
	if ( mfirst.m_flag != ALLOCFLAG ) {	/* first time code	*/
		mallocinit();
	}

	if ( size == 0 )	/* what the fuck	*/
		return((char *)0);

	for ( mp=mfirst.m_next; mp != NULL; mp=mp->m_next) {
		if ( size <= mp->m_size )
			break;
	}
	if ( mp == NULL ) {
		dprintf(("malloc failed "));
		return((char *)0);
	}

	/* now set up block */
	if ( mp->m_size - size <= sizeof (struct minfo) ) {
		/* not worth fragmenting */
		(mp->m_last)->m_next = mp->m_next;
		(mp->m_next)->m_last = mp->m_last;
	} else {
		nmp = (struct minfo *)((int)mp + sizeof (struct minfo) + size);
		(mp->m_last)->m_next = nmp;
		nmp->m_next = mp->m_next;
		nmp->m_last = mp->m_last;
		nmp->m_size = mp->m_size - size - sizeof (struct minfo);
	}
	mp->m_size = size;
	mp->m_flag = ALLOCFLAG;
	dprintf(("malloc returning 0x%x\n",((int)mp+sizeof (struct minfo))));
	return( (char *)((int)mp+sizeof (struct minfo)));
}

/* free the data buffer that was gotten by a malloc	*/
/* The buffers are kept in order by address to ease in combining buffers */ 
free(dp)
char *dp;	/* pointer to data	*/
{
	register struct minfo *fp;	/* freeing pointer */
	register struct minfo *lp;	/* one to left of freeing	*/

	fp = (struct minfo *)( (int)dp - sizeof (struct minfo));

	if ( fp->m_flag != ALLOCFLAG ) {
		return(-1);
	}

	/* find spot to replace */

	/* take care of no space case	*/
	if ( mfirst.m_next == NULL ) {
		mfirst.m_next = fp;
		fp->m_next = NULL;
		fp->m_last = &mfirst;
		return(0);
		
	}

	/* we want lp to point to left buffer */
	for ( lp = &mfirst; fp > lp->m_next; lp = lp->m_next )
		if ( lp->m_next == NULL )
			break;

	/* put in the chain and then try to combine with neighbors	*/
	fp->m_next = lp->m_next;
	lp->m_next = fp;
	if ( fp->m_next != NULL )
		(fp->m_next)->m_last = fp;

	/* try combining with buffer to right */
	if ((int)fp + fp->m_size + sizeof (struct minfo) == (int)fp->m_next ) {
		fp->m_size += ((fp->m_next)->m_size + sizeof (struct minfo));
		fp->m_next = (fp->m_next)->m_next;
	}
	/* try combining with buffer to left */
	if ((int)lp + lp->m_size + sizeof (struct minfo) == (int)fp ) {
		lp->m_size += ( fp->m_size +sizeof (struct minfo));
		lp->m_next = fp->m_next;
	}
	return(0);
}


mallocinit()
{
	register struct minfo *mp;
	
	mfirst.m_next = (struct minfo *)MBmallocVA;
	mfirst.m_last = NULL;
	mfirst.m_size = -1;
	mfirst.m_flag = ALLOCFLAG;
	mp = mfirst.m_next;
	mp->m_next = NULL;
	mp->m_last = &mfirst;
	mp->m_size = MBmallocSZ - sizeof (struct minfo);
}

#ifdef DEBUG
printm()
{
	register struct minfo *mp;

	if ( mfirst.m_flag != ALLOCFLAG )
		mallocinit(); 
	printf("here is the malloc chain:\n");
	mp = mfirst.m_next;
	while ( mp != NULL ) {
		printf("<daddr 0x%x, size %d>",
			(int)mp + sizeof (struct minfo), mp->m_size);
		mp = mp->m_next;
	}
	printf("\n");
}
#endif
