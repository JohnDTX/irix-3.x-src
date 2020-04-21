/*	@(#)dcopy.c	1.3	*/
/*	COMPILE:	cc -O dcopy.c -i -o dcopy	*/
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/etc/RCS/dcopy.c,v 1.1 89/03/27 15:37:36 root Exp $";
/*
 * $Log:	dcopy.c,v $
 * Revision 1.1  89/03/27  15:37:36  root
 * Initial check-in for 3.7
 * 
 * Revision 1.3  85/04/30  14:24:49  bob
 * Added symbolic links.
 * 
 * Revision 1.2  85/03/08  23:39:30  bob
 * 
 */
#include <sys/param.h>
#ifdef howmany
#undef howmany
#endif
#ifdef roundup
#undef roundup
#endif
#include <sys/types.h>
#include <sys/filsys.h>
#include <sys/dir.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/fblk.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>

#define	MAXCYL		1600		/* Maximum # of blocks on a cylinder */
					/* RP07 is this big */
#define STEPSIZE	   9		/* Default interleaving number */
#define CYLSIZE		 400		/* Default # of blocks on a cylinder */
#define	OLD		   7		/* Default for -a */
#define VERYOLD		 127		/* Too old to care to sort in directory */
#define	MAXDIR		  20		/* Biggest direct, #blocks */
#define	NIBLOCKS	 110		/* Max. inode blocks read at once */
#define	MIBLOCKS	  11		/* Min. inode blocks to be read */
#define	SECPDAY		1440		/* seconds per earth day */
#define	DIRPBLK		(BSIZE/sizeof (struct direct)) /* Dir entries/block */
#define	NBUFIN		   3		/* Number in indirect block buffers */
#define	GOOD		   2		/* Not NULL or EOF */
#define	B_WRITE		   1		/* For alloc */
#define	B_READ		   0		/* ditto */
#define FAIL		  -1

#define	howmany(a,b)	(((a)+((b)-1))/(b))
#define	roundup(a,b)	(howmany(a,b)*(b))
#define	min(a,b)	((a) < (b) ? (a) : (b))
#define	super0		_super0.b_sup
#define	super1		_super1.b_sup
#define	SUPER0		_super0.b_sup.correct_fs.fs
#define	SUPER1		_super1.b_sup.correct_fs.fs

/*
 * Smalli is the information contained in the inodes
 * that needs to be remembered between the time the
 * files are copies and the directory entries are updated.
 */

struct smalli
{
	ino_t	  	index;
};

/*
 * Dblock is a union for several of the forms a disk block may take.
 */

union dblock
{
	struct direct	b_dir[DIRPBLK];
	char		b_file[BSIZE];
	struct	fblk	b_free;
	daddr_t		b_indir[NINDIR];
	struct	correct_filsys {
		char	fill1[DEV_BSIZE];
		union	{
			char	fill2[DEV_BSIZE];
			struct	filsys	fs;
		} correct_fs;
	} b_sup;
};

/*
 * Inbuf is the internal in-core copies of new (and improved?) indirect
 * blocks along with some other information to make the buffering work.
 */

struct inbuf
{
	daddr_t		i_bn;
	int		i_indlev;
	daddr_t		i_indir[NINDIR];
};

/*
 * All is the information needed to allocate disk blocks (in alloc())
 * All[0] contains the information for the young files (or all files
 * if no -a is in effect) and all[1] has the info for old files.
 * Blocks for young files are allocated from the begining of the file
 * system and old files start on the last cylinder and work their way in.
 */

struct all
{
	daddr_t	a_baseblock;	/* First block of current cylinder */
	int	a_pblk;		/* Position in multiblock block (-c) */
	int	a_nalloc;	/* Counts blocks allocated in this cylinder */
	char	a_flg[MAXCYL];	/* Remembers which blocks we have given away */
	int	a_lastb;
};

char	xflg = 0;	/* -x makes executable files contiguous */
char	dflg = 1;	/* -d sorts directory entries */
char	fflg = 0;	/* -f lets you specify file system and ilist size */
char	aflg = 1;	/* -a separates old files from new ones */
char	zflg = 0;	/* -z for debugging output */
char	vflg = 0;	/* -v is verbose */

short	bsize = 1;	/* Number of physical blocks/logical block (for -c) */
int	infs;		/* Source file system file descriptor */
int	outfs;		/* Destination file system file descriptor */
int	tempfd;		/* temporary file for internal buffers	*/
int	pass;		/* What are we doing these days */
union	dblock	_super0;	/* Old superblock */
union	dblock	_super1;	/* New superblock */
short	old;		/* Files not accessed since the old days are put far away */
long	oldtime;	/* The actual date for above */
int	cylsize;	/* User specified cylinder size */
int	stepsize;	/* User specified interleaving size */
short   pblock;		/* Current physical block in log. block (0-bsize-1) */
ino_t	inext = 1;	/* Counter for new i-list */
ino_t	incnt;		/* Counter for old i-list */
ino_t	icount;		/* Total count of inodes */
daddr_t	fmin;		/* block number for first data block	*/

