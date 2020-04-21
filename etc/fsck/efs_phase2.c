# include "efs_fsck.h"


char DotDot[DIRSIZ]	= "..";
char Dot[DIRSIZ]	= ".";
# define DotCheck(np)	(*(long *)np == *(long *)Dot \
			|| *(long *)np == *(long *)DotDot)

Phase2()
{
	extern int pass2();

	register DINODE *dp;

	idprintf("** Phase 2 - Check Pathnames\n");
	inum = ROOTINO;
	thisname = pathp = pathname;
	pfunc = pass2;
	switch(getstate()) {
		case USTATE:
			iderrexit("ROOT INODE UNALLOCATED. TERMINATING.\n");
		case FSTATE:
			idprintf("ROOT INODE NOT DIRECTORY");
			if(reply("FIX") == NO || (dp = ginode()) == NULL)
				errexit("");
			dp->di_mode &= ~IFMT;
			dp->di_mode |= IFDIR;
			inodirty();
			setstate(DSTATE);
		case DSTATE:
			descend();
			break;
		case CLEAR:
			idprintf("DUPS/BAD IN ROOT INODE\n");
			if(reply("CONTINUE") == NO)
				errexit("");
			setstate(DSTATE);
			descend();
	}

	pss2done++;
}

pass2(dirp)
register DIRECT *dirp;
{
	register char *p;
	register n;
	register DINODE *dp;

	if((inum = dirp->d_ino) == 0)
		return(KEEPON);
	thisname = pathp;
	if((&pathname[MAXPATH] - pathp) < DIRSIZ) {
		if((&pathname[MAXPATH] - pathp) < strlen(dirp->d_name)) {
			idprintf("DIR pathname too deep\n");
			idprintf("Increase MAXPATH and recompile.\n");
			idprintf("DIR pathname is <%s>\n", pathname);
			ckfini();
			exit(4);
		}
	}
	for(p = dirp->d_name; p < &dirp->d_name[DIRSIZ]; )
		if((*pathp++ = *p++) == 0) {
			--pathp;
			break;
		}
	*pathp = 0;
	n = NO;
	if(inum > max_inodes || inum < ROOTINO)
		n = direrr("I OUT OF RANGE");
	else {
	again:
		switch(getstate()) {
			case USTATE:
				n = direrr("UNALLOCATED");
				break;
			case CLEAR:
				if((n = direrr("DUP/BAD")) == YES)
					break;
				if((dp = ginode()) == NULL)
					break;
				setstate(DIR ? DSTATE : FSTATE);
				goto again;
			case FSTATE:
				declncnt();
				break;
			case DSTATE:
				declncnt();
				descend();
		}
	}
	pathp = thisname;
	if(n == NO)
		return(KEEPON);
	dirp->d_ino = 0;
	return(KEEPON|ALTERD);
}

long dir_size;
/*
 * chkempt() --
 * check whether a directory is "empty" .
 * i.e., has nothing but . and .. .
 * by calling extent-list checker or indirect-extent
 * checker as appropriate.
 */
int
chkempt(dp)
	register DINODE *dp;
{
	register int ret;

	dir_size = dp->di_size;
	ret = ckinode(dp, DEMPT);
	return ret == KEEPON ? YES : NO;
}

/*
 * chkeblk() --
 * check a directory block for being "empty" .
 */
int
chkeblk(bn)
	register daddr_t bn;
{
	register struct direct *dirp;

	if (outrange(bn)) {
		printf("chkempt: blk %d out of range\n",bn);
		return SKIP;
	}
	if (getblk(&fileblk, bn) == NULL) {
		printf("chkempt: Can't find blk %d\n", bn);
		return SKIP;
	}

	for (dirp = dirblk;
	 dirp < dirblk+NDIRECT && (dir_size -= sizeof (DIRECT)) >= 0;
	 dirp++) {
		if (DotCheck(dirp->d_name))
			continue;
		if(dirp->d_ino)
			return STOP;
	}

	return KEEPON;
}
