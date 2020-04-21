/*
 * $Source: /d2/3.7/src/sys/pmII/RCS/autoconf.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:28 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/dk.h"
#include "../h/dklabel.h"
#include "../h/setjmp.h"
#include "../h/autoconf.h"
#include "../h/printf.h"
#include "../pmII/reg.h"
#include "../pmII/trap.h"
#include "../pmII/frame.h"
#include "../multibus/mbvar.h"

struct	autoconf ac = { 0 };

/* interrupt routines */
extern	int level1(), level2(), level5();
int	default_intr();

/* interrupt vectors for multibus i/o */
typedef	(*fptr_t)();
fptr_t	ivectors[8] = {
	default_intr, level1, level2, default_intr,
	default_intr, level5, default_intr, default_intr,
};

extern	short beprint;
extern	int *nofault;

/*
 * autoconfmsg:
 *	- print a message about the configuration
 */
autoconfmsg(state)
	int state;
{
	switch (state) {
	  case CONF_DEAD:
		printf("not functioning correctly\n");
		break;
	  case CONF_FAULTED:
		printf("not installed\n");
		break;
	}
}

/*
 * configure:
 *	- attempt to configure the systems i/o devices, using both the
 *	  controller tables and the device tables
 */
configure()
{
	register struct mb_ctlr *mm;
	register struct mb_driver *md;
	register struct mb_device *mi;
	register short state;
	register int *saved_jb;
	jmp_buf jb;

	ac.a_probing = 1;
	initvectors();			/* init so kernel printf's will work */

    /* probe the controllers */
	for (mm = mbcinit; mm->mm_driver; mm++) {
		md = mm->mm_driver;
		if (md->md_probe)
			probe(mm, md);
	}

    /* now probe any simple devices */
	for (mi = mbdinit; mi->mi_driver; mi++) {
		md = mi->mi_driver;
		if ((mi->mi_slave != 0xFF) || (md->md_probe == 0))
			continue;

		/* invalidate mi_dk so that vmstat will leave it alone */
		mi->mi_dk = -1;

		printf("%s%d ", md->md_dname, mi->mi_unit);
		saved_jb = nofault;
		beprint = 0;
		if (setjmp(jb) == 0) {
			nofault = jb;
			state = (*md->md_probe)(mi->mi_addr, mi->mi_ctlr);
		} else
			state = CONF_FAULTED;
		beprint = 1;
		nofault = saved_jb;
		switch (state) {
		  case CONF_DEAD:
		  case CONF_FAULTED:
		  default:
			autoconfmsg(state);
			break;
		  case CONF_ALIVE:
			printf("at mbio 0x%04x ipl %d\n", mi->mi_addr,
				   mi->mi_priority);
			mi->mi_alive = 1;
			break;
		}
	}

#ifndef	KOPT_NOGL
	/*
	** probe for the lightpen
	*/
	lpenprobe();
#endif	KOPT_NOGL

	ac.a_probing = 0;
	initvectors();			/* force, in case user messed up */

    /* figure out root and swap devices */
	pickroot();
}

/*
 * initvectors:
 *	- setup interrupt vectors for graphics hardware
 */
initvectors()
{
#ifndef	KOPT_NOGL
	extern int ge_intr(), fbc_intr();
#endif
	extern int mouse_intr();

#ifndef	KOPT_NOGL
	/* patch in graphics devices, if they are there */
	if (havegrconsole) {
		ivectors[3] = fbc_intr;
		ivectors[4] = ge_intr;
	} else
#endif
		ivectors[4] = mouse_intr;	/* dummy mouse handler */
}

/*
 * probe:
 *	- probe a controller, and its drives
 */
probe(mm, md)
	register struct mb_ctlr *mm;
	register struct mb_driver *md;
{
	register short state;
	register int *saved_jb;
	extern int *nofault;
	jmp_buf jb;

	printf("%s%d ", md->md_cname, mm->mm_ctlr);
	saved_jb = nofault;
	beprint = 0;
	if (setjmp(jb) == 0) {
		nofault = jb;
		state = (*md->md_probe)(mm->mm_addr, mm->mm_ctlr);
	} else
		state = CONF_FAULTED;
	beprint = 1;
	nofault = saved_jb;
	switch (state) {
	  case CONF_DEAD:
	  case CONF_FAULTED:
	  default:
		autoconfmsg(state);
		break;
	  case CONF_ALIVE:
		printf("at mbio 0x%04x ipl %d\n",
			   mm->mm_addr, mm->mm_priority);
		mm->mm_alive = 1;
		md->md_cinfo[mm->mm_ctlr] = mm;
		if (md->md_attach)
			attach(mm, md);
		break;
	}
}

/*
 * attach:
 *	- search device init table for this controllers drives
 *	  and attach them
 */
