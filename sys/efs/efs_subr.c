/*
 * Miscellaneous subroutines
 *
 * $Source: /d2/3.7/src/sys/efs/RCS/efs_subr.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:28:02 $
 */
#include "efs.h"
#if NEFS > 0

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../efs/efs_inode.h"
#include "../efs/efs_fs.h"
#include "../efs/efs_sb.h"
#include "../efs/efs_ino.h"

/*
 * efs_builddsum:
 *	- scan the data bitmap, counting free blocks within the cg
 *	- construct data allocation summary information in cg_dsum
 *	- cg_firstdfree is a rotor we use to allow partial scanning
 *	  of the cylinder group
 *	- we optionally will continue scanning the bitmap, even when
 *	  the summary info is full, for the purpose of computing the
 *	  cg_dfree value
 * XXX	get the data block rotor working
 */
efs_builddsum(fs, cg, minimumsize, dontstop)
	register struct efs *fs;
	register struct cg *cg;
	register int minimumsize;
	int dontstop;
{
	register char *dmap;
	register int firstbit, bitlen;
	register daddr_t curbn;
	register off_t length;
	register int dfree;
	register struct cgdmap *cgd;
	int lookfor;
#ifdef	ASSERT
	int zfree;
#endif

	/*
	 * Don't bother scanning the data bitmap if the fs is corrupted
	 * or the cylinder group is corrupted, or if the filesystem
	 * is read only.
	 */
	if (fs->fs_corrupted || fs->fs_readonly)
		return;
	cg->cg_firstdfree = cg->cg_firstdbn;		/* XXX */
	firstbit = cg->cg_firstdfree;
	bitlen = (cg->cg_firstbn + fs->fs_cgfsize) - firstbit;
#ifdef	ASSERT
	if (bitlen != fs->fs_cgfsize - fs->fs_cgisize) {
		printf("firstbit=%d bitlen=%d fsize=%d isize=%d cg=%d\n",
				    firstbit, bitlen, fs->fs_cgfsize,
				    fs->fs_cgisize,
				    cg - &fs->fs_cgs[0]);
		debug("efs_builddsum0");
	}
#endif

	/*
	 * Now scan bitmap.  Build up data summary info.  Only accept
	 * regions of at least minimumsize in length.
	 */
	if (minimumsize <= 0)
		minimumsize = 1;
	dfree = 0;
#ifdef	ASSERT
	zfree = 0;
#endif
	length = 0;
	curbn = cg->cg_firstdfree;
	cg->cg_nextd = 0;
	cg->cg_lastd = 0;
	dmap = fs->fs_dmap;
	cgd = &cg->cg_dsum[0];
	lookfor = 1;
	while (bitlen) {
		if (lookfor) {
			length = bftstset(dmap, firstbit, bitlen);
			dfree += length;
			if (length >= minimumsize) {
				if (cg->cg_lastd < CG_DSUMS) {
					cgd->d_bn = curbn;
					cgd->d_length = length;
					cgd++;
					cg->cg_lastd++;
					if (!dontstop &&
					      (cg->cg_lastd >= CG_DSUMS))
						break;
				}
			} else
			if (length == 0)
				goto findzeros;
			lookfor = 0;
		} else {
findzeros:
			length = bftstclr(dmap, firstbit, bitlen);
			lookfor = 1;
#ifdef	ASSERT
			zfree += length;
#endif
		}
		firstbit += length;
		curbn += length;
		bitlen -= length;
	}
#ifdef	ASSERT
	if (dontstop && (zfree + dfree != fs->fs_cgfsize - fs->fs_cgisize)) {
		printf("zfree=%d dfree=%d\n", zfree, dfree);
		debug("efs_builddsum1");
	}
#endif
	if (dontstop)
		cg->cg_dfree = dfree;
}

/*
 * efs_cgdcheck:
 *	- destroy cg summary info after an extend
 */