daddr_t	iblkno;			/* Block number of inodes just read */
daddr_t	oblk = (daddr_t) 2;	/* Block number of inodes to be written */
int	niblks;			/* nbufi/INOPB */
daddr_t	rblock;		/* Current logical block from file or directory */
daddr_t	wblock;		/* Logical block to be written to file or direc. */
ino_t	inum;		/* Index into block of inodes just read in */
ino_t	onum;		/* Index into block of inodes to be written out */
ino_t	nbufi;		/* Number of inodes read in at once */
char	*dspc;		/* Remembered ages (in days) */
daddr_t	dspc_off;	/* file offset pointer for time, only used with '-d' */
struct	smalli	*ispace;/* Inode information, space gotten from malloc() */
daddr_t ispace_off;	/* offset into temporary file to ispace structure */
struct	dinode	node0;	/* Source inode of current file */
struct	dinode	node1;	/* Destination inode */
struct	dinode	*ibuf;	/* Current bunch of inodes just read */
struct	dinode	out_node[INOPB];/* Current block of inodes to be written */
struct	all	all[2];	/* Alloc() information */
union dblock in;	/* Block for file and directory data in */
union dblock out;	/* Block for file and directory data out */
daddr_t	addr0[NADDR];	/* Usable disk addresses from node0 */
daddr_t	addr1[NADDR];	/* New disk addresses to be put on node1 */
struct	inbuf	inbuf[2][NBUFIN];	/* Indirect address buffers */
char	*tempnm = "/tmp/dpyXXXXXX"; /* temporary file name	*/
int	blk_size = BSIZE;		/* blocksize of temp file	*/
daddr_t	blk_alloc = 0;			/* number of temp blocks allocated*/
daddr_t bmap();
daddr_t	alloc();
daddr_t allspace();
daddr_t countfree();
char	*getpwoff();
char	*malloc();
daddr_t	 *getindir();
char	*sbrk();
long	ulimit();
long	lseek();
int	older();
int	intr();

#ifdef	DEBUG
/*
 * For Debugging alloc.
 */
#define	PLACE(b)	map[((short) b)>>3] |= (1<<(((short) b) & 7))
#define	ISTHERE(b)	(map[((short) b)>>3] & (1 <<(((short) b) & 7)))
char	map[1000];
#endif

