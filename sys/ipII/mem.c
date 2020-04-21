/*
 * Driver for /dev/mem, /dev/kmem, /dev/null, and /dev/swap
 *	- minor device 0 is physical memory	(/dev/mem)
 *	- minor device 1 is kernel memory	(/dev/kmem)
 *	- minor device 2 is EOF/RATHOLE		(/dev/null)
 *	- minor device 3 is swap		(/dev/swap)
 *
 * $Source:
 * $Revision:
 * $Date:
 */
#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/buf.h"
#include "../h/systm.h"
#include "../h/conf.h"
#include "../h/cmap.h"
#include "../vm/vm.h"
#include "../h/setjmp.h"
#include "../ipII/cpureg.h"
#include "../ipII/cx.h"
#include "../ipII/pte.h"

/* buffer header for user swap i/o */
struct	buf rswbuf;

/*
 * mmstrategy:
 *	- strategy routine called by raw read/write routines to indirect
 *	  to the real swap driver
 */
mmstrategy(bp)
	register struct buf *bp;
{
	(*bdevsw[bmajor(swapdev)].d_strategy)(bp);
}

/*
 * mmread:
 *	- read mem, kmem, null, or swap dev
 */
mmread(dev)
	dev_t dev;
{
	switch (minor(dev)) {
	  case 0:				/* /dev/mem */
		pmem(B_READ);
		break;
	  case 1:				/* /dev/kmem */
		kmem(B_READ);
		break;
	  case 2:				/* /dev/null */
		break;
	  case 3:				/* /dev/swap */
		physio(mmstrategy, &rswbuf, swapdev, B_READ, minphys);
		break;
	}
}

/*
 * mmwrite:
 *	- write mem, kmem, null, or swap dev
 */
mmwrite(dev)
	dev_t dev;
{
	switch (minor(dev)) {
	  case 0:				/* /dev/mem */
		pmem(B_WRITE);
		break;
	  case 1:				/* /dev/kmem */
		kmem(B_WRITE);
		break;
	  case 2:				/* /dev/null */
		u.u_count = 0;
		break;
	  case 3:				/* /dev/swap */
		physio(mmstrategy, &rswbuf, swapdev, B_WRITE, minphys);
		break;
	}
}

/*
 * kmem:
 *	- process logical memory i/o
 */
kmem(flag)
	int flag;
{
	register long prot;
	struct pte apte;

	u.u_segflg = 0;
	while (!u.u_error && u.u_count) {
		if ( ( u.u_offset & SEG_MSK ) != SEG_OS ) {
			u.u_error = ENXIO;
			return;
		}
		/*
		 * Read hardware pagemap for given virtual address.  Check
		 * protection of page to make sure that the kernel can
		 * access it.  Also make sure that the page is writable if
		 * this is a write.
		 */
		getpte(u.u_offset, KCX, &apte);
		prot = *(long *)&apte & PG_PROT;

		/*
		 * have to be careful here as the protection bits
		 * can be somewhat mutually exclusive
		 */
		if (((prot != PTE_RWACC) && (prot != PG_KW) && (prot != PG_KR))
				|| ((flag == B_WRITE) && (prot != PG_KW))) {
			u.u_error = ENXIO;
			return;
		}
		/*
		 * Copy data, bounding the amount of data we will move to
		 * reside within the given page that u.u_offset resides in.
		 */
		iomove((caddr_t)u.u_offset,
		       (int) MIN((NBPG - (u.u_offset & PGOFSET)), u.u_count),
		       flag);
	}
}

/*
 * pmem:
 *	- process physical memory requests
 *	- this code KNOWS that copyout uses SCRPG0 and SCRPG1
 */
pmem(flag)
	int flag;
{
	register u_int pageoffset;
	register int count;
	struct pte pg2pte, apte;

	/*
	 * Copy the data, at most one page at a time.
	 */
	getpte(SCRPG2_VBASE, KCX, &pg2pte);		/* save */
	u.u_segflg = 0;
	while (!u.u_error && u.u_count) {
		pageoffset = u.u_offset & PGOFSET;
		count = MIN(NBPG - pageoffset, u.u_count);
		*(long *)&apte = btop(u.u_offset) | PG_KW | PG_V;
		setpte(SCRPG2_VBASE, KCX, &apte);
		iomove((caddr_t)SCRPG2_VBASE + pageoffset, count, flag);
	}
	setpte(SCRPG2_VBASE, KCX, &pg2pte);		/* restore */
}

/*
 * Ioctl's to get at kernel data structures.
 * This interface is **not** supported.
 */
char XXXetheraddr[6];
char XXXetheraddr_initialized;
/*ARGSUSED*/
mmioctl(dev, cmd, arg, flag)
	dev_t dev;
	int cmd;
	caddr_t arg;
	int flag;
{
	register int i;
	char hostident[64];		/* this size must never change */

	switch (cmd) {
	  case 1:
		if (!XXXetheraddr_initialized) {
			u.u_error = ENODEV;
			break;
		}
		/*
		 * Manufacture some part of the hostident - constant, then
		 * fill in the rest with the ethernet address sprinkled
		 * around for fun.
		 */
		for (i = 0; i < 64; i++)
			hostident[i] = i*9 + '#';
		hostident[0] = XXXetheraddr[0];
		hostident[2] = XXXetheraddr[1];
		hostident[4] = XXXetheraddr[2];
		hostident[6] = XXXetheraddr[3];
		hostident[8] = XXXetheraddr[4];
		hostident[10] = XXXetheraddr[5];
		if (copyout(hostident, arg, 64))
			u.u_error = EFAULT;
		break;

	  default:
		u.u_error = EINVAL;
		break;
	}
}
