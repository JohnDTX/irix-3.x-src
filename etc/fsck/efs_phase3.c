# include "efs_fsck.h"

Phase3()
{
	extern int pass2();

	register DINODE *dp;
	ino_t savino;

	idprintf("** Phase 3 - Check Connectivity\n");
	for(inum = ROOTINO; inum <= lastino; inum++) {
		if(getstate() == DSTATE) {
			pfunc = findino;
			srchname = "..";
			savino = inum;
			do {
				orphan = inum;
				if((dp = ginode()) == NULL)
					break;
				filsize = dp->di_size;
				parentdir = 0;
				ckinode(dp,DATA);
				if((inum = parentdir) == 0)
					break;
			} while(getstate() == DSTATE);
			inum = orphan;
			if(linkup() == YES) {
				thisname = pathp = pathname;
				*pathp++ = '?';
				pfunc = pass2;
				descend();
			}
			inum = savino;
		}
	}
}