main(argc, argv)
char **argv;
{
	long	time(), atol();
	long	t;
	int	iage;
	daddr_t	d;
	register i;
	register struct smalli	*sip;
	daddr_t	sip_off;
	char 	*bp;
	char **args;

	sync();
	args = argv + 1;
	while(--argc > 0 && **++argv == '-') {
	       	switch(*++*argv) {

		case 's':	/* supply device information */
			stype(++*argv);
			break;

/* NO LONGER SUPPORTED - no good reason for existence
		-cN make every N consecutive block of a file contiguous

		case 'c':
			if((bsize = atoi(++*argv)) == 0)
				err("Bad size (-c)");
			break;
   NO LONGER SUPPORTED */

/* NO LONGER SUPPORTED - no good reason for existence
		-x  force executable files to be contiguous

		case 'x':
			xflg++;
			break;
   NO LONGER SUPPORTED */

		case 'a':	/* alter file postion based on access times */
			aflg = 1;
			old = *++*argv ? atoi(*argv) : 0;
			if(old == 0)
				aflg = 0;	/* no movement */
			break;

		case 'd':	/* leave directory order as is */
			dflg = 0;
			break;

		case 'f':	/* supply filesystem and inode size (blocks) */
			fflg++;
			SUPER1.s_fsize = atol(++*argv);
			while(**argv != '\0' && *(*argv)++ != ':')
				;
			SUPER1.s_isize = atoi(*argv) + 2;
			break;

		case 'z':	/* turn on DEBUG output */
			zflg++;
			break;

		case 'v':	/* verbose mode */
			vflg++;
			break;

		default:
			err("Unknown arg (-%c)", **argv);
		}
	}
	if(argc != 2)
		err("usage: dcopy [ options ] source-fs dest-fs");
	t = time(0);
	if(aflg)
		oldtime = t - old * SECPDAY;
	if((infs = open(argv[0], O_RDONLY)) < 0)
		err("Can't open %s", argv[0]);

	if((outfs = open(argv[1], O_RDWR)) < 0)
		err("Can't open %s", argv[1]);

	args[1] = 0;
	/*
	 * Copy bootstrap (block 0) and read in super block.
	 */

	getblk(&super0, (daddr_t) 0);
	putblk(&super0, (daddr_t) 0);
#ifdef	notdef
	getblk(&super0, (daddr_t) 1);
#else
	getblk(&super0, (daddr_t) 0);
#endif
	/*
	 * Assign and check new file system sizes.
	 */
	if(SUPER1.s_fsize == 0)
		SUPER1.s_fsize = SUPER0.s_fsize;
	if(SUPER1.s_isize <= 2)
		SUPER1.s_isize = SUPER0.s_isize;
	if(fflg && (SUPER1.s_isize < counti() + 2 || SUPER0.s_fsize -
	  (SUPER0.s_isize + countfree()) > SUPER1.s_fsize - SUPER1.s_isize))
		err("Bad file system sizes (-f)");
	/*
	 * Compute interleaving and remember it in new superblock.
	 */
	if(cylsize == 0 || stepsize == 0) {
		stepsize = SUPER0.s_dinfo[0];	/* Assumes old and new */
		cylsize = SUPER0.s_dinfo[1];	/* disks are the same type */
	}
	if(vflg)
	{
		printf("old filesize = %ld, old inode size = %u\n",
			SUPER0.s_fsize,SUPER0.s_isize);
		printf("old stepsize = %d, old cylinder size = %d\n",
			stepsize,cylsize);
	}
	if(stepsize > cylsize || stepsize <= 0 ||
	    cylsize <= 0 || cylsize > MAXCYL) {
		stepsize = STEPSIZE;
		cylsize = CYLSIZE;
	}
	SUPER1.s_dinfo[0] = stepsize;
	SUPER1.s_dinfo[1] = cylsize;
	/*
	 * Set up block allocation and get mem for inode information.
	 */
	fmin = (daddr_t)SUPER1.s_isize;
	all[0].a_baseblock = roundup(fmin, cylsize) - cylsize;
	all[0].a_nalloc = fmin % cylsize;
	all[0].a_pblk  = all[1].a_pblk = 1;
	d = roundup(SUPER1.s_fsize, cylsize);
	all[1].a_baseblock = d - cylsize;
	all[1].a_nalloc = d - SUPER1.s_fsize;
	if(vflg)
	{
		printf("new filesize = %ld, new inode size = %u\n",
			SUPER1.s_fsize,SUPER1.s_isize);
		printf("new stepsize = %d, new cylinder size = %d\n",
			stepsize,cylsize);
	}

	d = (daddr_t)(SUPER0.s_isize - 2) * INOPB * sizeof(struct smalli);
	ispace_off = sip_off = allspace(d);
	if(dflg) {
		dspc_off = allspace(d/2);
	}
	/*
	 * Get mem for inodes
	 */
	getimem();
	inum = nbufi;
	iblkno = 2 - niblks;
	/*
	 * Copy files
	 */
	icount = (SUPER0.s_isize - 2) * INOPB;
	pass = 1;
	signal(SIGQUIT, intr);
	signal(SIGINT, intr);
	printf("Pass 1: Reorganizing file system\n");
	for(incnt = 1; incnt <= icount; incnt++) {
		if((sip = (struct smalli *)getpwoff(sip_off)) == NULL) {
			err("couldn't get sip pointer\n");
		}
		geti();
		if(node0.di_mode & IFMT)  {
			copyi();
			sprintf(*args, "On i-num %d of %u", incnt, icount);
			sip->index = inext++;
		}
		else
			sip->index = 0;
		if(dflg) {
			iage = (t - node0.di_atime) / SECPDAY;
			if((bp = getpwoff(dspc_off + incnt)) == NULL) {
				err("couldn't get directory time pointer");
			}
			if( (node0.di_mode & IFMT) == IFDIR )
				*bp = 0;
			else
				*bp = min(iage, VERYOLD);
		}
		sip_off += sizeof(struct smalli);
	}
	flushi();
	/*
	 * Now make the source filesystem the destination filesystem
	 * so getblk() will get blocks from there, which will have
	 * already compressed the directories.
	 */
	infs = outfs;
	/*
	 * Reinitialize inode pointers.
	 */
	inum = nbufi;
	iblkno = 2 - niblks;
	onum = (ino_t) 0;
	oblk = (daddr_t) 2;
	/*
	 * Fix i-numbers in directory entries, with deletion of
	 * zero entries and sorting, if desired.
	 */
	pass = 2;
	printf("Pass 2: Fixing inums in directories\n");
	for(incnt = 1; incnt < inext; incnt++) {
		geti();
		if((node0.di_mode & IFMT) == IFDIR)
			reldir();
	}
	/*
	 * Make a new i-list and freelist
	 */
	pass = 3;
	printf("Pass 3: Remake freelist\n");
	icount = (SUPER1.s_isize - 2) * INOPB;
	i = inext;
	while(i <= icount && SUPER1.s_ninode < NICINOD)
		SUPER1.s_inode[SUPER1.s_ninode++] = i++;
	freelist();
	/*
	 * Copy superblock and sync to make sure it all gets done.
	 */
	putblk(&SUPER1, (daddr_t) 1);
	sync();
	if(vflg) {
		printf("Files:		%d\n", --inext);
		printf("Free blocks in:		%ld\nFree blocks out:	%ld\n",
			SUPER0.s_tfree, SUPER1.s_tfree);
	}
	printf("Complete\n");

/*
	if(zflg > 1)
		for(d=0; d<SUPER1.s_fsize;d++)
			if(!ISTHERE(d))
				fprintf(stderr, "%lu MISSING\n", d);
 */

	exit(0);
}

/*
 * Stype processes the user's -s argument.
 * Stolen from fsck.c
 */

stype(p)
register char *p;
{
	if(*p == 0)
		return;
	if (*(p+1) == 0) {
		if (*p == '3') {
			cylsize = 200;
			stepsize = 5;
			return;
		}
		if (*p == '4') {
			cylsize = 418;
			stepsize = 7;
			return;
		}
	}
	cylsize = atoi(p);
	while(*p && *p != ':')
		p++;
	if(*p)
		p++;
	stepsize = atoi(p);
	if(stepsize <= 0 || stepsize > cylsize ||
	cylsize <= 0 || cylsize > MAXCYL) {
		fprintf(stderr, "Invalid -s argument, defaults assummed\n");
		cylsize = stepsize = 0;
	}
}

