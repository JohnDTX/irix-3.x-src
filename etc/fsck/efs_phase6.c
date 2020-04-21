# include "efs_fsck.h"

Phase6()
{
	idprintf("** Phase 6 - Salvage Free List\n");
	makefree();
	n_free = superblk.fs_tfree;
}

makefree()
{
	register int i;
	register BUFAREA *fp;
	register char *cp;

	superblk.fs_dirty = 1;
	superblk.fs_tfree = data_blocks - n_blks;
	sbdirty();

	cp = blkmap;
	for (i = 0; i < bitmap_blocks; i++) {
		if (blkmap == 0) {
			fp = tgetblk(bmapblk + i);
			cp = fp->b_un.b_buf;
		}
		bcom(cp, BSIZE);
		if (bwrite(&dfile,
				cp, BITMAPB + i, (MEMSIZE)BSIZE) == NO)
			return;
		cp += BSIZE;
	}
}

int
bcom(t, n)
	register char *t;
	register int n;
{

	while (--n >= 0) {
		*t = ~*t;
		t++;
	}
}
