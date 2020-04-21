# include "efs_fsck.h"

Phase4()
{
	extern int pass4();

	idprintf("** Phase 4 - Check Reference Counts\n");
	pfunc = pass4;
	for(inum = ROOTINO; inum <= lastino; inum++) {
		switch(getstate()) {
		case FSTATE:
			phase4fstate();
			break;
		case DSTATE:
			clri("UNREF",YES);
			break;
		case CLEAR:
			clri("BAD/DUP",YES);
			break;
		}
	}

	if(max_inodes - n_files != superblk.fs_tinode) {
		idprintf("FREE INODE COUNT WRONG IN SUPERBLK");
		if (qflag) {
			superblk.fs_tinode = max_inodes - n_files;
			sbdirty();
			newline();
			idprintf("FIXED\n");
		}
		else if(reply("FIX") == YES) {
			superblk.fs_tinode = max_inodes - n_files;
			sbdirty();
		}
	}
	flush(&dfile,&fileblk);
}

phase4fstate()
{
	register DINODE *dp;
	register ino_t *blp;
	register int n;

	if(n = getlncnt())
		adjust((short)n);
	else {
		for(blp = badlncnt; blp < badlnp; blp++)
			if(*blp == inum) {
				if((dp = ginode()) && dp->di_size) {
					if((n = linkup()) == NO)
					   clri("UNREF",NO);
					if (n == REM)
					   clri("UNREF",REM);
				}
				else {
					clri("UNREF",YES);
				}
				break;
			}
	}
}