/*
 * Count inodes
 */

counti()
{
	register i, j, n;
	struct dinode in_node[INOPB];

	n = 0;
	for(i = 2; i < SUPER0.s_isize; i++) {
		getblk(in_node, (daddr_t) i);
		for(j = 0; j < INOPB; j++)
			if(in_node[j].di_mode & IFMT)
				n++;
	}
	return((n + INOPB - 1) / INOPB);
}

/*                                                                             /
 * Countfree counts the free blocks (obviously).
 */

daddr_t
countfree()
{
	daddr_t n, m;

	n = SUPER0.s_nfree;
	if(n) {
		m = SUPER0.s_free[0];
		while(m) {
			getblk(&in.b_free, (daddr_t) m);
			n += in.b_free.df_nfree;
			m = in.b_free.df_free[0];
		}
	}
	return(n);
}

/*
 * Geti sets the external structure node0 with the disk inode information
 * for the inum'th inode in chunk iblkno. The i-list is read in a block
 * at a time and inum is incremented with each call.
 */

geti()
{
	if(inum >= nbufi) {
		lseek(infs, (long) BSIZE * (iblkno += niblks), 0);
		if(zflg)
			fprintf(stderr, "Reading block %ld of inodes\n",
				iblkno);
		if(read(infs, ibuf, niblks * BSIZE) != niblks * BSIZE)
			err("Inode read error: block: %ld, size %d\n",
				iblkno, niblks * BSIZE);
		inum = 0;
	}
	node0 = ibuf[inum++];
}

/*
 * Puti saves the current inode (node1) in the current block of
 * inodes to be written out.  If the block is full, it writes it
 * out.
 */

puti()
{
	if(onum >= INOPB) {
		putblk(out_node, oblk++);
		onum = 0;
	}
	out_node[onum++] = node1;
}

/*
 * Flushi writes out the last block of inodes, and zeros the rest of the
 * ilist.
 */

flushi()
{
	register i;

	i = onum * sizeof(struct dinode);
	clear(out_node + onum, BSIZE - i);
	putblk(out_node, oblk);
	clear(out_node, BSIZE);
	while(++oblk < SUPER1.s_isize)
		putblk(out_node, oblk);
}

/*
 * Copyi copies the inode (with associated blocks, if it is a file)
 * which is stored in node0.
 */

copyi()
{
	int type;

	node1 = node0;
	switch(type = (node1.di_mode & IFMT)) {

	case IFDIR:

#ifdef	IFLNK
	case IFLNK:
#endif	IFLNK

	case IFREG:
		fixaddr();
		type == IFREG ? copyfile() : copydir();
		flushaddr();
		puti();
		return;

	default:
		fprintf(stderr, "I=%d Unexpected mode (%o)\n",
				inext, node1.di_mode);
		/* fall into ... */

	case IFCHR:

	case IFBLK:

	case IFIFO:
		puti();
		return;
	}
}

/*
 * Copy and compress directory files
 */

copydir()
{
	register struct direct *odp, *idp;
	register int nent;
	int dirent, r;

	dirent = 0;
	odp = out.b_dir;
	nent = node0.di_size/sizeof(struct direct);
	while((r = r_block(in.b_dir)) != EOF) {
		if(r == NULL)
			continue;
		for( idp = in.b_dir; (idp < &in.b_dir[DIRPBLK]) && (nent-->0); idp++ )
			if(idp->d_ino) {
				dirent++;
				*odp++ = *idp;
				if(odp >= &out.b_dir[DIRPBLK]) {
					w_block(out.b_dir);
					odp = out.b_dir;
				}
			}
	}
	if(odp != out.b_dir) {
		clear(odp, ((char *) &out.b_dir[DIRPBLK]) - ((char *) odp));
		w_block(out.b_dir);
	}
	node1.di_size = dirent * sizeof(struct direct);
}

/*
 * Copyfile reads in the current file and writes it back out,
 * taking care not to write null blocks out.
 */

copyfile()
{
	register r;

	while((r = r_block(in.b_file)) != EOF)
		if(r == NULL)
			wblock++;	/* Bmap does the dirty work */
		else
			w_block(in.b_file);
}

/*
 * Alloc allocates disk blocks according to the user specified
 * options.  The file mode (for -x) is in node0.di_mode.  The
 * access time (for -a) is in node0.di_atime.  Depending on
 * whether -a is in effect and the file is old, we choose all[0]
 * or all[1] to provide the information about the current cylinder.
 * The array a_flg[] keeps track of the disk blocks we have already
 * allocated in this cylinder.  A_pblock keeps track of which
 * physical block we are in in the current logical block and
 * has a range from zero to bsize-1.  A_nalloc counts how many blocks
 * already given away in this cylinder.
 */

