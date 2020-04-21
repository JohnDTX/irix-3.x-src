/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/bfs_fs.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:17 $
 */
/*
 * routines for bell filesystem:
 *
 * bell_mount - determine type of filesystem
 *
 * bell_iread - read in inode ( ie get an inode )
 *
 * bell_bmap - map logical file block to filesystem block
 *
 */
#include "stand.h"
#include "fs.h"
#include "machine/param.h"
#include "dprintf.h"

extern struct	inode *bell_iread();
extern int	bell_bmap(), nullop();

struct	iops bell_iops = {
	bell_iread,
	nullop,
	nullop,
	(struct inode *(*)())nullop,
	nullop,
	nullop,
	nullop,
	bell_bmap,
};

/* XXX all of this shit should be put into a per mount data structure */
# define FsbToDb(n)	((n)<<FsDblockShift)

#define INODEBLOCK	2
#define lblkno(n)	((n)>>FsBlockShift)
#define lblkoff(n)	((n)&FsBlockOffMask)
#define itod(n)	(INODEBLOCK+(unsigned)((n)-FIRSTINO)/NinodesPerBlock)
#define itoo(n)	((unsigned)((n)-FIRSTINO)%NinodesPerBlock)

short NaddrsPerBlock;
short NinodesPerBlock;
short FsBlockSize;
short FsDblockShift;
short FsBlockShift;
short FsBlockOffMask;


/* XXX malloc this too	*/
struct incore_dinode bf_incore;
char *Inbuf;
struct filsys *f;

int
bell_mount(io)
struct iob *io;
{
	register struct buf *bp;
	char *mbmalloc();

	dprintf(("bellmount of dev %d\n",io->i_ip->i_dev));
	if ( f == 0 ) {
		f = (struct filsys *)mbmalloc(DBLOCK);
	}
	bp = io->i_bp;
	bp->b_iobase = (caddr_t)f;
	bp->b_bcount = DBLOCK;
	bp->b_iobn = SUPERBOFF >> DBSHIFT;

	if ( _devread(io) < 0 ) {
		return(-1);
	}

	if( f->s_magic != FsMAGIC ) {
		return(-1);
	}

	/* XXX - all of the F* and N* stuff should be put in a per
	 * device data structure
	 */
	if( f->s_type == Fs2b ) {
		dprintf(("1K bell fs\n"));
		FsDblockShift = 1;
	} else {
		dprintf((".5K bell fs\n"));
		FsDblockShift = 0;
	}

	FsBlockSize = DBLOCK<<FsDblockShift;
	FsBlockShift = FsDblockShift+DBSHIFT;
	FsBlockOffMask = ~(~0<<FsBlockShift);

	NaddrsPerBlock = FsBlockSize / sizeof (daddr_t);
	NinodesPerBlock = FsBlockSize / sizeof (struct dinode);

	return(0);
}

/*
 * read in an inode from a bell filesystem
 */
struct	inode *
bell_iread(io)
register struct iob *io;
{
	register struct inode *ip;
	register struct dinode *dip;
	register u_char *cp, *tp;
	register int i;
	register ino_t ino;
	struct buf *bp;
	char *mbmalloc();

	ip = io->i_ip;
	bp = io->i_bp;
	ip->i_data = (handle *)&bf_incore;

	if ( Inbuf == 0 ) {
		if ( (Inbuf = mbmalloc(FsBlockSize)) == 0 )
			return(0);
	}

	ino = ip->i_number;
	bp->b_iobn = FsbToDb(itod(ino));
	bp->b_iobase = Inbuf;
	bp->b_bcount = FsBlockSize;

	dprintf(("bell_iread(%d) blk %d\n",ino,bp->b_iobn));

	if ( _devread(io) < 0 )
		return(0);
	dip = (struct dinode *)Inbuf + itoo(ino);

	/* fill in inode from what we read of the disk	*/
	ip->i_mode = dip->di_mode;
	ip->i_size = dip->di_size;
	io->i_offset = 0;
	dprintf(("mode 0x%x, size %d\n",ip->i_mode, ip->i_size));

	/* now unpack the block numbers (l3toi)*/
	cp = (u_char *)dip->di_addr;
	tp = (u_char *)bf_incore.ii_addr;
	for ( i=0; i < NADDR; i++ ) {
		*tp++ = 0;
		*tp++ = *cp++;
		*tp++ = *cp++;
		*tp++ = *cp++;
	}
	return(ip);
}