attach(mm, md)
	register struct mb_ctlr *mm;
	register struct mb_driver *md;
{
	register struct mb_device *mi;
	register char *cp;
	register short state, isdk;

	for (mi = mbdinit; mi->mi_driver; mi++) {
		if ((mi->mi_driver != md) ||
		    (mm->mm_ctlr != mi->mi_ctlr))
			continue;
		cp = md->md_sname ? (*md->md_sname)(mi) : 
			md->md_dname;
		printf("%s%d ", cp, mi->mi_unit);
		state = (*md->md_attach)(mi);

		/*
		 * Assume that device is not a disk, or does not exist
		 * by invalidating mi_dk so that vmstat will work
		 * correctly.
		 */
		isdk = mi->mi_dk;
		mi->mi_dk = -1;

		switch (state) {
		  case CONF_DEAD:
		  case CONF_FAULTED:
		  default:
			autoconfmsg(state);
			break;
		  case CONF_ALIVE:
			printf("slave %d\n", mi->mi_slave);
			mi->mi_alive = 1;
			md->md_dinfo[mi->mi_unit] = mi;
			if (isdk && (ac.a_dkn < DK_NDRIVE))
				mi->mi_dk = ac.a_dkn++;
			break;
		}
	}
}

/*
 * default_intr:
 *	- default interrupt routine
 */
default_intr(frame)
	struct frame frame;
{
	printf("stray interrupt level %d\n",
		      ((frame.vecoffset & 0xFFF) >> 2) - T_SPURINT);
	panic("default_intr");
}

/*
 * setroot:
 *	- look at label information and compare it against prototype
 *	  rootdev & swapdev
 *	- if we have a match, then record this drives information for
 *	  later root & swap dev choices
 */
setroot(md, mi, drive, mask, dk, strat)
	struct mb_driver *md;
	struct mb_device *mi;
	register short drive;
	register struct disk_label *dk;
	int (*strat)();
{
	register dev_t maj;
	register long swapsize;
	register struct autoconf *ap = &ac;
	register u_char rootfs;

	/*
	 * If device is not in the bdevsw, then we can't use it.
	 */
	if ((maj = getmajor(strat)) == NODEV)
		return;

	/*
	 * see if this drive matches what we booted from
	 */
	rootfs = (dk->d_rootnotboot) ? dk->d_rootfs : dk->d_bootfs;
	if ((rootfs != 0xFF) && (drive == (rootdev & mask))) {
		ap->a_root.r_major = maj;
		ap->a_root.r_drive = drive;
		ap->a_root.r_fs = rootfs;
		ap->a_root.r_md = md;
		ap->a_root.r_mi = mi;
	}

	/*
	 * see if this drive contains a swap region to use
	 */
	if ((dk->d_swapfs != 0xFF) && (drive == (swapdev & mask))) {
		/*
		 * If the size of this swap partition is larger than
		 * the best size to date, then remember it.
		 */
		swapsize = dk->d_map[dk->d_swapfs].d_size;
		if (swapsize > ap->a_swap.s_size) {
			ap->a_swap.s_major = maj;
			ap->a_swap.s_drive = drive;
			ap->a_swap.s_fs = dk->d_swapfs;
			ap->a_swap.s_md = md;
			ap->a_swap.s_mi = mi;
			ap->a_swap.s_size = swapsize;
		}
	}
}

/*
 * pickroot:
 *	- once the dust has settled after all the probing has been done,
 *	  figure out where the root and swap devices are
 */
pickroot()
{
	register char *name;
	register struct mb_driver *md;
	register struct mb_device *mi;
	register struct autoconf *ap = &ac;

	/*
	 * If we are told to force the root partition, do so now.
	 */
	if (rootfs != 0xFF)
		ap->a_root.r_fs = rootfs;

	/*
	 * Warn user if root was not found by autoconfiguration
	 */
	if (ap->a_root.r_md == NULL) {
		printf("WARNING: no root drive found during configuration\n");
		printf("root on dev %04x\n", rootdev);
	} else {
		rootdev = makedev(ap->a_root.r_major,
				  ap->a_root.r_drive | ap->a_root.r_fs);
		md = ap->a_root.r_md;
		mi = ap->a_root.r_mi;
		name = md->md_dname;
		if (md->md_sname)
			name = (*md->md_sname)(mi);
		printf("root on %s%d%c\n", name, mi->mi_unit,
			     ap->a_root.r_fs + 'a');
	}

	/*
	 * If nswap was patched, it is the swap length to use, starting at
	 * swplo.  Otherwise convert ac.a_swap.s_size, which is the best
	 * swap size found so far, into a length and put it in nswap.
	 */
	if (nswap != 0) {
		if (ap->a_swap.s_size < swplo + nswap)
	    printf("WARNING: patched nswap is LARGER than drive swap size\n");
	} else
		nswap = ap->a_swap.s_size - swplo;

	/*
	 * Warn user if swap was not found by autoconfiguration
	 */
	if (ap->a_swap.s_md == NULL) {
		printf("WARNING: no swap drive found during configuration\n");
		printf("swap on dev %04x, swplo=%d nswap=%d\n",
			     swapdev, swplo, nswap);
	} else {
		swapdev = makedev(ap->a_swap.s_major,
				  ap->a_swap.s_drive | ap->a_swap.s_fs);
		md = ap->a_swap.s_md;
		mi = ap->a_swap.s_mi;
		name = md->md_dname;
		if (md->md_sname)
			name = (*md->md_sname)(mi);
		printf("swap on %s%d%c, swplo=%d nswap=%d\n",
			     name, mi->mi_unit, ap->a_swap.s_fs + 'a',
			     swplo, nswap);
	}
}
