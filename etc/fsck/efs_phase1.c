# include "efs_fsck.h"

/*
 * Phase1() --
 *
 * cause each inode to be checked, causing each
 * block used by the inode to be checked.
 * build the free block bitmap as a side-effect.
 * report extent botches, badblocks, and dups.
 */
Phase1()
{
	extern int pass1(), pass1b();

	register DINODE *dp;
	register int n;
	BUFAREA *bp1, *bp2;

	idprintf("** Phase 1 - Check Blocks and Sizes\n");
	init_bmap();

	pfunc = pass1;
	for (inum = FIRSTINO; inum <= max_inodes; inum++) {
		if ((dp = ginode()) == NULL)
			continue;
		if (ALLOC) {
			lastino = inum;
			if (!ftypeok(dp)) {
				idprintf("UNKNOWN FILE TYPE I=%u",inum);
				if(dp->di_size)
					printf(" (NOT EMPTY)");
				if(reply("CLEAR") == YES) {
					zapino(dp);
					inodirty();
				}
				continue;
			}
			n_files++;
			if (setlncnt(dp->di_nlink) <= 0) {
				if(badlnp < &badlncnt[MAXLNCNT])
					*badlnp++ = inum;
				else {
					idprintf("LINK COUNT TABLE OVERFLOW");
					if(reply("CONTINUE") == NO)
						errexit("");
				}
			}
			setstate(DIR ? DSTATE : FSTATE);
			badblk = dupblk = 0;
			filsize = 0;
			ckinode(dp, ADDR);
			if ((n = getstate()) == DSTATE || n == FSTATE)
				sizechk(dp);

		}
		else
		if (dp->di_mode != 0) {
			idprintf("PARTIALLY ALLOCATED INODE I=%u",inum);
			if(dp->di_size)
				printf(" (NOT EMPTY)");
			if(reply("CLEAR") == YES) {
				zapino(dp);
				inodirty();
			}
		}
	}

	if (enddup != &duplist[0]) {
		idprintf("** Phase 1b - Rescan For More DUPS\n");
		pfunc = pass1b;
		for (inum = FIRSTINO; inum <= lastino; inum++) {
			if(getstate() != USTATE && (dp = ginode()) != NULL)
				if(ckinode(dp,ADDR) & STOP)
					break;
		}
	}

	if (rawflg) {
		if (inoblk.b_dirty)
			bwrite(&dfile,membase,startib,niblk*BSIZE);
		inoblk.b_dirty = 0;
		if (poolhead) {
			clear(membase,niblk*BSIZE);
			for (bp1 = poolhead;bp1->b_next;bp1 = bp1->b_next);
			bp2 = &((BUFAREA *)membase)[(niblk*BSIZE)/sizeof(BUFAREA)];
			while (--bp2 >= (BUFAREA *)membase) {
				initbarea(bp2);
				bp2->b_next = bp1->b_next;
				bp1->b_next = bp2;
			}
		}
		rawflg = 0;
	}
}

/*
 * ckinode() --
 *
 * check an inode.
 * cause each block used to be checked.
 */
ckinode(dp, flg)
	register DINODE *dp;
	register flg;
{
	register ret;

	int (*func)();
	int stopper;
	daddr_t firstoff;

	struct extent extents[EXTSPERDINODE];

	if (SPECIAL)
		return KEEPON;

	switch (flg) {

	case ADDR:
		func = pfunc;
		stopper = STOP;
		break;

	case DATA:
		func = dirscan;
		stopper = STOP;
		break;

	case BBLK:
		func = chkblk;
		stopper = 0;
		break;

	case DEMPT:
		func = chkeblk;
		stopper = STOP|SKIP;
		break;
	}

	firstoff = 0;
	bcopy(dp->di_x, extents, sizeof extents);
	if ((unsigned short)dp->di_nx <= EXTSPERDINODE)
		ret = chk_extlist(extents, dp->di_nx,
				func, stopper, &firstoff);
	else
		ret = chk_iext(extents, dp->di_nx,
				func, stopper, &firstoff);

	if (ret & stopper)
		return ret;
	return KEEPON;
    }

/*
 * chk_iext() --
 *
 * check an indirect extent.
 * cause each block of direct extents to be checked.
 */