daddr_t
alloc()
{
	register i;
	register struct all *ap;
	static overlap, ovw;
	int which;
	daddr_t blk;

	which =	overlap ? ovw : (aflg && (node0.di_atime <= oldtime) && ((node0.di_mode &IFMT) != IFDIR) );
	ap = all + which;
	if(ap->a_nalloc++ >= cylsize)	/* New cylinder needed */
		do {
			/*
			 * If we already overlapped, we are now out of space.
			 */
	        	if(overlap)
	        		return((daddr_t) 0);
			clear(ap->a_flg, sizeof(ap->a_flg));
			i = 0;
			ap->a_pblk = 1;
			ap->a_nalloc = 0;
			ap->a_baseblock += which ? -cylsize : cylsize;
	        	/*
	        	 * When freelist() gets into the cylinder of the old files,
			 * or the young and old files have overlapped on the same
	        	 * cylinder, we have to make sure we only free the blocks
	        	 * not already allocated to a file.  We do this by making
			 * sure to use the all[] structure which came first.
	        	 */
	       		if(all[0].a_baseblock == all[1].a_baseblock) {
				overlap++;
				ovw = 1 - which;
				ap = &all[ovw];
			}
		}  while(ap->a_nalloc++ >= cylsize);
	else {
		if(ap->a_pblk++ < bsize || (xflg && (node0.di_mode & IEXEC)))
			i = ap->a_lastb + 1;
		else {
			ap->a_pblk = 1;
			i = ap->a_lastb + stepsize;
		}
	}
	i %= cylsize;
	do {
	       	while(ap->a_flg[i])
	       		i = (i + 1) % cylsize;
		ap->a_flg[i]++;
		blk = ap->a_baseblock + i;
		ap->a_lastb = i;
	} while(blk < fmin || blk >= SUPER1.s_fsize);
	if(zflg > 1 && blk%cylsize == 0) {
		fprintf(stderr, "blk. no. %lu\n", blk);
		for(i=0; i<2; i++)
			fprintf(stderr, "[%d]: b:%lu p:%u n:%u l:%u\n", i,
			all[i].a_baseblock, all[i].a_pblk, all[i].a_nalloc, all[i].a_lastb);
	}

#ifdef	DEBUG
	if(zflg > 1) {
		if(ISTHERE(blk))
			fprintf(stderr, "DUP %lu\n", blk);
		PLACE(blk);
		printf("%lu ", blk);
	}
#endif

	return(blk);
}

err(fmt, x1, x2, x3, x4)
char *fmt;
{
	fprintf(stderr, "dcopy: ");
	fprintf(stderr, fmt, x1, x2, x3, x4);
	fprintf(stderr, "\n");
	exit(1);
}

clear(p, b)
register char *p;
register b;
{
	while(b--)
		*p++ = 0;
}

/*
 * R_block reads in a block from the current file.
 */

r_block(b)
char *b;
{
	daddr_t bn;

	if((bn = bmap(rblock++, B_READ)) == (daddr_t) (-1))
		return(rblock > howmany(node0.di_size, BSIZE) ? EOF : NULL);
	getblk(b, bn);
	return(GOOD);
}

/*
 * W_block writes out a block to the current file.
 */

w_block(b)
char *b;
{
	daddr_t bn;

	if((bn = bmap(wblock++, B_WRITE)) == (daddr_t) (-1))
		return(EOF);
	putblk(b, bn);
	return(!EOF);
}

/*
 * Bmap defines the structure of file system storage
 * by returning the physical block number on a device given the
 * inode and the logical block number in a file.
 * Stolen from os/subr.c.
 */

daddr_t
bmap(bn, rwflg)
daddr_t bn;
{
	register daddr_t *dp;
	register i;
	register daddr_t	*inp;
	int j, sh;
	daddr_t nb;

	dp = (rwflg == B_READ) ? addr0 : addr1;
	/*
	 * blocks 0..NADDR-4 are direct blocks
	 */
	if(bn < NADDR-3) {
		i = bn;
		nb = dp[i];
		if(nb == 0) {
			if(rwflg==B_READ || (nb = alloc())==NULL)
				return((daddr_t)-1);
			dp[i] = nb;
		}
		/* We should always be doing block writes at EOF in pass1 */
		else if(rwflg != B_READ && pass == 1)
			err("Write not at EOF");
		return(nb);
	}
	/*
	 * addresses NADDR-3, NADDR-2, and NADDR-1
	 * have single, double, triple indirect blocks.
	 * the first step is to determine
	 * how many levels of indirection.
	 */
	sh = 0;
	nb = 1;
	bn -= NADDR-3;
	for(j=3; j>0; j--) {
		sh += NSHIFT;
		nb <<= NSHIFT;
		if(bn < nb)
			break;
		bn -= nb;
	}
	/*
	 * Check for HUGE file.
	 */
	if(j == 0)
		return((daddr_t)-1);
	/*
	 * fetch the address from the inode
	 */
	nb = dp[NADDR-j];
	if(nb == 0) {
		if(rwflg==B_READ || (nb = alloc())==NULL)
			return((daddr_t)-1);
		dp[NADDR-j] = nb;
	}
	/*
	 * fetch through the indirect blocks
	 */
	for(; j<=3; j++) {
		inp = getindir(nb, 4 - j, rwflg);
		sh -= NSHIFT;
		i = (bn>>sh) & NMASK;
		nb = inp[i];
		if(nb == 0) {
			if(rwflg==B_READ || (nb = alloc())==NULL)
				return((daddr_t)-1);
			inp[i] = nb;
		}
		else if(j == 3 && rwflg != B_READ && pass == 1)
			err("Write not at EOF");
	}
	return(nb);
}

