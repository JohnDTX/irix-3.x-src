/*
 * $Source: /d2/3.7/src/sys/vm/RCS/vm_subr.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:36:05 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../vm/vm.h"
#include "../h/proc.h"
#include "../h/cmap.h"
#include "../h/inode.h"
#include "../h/text.h"
#include "machine/pte.h"

/*
 * Make uarea of process p addressible at kernel virtual
 * address uarea through sysmap locations starting at map.
 */
uaccess(p, map, uarea)
	register struct proc *p;
	struct pte *map;
	register struct user *uarea;
{
	register int i;
	register struct pte *mp = map;

	for (i = 0; i < UPAGES; i++) {
		*(int *)mp = 0;
		mp->pg_pfnum = p->p_addr[i].pg_pfnum;
		mp++;
	}
	vmaccess(map, (caddr_t)uarea, UPAGES);
}

/*
 * Validate the kernel map for size ptes which
 * start at ppte in the sysmap, and which map
 * kernel virtual addresses starting with vaddr.
 */
vmaccess(ppte0, vaddr0, size0)
	struct pte *ppte0;
	caddr_t vaddr0;
	int size0;
{
	register struct pte *ppte = ppte0;
	register int size = size0;

	while (size != 0) {
		initpte(ppte, ppte->pg_pfnum, PG_V | PG_KW);
		ppte++;
		--size;
	}
	ptaccess(ppte0, (struct pte *)vaddr0, size0);
}

/* 
 * Convert a pte pointer to
 * a virtual page number.
 */
ptetov(p, pte)
	register struct proc *p;
	register struct pte *pte;
{

	if (isatpte(p, pte))
		return (tptov(p, ptetotp(p, pte)));
	else if (isadpte(p, pte))
		return (dptov(p, ptetodp(p, pte)));
	else
		return (sptov(p, ptetosp(p, pte)));
}