int
chk_iext(xp, nx, func, stopper, _firstoff)
	register struct extent *xp;
	int nx;
	int (*func)();
	int stopper;
	daddr_t *(_firstoff);
{
	register int ixb, xl;
	register daddr_t xb;
	register int ret;
	BUFAREA ib;

	if ((unsigned)nx >= RIDICULOUSEXTS) {
		idprintf("RIDICULOUS NUMBER OF EXTENTS (%d)", nx);
		return SKIP;
	}
	xl = xp->ex_length;
	if (xp->ex_magic != EFS_EXTENT_MAGIC) {
		exterr("BAD MAGIC IN EXTENT", xp);
		return SKIP;
	}
	for (ixb = 0 , xb = xp->ex_bn; ixb < xl; ixb++ , xb++) {
		if (func == pfunc)
			if (!((ret = (*func)(xb)) & KEEPON))
				return ret;

		if (outrange(xb))
			return SKIP;

		initbarea(&ib);
		if (getblk(&ib, xb) == NULL)
			return SKIP;

		ret = chk_extlist(ib.b_un.b_ext,
				nx < EXTSPERBB ? nx : EXTSPERBB,
				func, stopper, _firstoff);
		if (ret & stopper)
			return ret;

		nx -= EXTSPERBB;
	}

	if (xl < (nx * sizeof (struct extent) + BBSIZE-1) >> BBSHIFT) {
		exterr("INDIRECT EXTENT SIZE ERROR", xp);
		return SKIP;
	}
	return KEEPON;
}

/*
 * chk_extlist() --
 * cause checking of each (direct) extent in a list,
 * causing each block to be checked.
 */
int
chk_extlist(xp, nx, func, stopper, _firstoff)
	register struct extent *xp;
	int (*func)();
	int stopper;
	daddr_t *_firstoff;
{
	register int ix;
	register int ret;
	
	for (ix = 0; ix < nx; ix++ , xp++) {
		ret = chk_ext(xp, func, stopper, _firstoff);
		if (ret & stopper)
			return(ret);
	}
	return KEEPON;
}

/*
 * chk_ext() --
 * cause checking of each block in an extent.
 */
int
chk_ext(xp, func, stopper, _firstoff)
	register struct extent *xp;
	int (*func)();
	int stopper;
	daddr_t *_firstoff;
{
	register int ixb, xl;
	register daddr_t xb;
	register int ret;
	register int firsttime;

	xl = xp->ex_length;

	firsttime = *_firstoff == 0;

	if (*_firstoff > xp->ex_offset) {
		exterr("EXTENT OUT OF ORDER", xp);
		return SKIP;
	}
	if (xp->ex_magic != EFS_EXTENT_MAGIC) {
		exterr("BAD MAGIC IN EXTENT", xp);
		return SKIP;
	}
	if (xp->ex_length == 0) {
		exterr("ZERO LENGTH EXTENT", xp);
		return SKIP;
	}

	*_firstoff = xp->ex_offset + xp->ex_length;

	for (ixb = 0 , xb = xp->ex_bn; ixb < xl; ixb++ , xb++) {
		ret = (*func)(xb, firsttime);
		if (ret & stopper)
			return ret;
		firsttime = 0;
	}
	
	return KEEPON;
}


chkblk(blk, dotflag)
register daddr_t blk;
{
	register DIRECT *dirp;
	register char *ptr;
	int zerobyte, baddir = 0, dotcnt = 0;

	if (outrange(blk))
		return(SKIP);
	if (getblk(&fileblk, blk) == NULL)
		return(SKIP);
	for (dirp = dirblk; dirp <&dirblk[NDIRECT]; dirp++) {
		ptr = dirp->d_name;
		zerobyte = 0;
		while (ptr <&dirp->d_name[DIRSIZ]) {
			if (zerobyte && *ptr) {
				baddir++;
				break;
			}
			if (dotflag) {
				if (ptr == &dirp->d_name[0] && *ptr == '.' &&
					*(ptr + 1) == '\0') {
					dotcnt++;
					if (inum != dirp->d_ino) {
						idprintf("NO VALID '.' in DIR I = %u\n",
							inum);
						baddir++;
					}
					break;
				}
				if (ptr == &dirp->d_name[0] && *ptr == '.' &&
					*(ptr + 1) == '.' && *(ptr + 2) == '\0') {
					dotcnt++;
					if (!dirp->d_ino) {
						idprintf("NO VALID '..' in DIR I = %u\n",
							inum);
						baddir++;
					}
					break;
				}
			}
			if (*ptr == '/') {
				baddir++;
				break;
			}
			if (*ptr == NULL) {
				if (dirp->d_ino && ptr == &dirp->d_name[0]) {
					baddir++;
					break;
				}
				else
					zerobyte++;
			}
			ptr++;
		}
	}
	if (dotflag && dotcnt < 2) {
		idprintf("MISSING '.' or '..' in DIR I = %u\n",inum);
		idprintf("BLK %ld ",blk);
		pinode();
		newline();
		idprintf("DIR=%s\n\n",pathname);
		return(YES);
	}
	if (baddir) {
		idprintf("BAD DIR ENTRY I = %u\n",inum);
		idprintf("BLK %ld ",blk);
		pinode();
		newline();
		idprintf("DIR=%s\n\n",pathname);
		return(YES);
	}
	return(KEEPON);
}