/*
 * Getindir returns a pointer to a block of indirect addresses.
 * The first time getindir is called with a new bno, the list is
 * searched for free buffers, and if none are found, a single or double
 * indirect block is used (after writing it out).  The found block is cleared
 * and a pointer to it is returned. Once an indirect block is written
 * out it will not be read in again from the destination file system.
 * Thus, NBUFIN has to be large enough to accomadate all indirect blocks
 * needed to access any block in the file (NBUFIN = 3 for triple indir-
 * ection) and since the blocks in the file are always accessed in order
 * and single indirect blocks are reused before double indirect blocks,
 * no more than 3 indirect blocks are ever needed. Thus making NBUFIN
 * bigger than 3 doesn't gain anything.
 * Lev is the level of indirection passed from bmap.
 */

daddr_t *
getindir(bno, lev, rwflg)
daddr_t	bno;
{
	register struct inbuf *ip;
	register levcnt;
	struct inbuf *instart, *inend;

	instart = inbuf[rwflg];
	inend = instart + NBUFIN;
	for(levcnt = -1; levcnt <= 2; levcnt++) {
		for(ip = instart; ip < inend; ip++)
			if(levcnt < 0 && ip->i_bn == bno)	/* A hit */
				return(ip->i_indir);
			else if(levcnt == ip->i_indlev) {
				if(levcnt && rwflg == B_WRITE)
					putblk(ip->i_indir, ip->i_bn);
				ip->i_indlev = lev;
				ip->i_bn = bno;
				if(pass != 1 || rwflg == B_READ)
					getblk(ip->i_indir, bno);
				else
					clear(ip->i_indir, BSIZE);
				return(ip->i_indir);
			}
	}
	/* This error "cannot happen" */
	err("getindir: no freeable blocks");
}

/*
 * Getblock() reads block bno of the source f.s.
 * It could be made smarter
 */

getblk(p, bno)
char *p;
daddr_t	bno;
{
	if(zflg > 1)
		fprintf(stderr, "Reading block %lu\n", bno);
	if(lseek(infs, (long) bno * BSIZE, 0) == (long) -1)
		err("Can't seek to b.n. %lu on source", bno);
	if(read(infs, p, BSIZE) != BSIZE)
		err("Read error: block no %lu", bno);
}

/*
 * Putblk() writes block bno of the source f.s.
 * it could buffer, but it dont.
 */

putblk(p, bno)
char *p;
daddr_t	bno;
{
	if(zflg > 1)
		fprintf(stderr, "Writing to %lu\n", bno);
	if(lseek(outfs, (long) bno * BSIZE, 0) == (long) -1)
		err("Can't seek to b.n. %lu on destination", bno);
	if(write(outfs, p, BSIZE) != BSIZE)
		err("Write error: block no. %lu", bno);
}

/*
 * Reldir() fixes up the i-numbers in directory files
 * and sorts entries, if -d option in effect.
 */

reldir()
{
	register struct direct *dsp, *dend;
	char *mspace;
	register unsigned i;

	fixaddr();
	i = roundup(node0.di_size, BSIZE);
	if((mspace = malloc(i))  == NULL)
		err("Can't get mem for directory");
	dsp = (struct direct *) mspace;
	dend = dsp + node0.di_size/sizeof(struct direct);
	for(; dsp < dend; dsp += DIRPBLK)
		r_block(dsp);
	dsp = (struct direct *) mspace;
	if(dend - dsp < 2)
		fprintf(stderr, "Truncated directory I=%d\n", incnt);
	else if(dflg)
		qsort(dsp + 2, dend - dsp - 2, sizeof(struct direct), older);
	for(; dsp < dend; dsp++)
	{
		if((ispace = (struct smalli *)getpwoff(ispace_off +
			((long)(dsp->d_ino -1) * sizeof(struct smalli)) )) == NULL)
		{
			err("Coundn't get ispace pointer\n");
		}
		dsp->d_ino = ispace->index;
	}
	for(dsp = (struct direct *) mspace; dsp < dend; dsp += DIRPBLK)
		w_block(dsp);
	free(mspace);
}

/*
 * Older - called from qsort, returns + if the directory entry pointed
 * to by d1 is older than the directory entry pointed to by d2.
 * The ages were found out in the first pass and gotten by indexing
 * ispace, the smalli structure;
 */

older(d1, d2)
struct direct *d1, *d2;
{
	char *b1,*b2;
	int i;

	if((b1 = getpwoff(dspc_off + (d1->d_ino))) == NULL){
		err("cannot get offset to time\n");
	}
	i = *b1;
	if((b2 = getpwoff(dspc_off + (d2->d_ino))) == NULL){
		err("cannot get offset to time\n");
	}
	return(i - *b2);
}

/*
 * Fixaddr() sets up variables such that sucessive read and write calls
 * start from the beginning of the data for the current inode.
 */

