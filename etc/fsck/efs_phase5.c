# include "efs_fsck.h"

# define DEBUG fsck_5debug
# include "dprintf.h"
short fsck_5debug = 0;

Phase5()
{
	register BUFAREA *bp1, *bp2;
	register int n;
	daddr_t blk;

	idprintf("** Phase 5 - Check Free List ");
	if (sflag || (csflag && rplyflag == 0)) {
		printf("(Ignored)\n");
		fixfree = 1;
	}
	else {
		newline();
		if (freemap) {
			copy(blkmap, freemap, (MEMSIZE)bmapsz);
		}
		else {
			for(blk = 0; blk < fmapblk; blk++) {
				bp1 = tgetblk(blk+bmapblk);
				bp2 = tgetblk(blk+fmapblk);
				copy(bp1->b_un.b_buf, bp2->b_un.b_buf, BSIZE);
				dirty(bp2);
			}
		}
		badblk = dupblk = 0;
		freechk();
		if (badblk)
			idprintf("%d BAD BLKS IN FREE LIST\n", badblk);
		if (dupblk)
			idprintf("%d DUP BLKS IN FREE LIST\n", dupblk);

		if (fixfree == 0) {
			if (n_blks+n_free != data_blocks) {
				idprintf("%ld BLK(S) MISSING\n",
					data_blocks - n_blks - n_free);
				fixfree = 1;
			}
			else
			if (n_free != superblk.fs_tfree) {
				idprintf("FREE BLK COUNT WRONG IN SUPERBLK");
				if (qflag) {
					superblk.fs_tfree = n_free;
					sbdirty();
					newline();
					idprintf("FIXED\n");
				}
				else
				if (reply("FIX") == YES) {
					superblk.fs_tfree = n_free;
					sbdirty();
				}
			}
		}
		if (fixfree) {
			idprintf("BAD FREE LIST");
			if (qflag && !sflag) {
				fixfree = 1;
				newline();
				idprintf("SALVAGED\n");
			}
			else
			if (reply("SALVAGE") == NO)
				fixfree = 0;
		}
	}
}

pass5(blk)
	register daddr_t blk;
{
dprintf((" pass5(%ld)",blk));

	if (outrange(blk)) {
		fixfree = 1;
dprintf((" freebad %ld",blk));
		if (++badblk >= MAXBAD) {
			idprintf("EXCESSIVE BAD BLKS IN FREE LIST.");
			if (reply("CONTINUE") == NO)
				errexit("");
			return(STOP);
		}
		return(SKIP);
	}
	if (getfmap(blk)) {
		fixfree = 1;
		if (++dupblk >= DUPTBLSIZE) {
			idprintf("EXCESSIVE DUP BLKS IN FREE LIST.");
			if (reply("CONTINUE") == NO)
				errexit("");
			return(STOP);
		}
	}
	else {
		n_free++;
		setfmap(blk); 
	}
	return(KEEPON);
}

freechk()
{
	register daddr_t bn;
	register int blockoff, byteoff;
	register char *bp;
	register short b;
	int bufblock;
	BUFAREA bmbuf;

	if (superblk.fs_tfree
	 >= superblk.fs_size - superblk.fs_cgisize*superblk.fs_ncg) {
		idprintf("BAD FREEBLK COUNT\n");
		fixfree = 1;
		return;
	}

	bufblock = BITMAPB;
	bn = 0;
	for (;;) {
		initbarea(&bmbuf);
		if (getblk(&bmbuf, bufblock) == NULL)
			return;
		bufblock++;

		bp = bmbuf.b_un.b_buf;
		for (blockoff = BSIZE; --blockoff >= 0;) {
			b = *bp++;
			for (byteoff = BITSPERBYTE; --byteoff >= 0;) {
				if (bn >= fmax)
					return;
				if (b&01) {
					if (pass5(bn) != KEEPON)
						return;
				}
				b >>= 1;
				bn++;
			}
		}
	}
}