pass1(blk)
register daddr_t blk;
{
	register i;
	register daddr_t *dlp;
	char baddie;

	if(chkblki)
		for(i = 0; i < chkblki; i++)
			if(blk == chkblks[i])
				idprintf("BLK=%u in I=%u\n", blk, inum);
	baddie = 0;
	if(outrange(blk)) {
		baddie++;
	}
	else
	if(getbmap(blk)) {
	    /*
	     * determine if it's really a dup -
	     * or a bad (we know already it's not
	     * out of range).  ie, if it's within
	     * a valid data area or not.
	     */
	    if ((blk-superblk.fs_firstcg) % superblk.fs_cgfsize
			>= superblk.fs_cgisize
	     || (blk-superblk.fs_firstcg) / superblk.fs_cgfsize
			>= superblk.fs_ncg) {

		blkerr("DUP",blk);
		if(++dupblk >= MAXDUP) {
			idprintf("EXCESSIVE DUP BLKS I=%u",inum);
			if(reply("CONTINUE") == NO)
				errexit("");
			return(STOP);
		}
		if(enddup >= &duplist[DUPTBLSIZE]) {
			idprintf("DUP TABLE OVERFLOW.");
			if(reply("CONTINUE") == NO)
				errexit("");
			return(STOP);
		}
		for(dlp = duplist; dlp < muldup; dlp++) {
			if(*dlp == blk) {
				*enddup++ = blk;
				break;
			}
		}
		if(dlp >= muldup) {
			*enddup++ = *muldup;
			*muldup++ = blk;
		}
	    }
	    else {
		baddie++;
	    }
	}
	else {
		n_blks++;
		setbmap(blk); 
	}

	if (baddie) {
		blkerr("BAD",blk);
		if(++badblk >= MAXBAD) {
			idprintf("EXCESSIVE BAD BLKS I=%u",inum);
			if(reply("CONTINUE") == NO)
				errexit("");
			return(STOP);
		}
		return(SKIP);
	}

	filsize++;
	return(KEEPON);
}

pass1b(blk)
register daddr_t blk;
{
	register daddr_t *dlp;

	if(outrange(blk))
		return(SKIP);
	for(dlp = duplist; dlp < muldup; dlp++) {
		if(*dlp == blk) {
			blkerr("DUP",blk);
			*dlp = *--muldup;
			*muldup = blk;
			return(muldup == duplist ? STOP : KEEPON);
		}
	}
	return(KEEPON);
}

pass4(blk)
register daddr_t blk;
{
	register daddr_t *dlp;

	if(outrange(blk))
		return(SKIP);
	if(getbmap(blk)) {
		for(dlp = duplist; dlp < enddup; dlp++)
			if(*dlp == blk) {
				*dlp = *--enddup;
				return(KEEPON);
			}
		clrbmap(blk);
		n_blks--;
	}
	return(KEEPON);
}

dirscan(blk)
register daddr_t blk;
{
	register DIRECT *dirp;
	register n;
	DIRECT direntry;

	if(outrange(blk)) {
		filsize -= BSIZE;
		return(SKIP);
	}
	for(dirp = dirblk; dirp < &dirblk[NDIRECT] && filsize > 0;
			dirp++, filsize -= sizeof(DIRECT)) {

		if(getblk(&fileblk,blk) == NULL) {
			filsize -= (&dirblk[NDIRECT]-dirp)*sizeof(DIRECT);
			return(SKIP);
		}

		direntry = *dirp;

		if((n = (*pfunc)(&direntry)) & ALTERD) {
			if(getblk(&fileblk,blk) != NULL) {
				*dirp = direntry;
				fbdirty();
			}
			else
				n &= ~ALTERD;
		}
		if(n & STOP)
			return(n);
	}
	return(filsize > 0 ? KEEPON : STOP);
}

/*
 * busy out the inodes, bitmap, and superblock.
 */
init_bmap()
{
	register int cgno;
	register int i;
	register daddr_t bn;

	for (bn = superblk.fs_firstcg; --bn >= 0;)
		setbmap(bn);
	for (cgno = 0; cgno < superblk.fs_ncg; cgno++) {
		bn = CGIMIN(&superblk, cgno);
		for (i = superblk.fs_cgisize; --i >= 0;)
			setbmap(bn++);
	}
}

exterr(s, xp)
	register struct extent *xp;
	char *s;
{
	idprintf("[%ld+%d: %ld] %s I=%u\n",
			(daddr_t)xp->ex_offset, xp->ex_length,
			(daddr_t)xp->ex_bn, s, inum);
	setstate(CLEAR);	/* mark for possible clearing */
}