fixaddr()
{
	register struct inbuf *ip;

	l3tol(addr0, node0.di_addr, NADDR);
	rblock = wblock = (daddr_t) 0;
	pblock = 0;
	if(pass == 1)
		clear(addr1, sizeof(addr1));
	else
		l3tol(addr1, node0.di_addr, NADDR);
	for(ip = inbuf[B_READ]; ip < &inbuf[B_READ][NBUFIN]; ip++)
		ip->i_indlev = 0;
	for(ip = inbuf[B_WRITE]; ip < &inbuf[B_WRITE][NBUFIN]; ip++)
		ip->i_indlev = 0;
}

/*
 * Flushaddr() is called to fix up the current inode, and write out any
 * buffered indirect blocks.
 */

flushaddr()
{
	register struct inbuf *ip;

	for(ip = inbuf[B_WRITE]; ip < &inbuf[B_WRITE][NBUFIN]; ip++)
		if(ip->i_indlev)
			putblk(ip->i_indir, ip->i_bn);
	ltol3(node1.di_addr, addr1, NADDR);
}

/*
 * Freelist() creates the new free list and fills in the superblock.
 * It adds blocks to the freelist in the same order that the system
 * will ask for them;
 */

freelist()
{
	register daddr_t *fp;
	register i;
	daddr_t frblk;

	frblk = (daddr_t) 0;
	SUPER1.s_nfree = 0;
	SUPER1.s_ninode = 0;
	SUPER1.s_flock = 0;
	SUPER1.s_ilock = 0;
	SUPER1.s_fmod = 0;
	SUPER1.s_ronly = 0;
	SUPER1.s_tfree = 0;
	SUPER1.s_tinode = icount - inext + 1;
	SUPER1.s_time = SUPER0.s_time;
	SUPER1.s_dinfo[2] = SUPER0.s_dinfo[2];
	SUPER1.s_dinfo[3] = SUPER0.s_dinfo[3];
	for(i = 0; i < 6; i++)
		SUPER1.s_fname[i] = SUPER0.s_fname[i];

	for(i = 0; i < 6; i++)
		SUPER1.s_fpack[i] = SUPER0.s_fpack[i];

	node0.di_mode = IFREG;		/* For alloc() */
	aflg = 0;			/* ditto */
	i = 0;
	fp = &SUPER1.s_free[NICFREE];
	while(1) {
		if((*--fp = alloc()) != (daddr_t) 0) {
			i++;
			SUPER1.s_tfree++;
		}
		if(i == NICFREE || *fp == (daddr_t) 0) {
			if(*fp == (daddr_t) 0) {
				register daddr_t *cfp;
				register k;

				cfp = (frblk == (daddr_t) 0) ? SUPER1.s_free
					    : in.b_free.df_free;
				for(k = i; k-- > 0;)
					*++cfp = *++fp;
				i++;
			}
			if(frblk) {
				in.b_free.df_nfree = i;
				putblk(&in.b_free, frblk);
			}
			else
				SUPER1.s_nfree = i;
			if((frblk = *fp) == (daddr_t) 0)
				return;
			fp = &in.b_free.df_free[NICFREE];
			clear(&in.b_free, BSIZE);
			i = 0;
		}
	}
}

intr(sig)
{
	printf("Pass %d, inum %u of %u\n", pass, incnt, icount);
	if(sig == SIGQUIT) {
		printf("No longer catching interupts\n");
		signal(SIGINT, SIG_DFL);
		return;
	}
	signal(SIGINT, intr);
}

getimem()
{
	register unsigned size, avail;
	char *space;

	/* determine what space is available */
	avail = (ulimit(3,0L) - (unsigned)sbrk(0))/BSIZE;
	if( avail > (MAXDIR+NIBLOCKS+4) )
		size = MAXDIR+NIBLOCKS+4;
	else
		size = avail;
	while( (space = malloc(size*BSIZE)) == 0 )
		size--;
	free(space);
	if( size < (MAXDIR+MIBLOCKS+4) )
		err("Can't get inode buffer space");
	size -= MAXDIR + 4;
	if( size > NIBLOCKS )
		size = NIBLOCKS;
	if((ibuf = (struct dinode *) malloc(size*BSIZE)) == NULL)
		err("Can't get inode buffer space");
	niblks = size;
	nbufi = niblks * INOPB;
	printf("Available mem %u, got %u for inodes (that's %u inodes)\n",
		avail*BSIZE, size*BSIZE, nbufi);
}

/*	Allocate space in a file which can be accessed through
 *	routine getpwoff.
 */