efs_cgdcheck(cg, bn, len)
	register struct cg *cg;
	register daddr_t bn;
	register int len;
{
	register struct cgdmap *cgd, *cgdlast;
	register daddr_t cgdbn;

	/*
	 * Check for summary information overlap with region
	 * just allocated.  If any overlap at all, invalidate the
	 * summary info.
	 */
	cgd = &cg->cg_dsum[cg->cg_nextd];
	cgdlast = &cg->cg_dsum[(short) cg->cg_lastd];
	while (cgd < cgdlast) {
		cgdbn = cgd->d_bn;
		if ( ((cgdbn <= bn) && (cgdbn + cgd->d_length > bn)) ||
		     ((bn <= cgdbn) && (bn + len > cgdbn)) ) {
			cg->cg_nextd = CG_DSUMS;
			return;
		}
		cgd++;
	}
}

/*
 * efs_badextent:
 *	- make sure that the extent doesn't overlap anything except
 *	  data blocks
 */
/* ARGSUSED */
efs_badextent(fs, ex, ip)
	register struct efs *fs;
	register extent *ex;
	register struct inode *ip;
{
	register daddr_t bn;
	register int len;
	long foundlen;

#ifdef	ASSERT
	if ((ex->ex_bn == 0) || (ex->ex_length == 0)) {
		printf("ip=%x fs=%x ex=%x\n", ip, fs->fs_dev, ex);
		panic("efs oops");
	}
#endif

	/* insure that bn,len are reasonable */
	if ((ex->ex_bn < fs->fs_firstcg) || (ex->ex_length == 0) ||
	    (ex->ex_bn > fs->fs_size) || (ex->ex_length > EFS_MAXEXTENTLEN)) {
		printf("bn/len bad: ");
		goto bad;
	}

	/* make sure extent doesn't overlap inodes */
	bn = ex->ex_bn - fs->fs_firstcg;
	len = ex->ex_length - 1;
	if ((bn % fs->fs_cgfsize) < fs->fs_cgisize) {
		printf("inode overlap: ");
		goto bad;
	}

	/* make sure that extent doesn't cross a cg boundary */
	if ((bn / fs->fs_cgfsize) != ((bn + len) / fs->fs_cgfsize)) {
		printf("crossing cg: ");
		goto bad;
	}

	/*
	 * Check bn against bitmap.  Insure that the extent doesn't overlap
	 * some free blocks.
	 */
	foundlen = bftstclr(fs->fs_dmap, (int) ex->ex_bn, (int) ex->ex_length);
	if (foundlen != ex->ex_length) {
		printf("bitmap messed up, foundlen=%d bitlen=%d: ",
			       foundlen, ex->ex_length);
		goto bad;
	}
	return (0);

bad:
	printf("bad extent, inum=%d, ip=%x bn=%d len=%d\n",
		    ip ? ip->i_number : 0, ip, ex->ex_bn, ex->ex_length);
#ifdef	ASSERT
	debug("efs_badextent");
#endif
	/*
	 * Mark the filesystem as corrupted so that the user will be
	 * forced to unmount and fsck it.
	 */
	fs->fs_corrupted = 1;
	efs_error(fs, 0);
	return (1);
}

/*
 * efs_error:
 *	- print out an error describing the problem with the fs
 *	- this should maybe be an fs_* operator
 */
efs_error(fs, type)
	register struct efs *fs;
	int type;
{
	register dev_t dev;

	dev = fs->fs_dev;
	if (fs->fs_corrupted)
		prdev("filesystem corruption", dev);
	else {
		switch (type) {
		  case 0:
			if (fs->fs_tfree)
				prdev("out of contiguous space", dev);
			else
				prdev("no space", dev);
			break;
		  case 1:
			if (fs->fs_tinode)
				prdev("inode corruption", dev);
			else
				prdev("out of inodes", dev);
			break;
		}
	}
	delay(hz);
}

#endif