/* a structure to help with bmap */
struct indirect {
	daddr_t i_phybno;	/* physical block no	*/
	daddr_t i_flbno;	/* first logical block# */
	daddr_t i_llbno;	/* last logical block #	*/
	daddr_t *i_inbuf;	/* a buffer for it	*/
} in_info;
#define NDIRECT	NADDR-3
int napb;			/* number of addresses per block	*/

/*
 * convert io->i_offset into a physical block number on a slice
 */
bell_bmap(io, asz)
register struct iob *io;
register unsigned *asz;
{
	register struct inode *ip;
	register struct buf *bp;
	register struct incore_dinode *dip;
	register daddr_t *addrp;
	register daddr_t lbn;
	register daddr_t tbn;
	register daddr_t inc;
	char *mbmalloc();

	ip = io->i_ip;
	bp = io->i_bp;
	dip = (struct incore_dinode *)ip->i_data;

	dprintf(("bellbmap dev 0x%x, offset 0x%x\n",ip->i_dev,io->i_offset));

	lbn = lblkno(io->i_offset);
	if ( lblkoff(io->i_offset) >= BBSIZE ) {
		inc = 1;
		*asz = BBSIZE;	/* maximum io size is the block size */
	} else {
		inc = 0;
		*asz = FsBlockSize;	/* maximum io size is the block size */
	}
	/* blocks 0 to NADDR-4 are direct blocks	*/
	if ( lbn < NADDR-3 ) {
		lbn = FsbToDb(dip->ii_addr[lbn]);
		dprintf(("bellbmap returning blk 0x%x\n",lbn));
		return(lbn+inc);
	} else {				/* indirect blocks */
		if ( in_info.i_inbuf == 0 ) {	/* get the buffer */
			if ( (in_info.i_inbuf = (daddr_t *)mbmalloc(FsBlockSize)) 
									== 0 ) {
				printf("Can't get indirect buffer\n");
				return(0);
			}
			in_info.i_flbno = 0;
			in_info.i_llbno = 0;
			napb = FsBlockSize/sizeof (daddr_t);
		}

		while ( (lbn < in_info.i_flbno) || (lbn > in_info.i_llbno) ) {
			/* need to read in the indirect block	*/
			if ( lbn < NDIRECT + napb ) {
				/* single indirect */
				tbn = dip->ii_addr[NDIRECT];
				in_info.i_flbno = NDIRECT;
				in_info.i_llbno = in_info.i_flbno + napb - 1;
			 } else {
				/* Double indirect	*/
				tbn = dip->ii_addr[NDIRECT+1];
				/* read in tbn to buffer	*/
				bp->b_iobn = FsbToDb(tbn);
				bp->b_iobase = (caddr_t)in_info.i_inbuf;
				bp->b_bcount = FsBlockSize;
				if ( _devread(io) < 0 )
					return(0);
				/* index into block */
				addrp = in_info.i_inbuf;
				addrp += (lbn - (NDIRECT+napb))/napb;
				tbn = *addrp;
				in_info.i_flbno = ( (lbn - (NDIRECT+napb))/napb
							+ (NDIRECT+napb) );
				in_info.i_llbno = in_info.i_flbno + napb - 1;
			}
			/* read in tbn to buffer	*/
			bp->b_iobn = FsbToDb(tbn);
			bp->b_iobase = (caddr_t)in_info.i_inbuf;
			bp->b_bcount = FsBlockSize;
			if ( _devread(io) < 0 )
				return(0);

		}
		/* indirect block is in buffer	*/
		addrp = in_info.i_inbuf;
		addrp += ( lbn - in_info.i_flbno);	/* index into block */
		dprintf(("bellbmap returning blk 0x%x\n",*addrp));
		return(FsbToDb(*addrp)+inc);
		
	}
}