long
allspace(size)
long size;
{
	extern long putblock();
	extern int blk_size;
	extern daddr_t blk_alloc;
	extern long curr_blk[];
	extern long cntr_blk;
	extern char *float_ptr[];
	register i;
	daddr_t next_blk;
	daddr_t offset;
	daddr_t blk;
	char *bp;

	if( zflg  > 1)
	{
		fprintf(stderr,"size requested = %ld\n",size);
		fprintf(stderr,"blocks allocated = %ld\n",blk_alloc);
	}
	if(blk_alloc == 0)
	{
		cntr_blk = FAIL;
		curr_blk[0] = 0;
		mktemp(tempnm);

/*	for systems older than 3.0	
		if(close( creat(tempnm,0777)) < 0) {
			err("Cannot create temporary name %s\n",tempnm);
		}
		if((tempfd = open(tempnm,O_RDWR)) < 0) {
			err("Cannot open temporary name %s\n",tempnm);
		}
 */

/*	For systems 3.0 and newer	*/
		if((tempfd = open(tempnm,O_RDWR|O_CREAT,0666)) < 0) {
			err("Cannot open temporary name %s\n",tempnm);
		}

		if((float_ptr[0] = malloc(blk_size)) == NULL){
			err("Cannot allocate space of paging block\n");
		}
		if((float_ptr[1] = malloc(blk_size)) == NULL){
			err("Cannot allocate space of paging block\n");
		}
	}
	else
	{
		if(putblock(float_ptr[1],curr_blk[1],blk_size) != curr_blk[1])
		{
			err("Cannot put blocks in temporary file");
		}
	}
	bp = float_ptr[1];
	for( i = 0; i < blk_size;i++){
		*bp++ = NULL;
	}

	offset = blk_alloc * blk_size;

	blk = howmany(size,blk_size) + blk_alloc;

	for ( next_blk = blk_alloc; next_blk < blk; next_blk++) {
		if(putblock(float_ptr[1],next_blk,blk_size) != next_blk) {
			err("Cannot put blocks in temporary file");
		}
	}
	curr_blk[1] = blk_alloc = blk;

	return(offset);
}

/*
 *	This funtion takes a offset in the file and returns a pointer
 *	to the offset.  If successful a pointer is returned,  if not
 *	NULL is returned.
 */

char *
getpwoff(offset)
off_t	offset;
{
	extern  int blk_size;
	extern	char *getblock();
	daddr_t	blk;
	char	*ptr;
	

/*
 *	Calculate block number
 */

	blk = offset / blk_size;

/*
 *	Get the block
 */

	if((ptr = getblock(blk)) == NULL)
	{
		return(NULL);
	}

	ptr += offset - (blk_size * blk);
	return(ptr);
}

/*
 *	This routine allocates and keeps track of the queue pages.
 *	If the page is unallocated the space for the page is allocated
 *	and the page is read.  Normally htere are two pages in memory,
 *	the control page, and the floating page.  When a page is not in
 *	memory and the block is in the floating page, the page is put
 *	back on disk and the new page is brought into the floating
 *	page.  The control page once allocated and brought in to memory
 *	doesn't leave memory.  Once the block is allocated and brought
 *	in a pointer to the block is passed to the calling routine.
 */

char	*cntr_ptr = NULL;
char	*float_ptr[2] = { NULL,NULL };

long	curr_blk[2];
long	cntr_blk = 0;

char *
getblock(blk_no)
long blk_no;
{
	extern 	long putblock();	/* put a block on disk		*/
	extern  long lseek();		/* disk seek routine		*/
	extern 	char *malloc();		/* memory allocation routine	*/
	extern	int blk_size;		/* page size			*/
	extern	int errno;
	daddr_t	offset;			/* calculated offset in file	*/
	char	**ptr;			/* pointer to buffer pointer	*/
	long	*bptr;			/* pointer to block number	*/
	long	dist0;
	long	dist1;
	int	index;
	int 	pflag;			/* put block flag		*/
	int	rflag;			/* read block flag		*/

	pflag = rflag = 0;
	if(blk_no == cntr_blk)
	{
		ptr = &cntr_ptr;
		bptr = &cntr_blk;
	}
	else
	{
		if(blk_no != curr_blk[0] && blk_no != curr_blk[1])
		{
			pflag++;
			rflag++;
		}

		dist0 = (blk_no >= curr_blk[0] ? blk_no - curr_blk[0]
			: curr_blk[0] - blk_no);

		dist1 = (blk_no >= curr_blk[1] ? blk_no - curr_blk[1]
			: curr_blk[1] - blk_no);

		index = (dist0 > dist1 ? 1 : 0);

		ptr = &float_ptr[index];
		bptr = &curr_blk[index];
	}

	if(*ptr == NULL)
	{
		if(( *ptr = malloc((unsigned)blk_size) ) == NULL)
			return(NULL);
		pflag = 0;
	}

	if(pflag)
	{
		if(*bptr != -1)
		{
			if(putblock(*ptr,*bptr,blk_size) != *bptr)
			{
				return(NULL);
			}
		}
	}

	if(rflag)
	{
		offset = blk_size * blk_no;

		if( zflg > 1)
		{
			fprintf(stderr,
			"getblock: offset = %ld size = %d, blk_no = %ld\n",
			offset, blk_size, blk_no);
		}

		if(( lseek(tempfd,offset,0) == offset ) &&
		   ( read(tempfd,*ptr,blk_size) == blk_size ))
		{
			*bptr = blk_no;
			return(*ptr);
		}
		return(NULL);
	}
	return(*ptr);
}

/*
 *	This routine puts the designated block back on the disk
 */

long
putblock(ptr,block,size)
char *ptr;
long block;
int size;
{
	extern long	lseek();	/* disk seek function		*/
	daddr_t		offset;		/* calculated offset		*/

	if(zflg > 1)
		printf("putblock(%o,%ld,%d)\n",ptr,block,size);

	offset = block * size;

	if(( lseek(tempfd,offset,0) == offset ) &&
	   ( write(tempfd,ptr,size) == size ))
	{
		return(block);
	}
	return(FAIL);
}
