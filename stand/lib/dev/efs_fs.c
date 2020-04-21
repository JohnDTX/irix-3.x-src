/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/efs_fs.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:27 $
 */
#include "stand.h"
#include "machine/param.h"
#include "fs.h"
#include "dprintf.h"

extern struct	inode *efs_iread();
extern int	efs_bmap(), nullop();

struct	iops efs_iops = {
	efs_iread,
	nullop,
	nullop,
	(struct inode *(*)())nullop,
	nullop,
	nullop,
	nullop,
	efs_bmap,
};

/* XXX all of this shit should be put into a per mount/ open data struct */
struct efs *ef;				/* for super block	*/
char *efs_ibuf;				/* for disk inode	*/
struct incore_efs_dinode *efs_incore;	/* incore inode		*/
short maxextents;			/* max incore extents	*/


efs_mount(io)
struct iob *io;
{
	register struct buf *bp;
	char *mbmalloc();

	dprintf(("efs_mount of dev %d\n",io->i_ip->i_dev));
	if ( ef == 0 ) {
		ef = (struct efs *)mbmalloc(DBLOCK);
	}
	bp = io->i_bp;
	bp->b_iobase = (caddr_t)ef;
	bp->b_bcount = DBLOCK;
	bp->b_iobn = SUPERBOFF >> DBSHIFT;

	if ( _devread(io) < 0 ) {
		return(-1);
	}

	if( ef->fs_magic != EFS_MAGIC ) {
		return(-1);
	}

	/* set up all of the dynamic variables that are not kept on disk
	 * superblock
	 */
	ef->fs_ipcg = ef->fs_cgisize * EFS_INOPBB;
	return(0);
}

/*
 * read in an inode from the extent file system
 */
struct	inode *
efs_iread(io)
register struct iob *io;
{
	register struct inode *ip;
	register struct buf *bp;
	register struct efs_dinode *dp;
	register struct incore_efs_dinode *dip;
	struct extent *ep, *ep2;
	short numext;			/* number of indirect extents to go */
	short extinblock;		/* num of indirect extents in block */
	register ino_t ino;
	char *mbmalloc();

	ip = io->i_ip;
	bp = io->i_bp;

	if ( efs_ibuf == 0 ) {
		if ( (efs_ibuf = mbmalloc(BBSIZE)) == 0 )
			return(0);
	}
	ino = ip->i_number;
	bp->b_iobn = ITOD(ef, ino);
	bp->b_iobase = efs_ibuf;
	bp->b_bcount = BBSIZE;

	
	dprintf(("efs_iread(%d) blk %d\n",ino,bp->b_iobn));

	if ( _devread(io) < 0 )
		return(0);

	dp = (struct efs_dinode *)efs_ibuf + ITOO(ef, ino);

	/* fill in inode from what we read of the disk	*/
	ip->i_mode = dp->di_mode;
	ip->i_size = dp->di_size;
	io->i_offset = 0;
	dprintf(("mode 0x%x, size %d\n",ip->i_mode, ip->i_size));

	/* now unpack the extents into the incore inode*/

	/* get space for incore inode */
	if ( efs_incore == 0 || dp->di_numextents > maxextents ) {
		if ( efs_incore != 0 ) {
			free(efs_incore);
		}

		efs_incore = (struct incore_efs_dinode *)mbmalloc(
			sizeof (struct incore_efs_dinode)
				+ dp->di_numextents*sizeof(struct extent) );
		if ( efs_incore == 0 ) {	/* malloc failed */
			maxextents = 0;
			return(0);
		}
		ip->i_data = (handle *)efs_incore;
		maxextents = dp->di_numextents;
	}
		
	ip->i_data = (handle *)efs_incore;

	dip = (struct incore_efs_dinode *)ip->i_data;
	dip->ii_numextents = dp->di_numextents;
	if ((dp->di_numextents < 0) /* ||
				(dp->di_numextents > EFS_MAXEXTENTS)*/ ) {
		dprintf(("bad number of extents [%d]\n",dp->di_numextents));
		io->i_error = EIO;
		return(0);
	}
	/* if you have indirect extents, all off the extents are in the
	 * indirect extents.  So read those in and set up the incore inode.
	 * The indirect extent start in the first extent.
	 */
	if ( dp->di_numextents > EFS_DIRECTEXTENTS ) {
		register int numindirs;

		dprintf(("Indirect extents!\n"));
		numext = dp->di_numextents;
		ep2 = dip->ii_extents;
		for (ep = &dp->di_u.di_extents[0], numindirs = ep->ex_offset;
		     numindirs > 0; ep++, --numindirs) {
			bp->b_iobn = ep->ex_bn;
			if ( (bp->b_iobase = mbmalloc(BBTOB(ep->ex_length)))
			    == 0 )
				return(0);
			bp->b_bcount = BBTOB(ep->ex_length);
			extinblock = BBTOB(ep->ex_length) / sizeof(extent);
			if ( _devread(io) < 0 )
				return(0);
			bcopy( (caddr_t)bp->b_iobase, (caddr_t)ep2,
				min(numext,extinblock) * sizeof(extent) );
			numext -= extinblock;
			ep2 += extinblock;
			free(bp->b_iobase);
		}
	} else {
		dip->ii_indir.ex_length = 0;
		bcopy( (caddr_t)dp->di_u.di_extents, (caddr_t)dip->ii_extents,
				dp->di_numextents * sizeof(extent) );
	}
	return(ip);

}

/*
 * Translate the offset into a physical block number.  Set the size to the
 * remaining size of the extent. ( ie the remaining size of the extent )
 * Just do a simple linear search through the extents to find which one
 * contains the offset.
 */
efs_bmap(io, szp)
register struct iob *io;
unsigned *szp;		/* pointer to size	*/
{
	register struct inode *ip;
	register struct incore_efs_dinode *dip;
	register struct extent *ep;
	register daddr_t lbn;
	register unsigned pos;	/* the offset in the file as we search */
	register int i;

	ip = io->i_ip;

	lbn = BTOBBT(io->i_offset);
	pos = 0;		/* get to the starting line	*/
	dip = (struct incore_efs_dinode *)ip->i_data;
	
	for(i = 0; i < dip->ii_numextents; i++) {
		ep = &dip->ii_extents[i];
		if ( lbn < (daddr_t)ep->ex_offset ) { /* hit a hole */
			*szp = BBTOB(ep->ex_offset - lbn);
			return(0);
		}

		if ( lbn < pos + ep->ex_length ) { /* this is it!! */
			*szp = BBTOB( (ep->ex_offset + ep->ex_length) - lbn );
			return( ep->ex_bn + ( lbn - ep->ex_offset) );
		}
		/* go on to next extent */
		pos += ep->ex_length;
		ep++;
	}
	/* we are not supposed to get here!!! */
	printf("efs_bmap: Fatal error\n");
	return(-1);
}
