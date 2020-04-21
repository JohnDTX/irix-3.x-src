# include <sys/param.h>
# include <sys/fs.h>
# include <sys/inode.h>
# include <sys/dir.h>
# include <sys/stat.h>
# include <sys/reboot.h>

# include <signal.h>

# define BITMAPB	EFS_BITMAPBB
# define BITMAPOFF	EFS_BITMAPBOFF
# define ITOD(fs, i)	EFS_ITOBB(fs, i)
# define ITOO(fs, i)	EFS_ITOO(fs, i)
# define CGIMIN(fs, cg)	EFS_CGIMIN(fs, cg)

# define BITMAPWORDSHIFT	3
# define BITMAPWORDMASK		((1<<BITMAPWORDSHIFT)-1)

# define BITMAPWORD(b,i)	((char *)(b))[(i)>>BITMAPWORDSHIFT]
# define BITMAPBIT(b,i)		(1<<((i)&BITMAPWORDMASK))

# define GETBIT(b,i) (BITMAPWORD(b,i) & BITMAPBIT(b,i))
# define CLRBIT(b,i) (BITMAPWORD(b,i) &= ~BITMAPBIT(b,i))
# define SETBIT(b,i) (BITMAPWORD(b,i) |= BITMAPBIT(b,i))

# include <stdio.h>
# include <ctype.h>

# ifndef BSHIFT
# define BSHIFT	(9-1+FsTYPE)
# endif  BSHIFT

# ifndef BMASK
# define BMASK	(BSIZE-1)
# endif  BMASK

# ifndef BSIZE
# define BSIZE (1<<BSHIFT)
# endif  BSIZE

# ifndef INOPB
# define INOPB (BSIZE/sizeof (struct efs_dinode))
# endif  INOPB

# ifndef NINDIR
# define NINDIR (BSIZE/sizeof (daddr_t))
# endif  NINDIR

# define di_nx			di_numextents
# define di_x			di_u.di_extents
# define RIDICULOUSEXTS		4096
# define EXTSPERDINODE		EFS_DIRECTEXTENTS
# define EFS_EXTENT_MAGIC	0
# define EXTSPERBB		(BBSIZE/sizeof (struct extent))

typedef struct efs_dinode	DINODE;
typedef struct direct		DIRECT;

struct bufarea {
	struct bufarea	*b_next;		/* must be first */
	daddr_t	b_bno;
	union {
		char	b_buf[BSIZE];		/* buffer space */
		short	b_lnks[1];		/* link counts */
		struct efs b_fs;		/* super block */
		struct efs_dinode b_dinode[1];	/* inode block */
		struct extent b_ext[1];		/* extent */
		DIRECT b_dir[1];		/* directory */
	} b_un;
	char	b_dirty;
};
typedef struct bufarea BUFAREA;

typedef unsigned MEMSIZE;

struct bitmap {
	char *incore;
	daddr_t swap;
	daddr_t mapsize;
	char *id;
};

struct bitmap blk_map;
struct bitmap free_map;

# define blkmap		blk_map.incore
# define bmapblk	blk_map.swap
# define freemap	free_map.incore
# define fmapblk	free_map.swap

# define BMSET		0
# define BMGET		1
# define BMCLR		2

# define NDIRECT	(BSIZE/sizeof(struct direct))
# define SPERB		(BSIZE/sizeof(short))

# define NO		0
# define YES		1

# define MAXDUP		10		/* limit on dup blks (per inode) */
# define MAXBAD		10		/* limit on bad blks (per inode) */

# define STEPSIZE	7		/* default step for freelist spacing */
# define CYLSIZE	400		/* default cyl size for spacing */
# define MAXCYL		1000		/* maximum cylinder size */

# define BYTESHIFT	3		/* log2(BITSPERBYTE) */
# define BYTEMASK	07		/* BITSPERBYTE-1 */
# define LSTATE		2		/* bits per inode state */
# define STATEPB (BITSPERBYTE/LSTATE)	/* inode states per byte */
# define USTATE		0		/* inode not allocated */
# define FSTATE		01		/* inode is file */
# define DSTATE		02		/* inode is directory */
# define CLEAR		03		/* inode is to be cleared */
# define EMPT		32		/* empty directory? */
# define SMASK		03		/* mask for inode state */

# define sfiletype(sp)	((sp)->st_mode & S_IFMT)
# define dfiletype(dp)	((dp)->di_mode & IFMT)
# define ALLOC		(dfiletype(dp) != 0)
# define REG		(dfiletype(dp) == S_IFREG)
# define DIR		(dfiletype(dp) == S_IFDIR)
# define LNK		(dfiletype(dp) == S_IFLNK)
# define BLK		(dfiletype(dp) == S_IFBLK)
# define CHR		(dfiletype(dp) == S_IFCHR)
# define FIFO		(dfiletype(dp) == S_IFIFO)
# define SPECIAL 	(BLK || CHR)
# define ftypeok(dp)	(REG||DIR||LNK||FIFO||BLK||CHR)

# define MAXPATH	1500		/*
					 * max size for pathname string.
					 * Increase and recompile if pathname
					 * overflows.
					 */

# define NINOBLK	11		/* num blks for raw reading */
# define MAXRAW		110		/* largest raw read (in blks) */


# define NCHKBLK	32		/* -b # # # block checking */
daddr_t	chkblks[NCHKBLK];
int	chkblki;

daddr_t	startib;			/* blk num of first in raw area */
unsigned niblk;				/* num of blks in raw area */

BUFAREA	inoblk;				/* inode blocks */
BUFAREA	fileblk;			/* other blks in filesys */
BUFAREA	sblk;				/* file system superblock */
BUFAREA	*poolhead;			/* ptr to first buffer in pool */

#define initbarea(x)	(x)->b_dirty = 0;(x)->b_bno = (daddr_t)-1
#define dirty(x)	(x)->b_dirty = 1
#define inodirty()	inoblk.b_dirty = 1
#define fbdirty()	fileblk.b_dirty = 1
#define sbdirty()	sblk.b_dirty = 1

#define dirblk		fileblk.b_un.b_dir
#define superblk	sblk.b_un.b_fs

struct filecntl {
	int	rfdes;
	int	wfdes;
	int	mod;
};

struct filecntl	dfile;			/* file descriptors for filesys */
struct filecntl	sfile;			/* file descriptors for scratch file */

MEMSIZE	memsize;			/* amt of memory we got */

#ifdef m68000
#define MAXDATA ((MEMSIZE)100*1024)
#endif
#ifdef	sgi
#undef	MAXDATA
#define MAXDATA ((MEMSIZE)400*1024)
#endif

#define	DUPTBLSIZE	100		/* num of dup blocks to remember */
daddr_t	duplist[DUPTBLSIZE];		/* dup block table */
daddr_t	*enddup;			/* next entry in dup table */
daddr_t	*muldup;			/* multiple dups part of table */

#define MAXLNCNT	20		/* num zero link cnts to remember */
ino_t	badlncnt[MAXLNCNT];		/* table of inos with zero link cnts */
ino_t	*badlnp;			/* next entry in table */

char	tflag;				/* scratch file specified */
char	qflag;				/* less verbose flag */
char	sflag;				/* salvage freeblocks */
char	csflag;				/* salvage freeblocks (conditional) */
char	nflag;				/* assume a no response */
char	yflag;				/* assume a yes response */
char	rplyflag;			/* any questions asked? */
char	Dirc;				/* extensive directory check */
char	fast;				/* fast check- dups and freelist */
char	hotroot;			/* checking root device */
char	rawflg;				/* read raw device */
char	rmscr;				/* remove scratch file when done */
char	fixfree;			/* corrupted free list */
char	*membase;			/* base of memory we get */
char	*statemap;			/* ptr to inode state table */
char	*pathp;				/* pointer to pathname position */
char	*thisname;			/* ptr to current pathname component */
char	*srchname;			/* name being searched for in dir */
char	pss2done;			/* do not check dir blks anymore */
char	pathname[MAXPATH];
char	devname[25];
char	initdone;
char	scrfile[80];

short	*lncntp;			/* ptr to link count table */
	
int	cylsize;			/* num blocks per cylinder */
int	stepsize;			/* num blocks for spacing purposes */
int	badblk;				/* num of bad blks seen (per inode) */
int	dupblk;				/* num of dup blks seen (per inode) */
int	(*pfunc)();			/* function to call to chk blk */

ino_t	inum;				/* inode we are currently working on */
ino_t	max_inodes;			/* number of inodes */
ino_t	parentdir;			/* i number of parent directory */
ino_t	lastino;			/* hiwater mark of inodes */
ino_t	lfdir;				/* lost & found directory */
ino_t	orphan;				/* orphaned inode */

off_t	filsize;			/* num blks seen in file */
off_t	bmapsz;				/* num chars in blkmap */
daddr_t	data_blocks;			/* num data blocks in fs */
daddr_t	inode_blocks;			/* num inode blocks in fs */
daddr_t bitmap_blocks;			/* num bitmap blocks in fs */

daddr_t	smapblk;			/* starting blk of state map */
daddr_t	lncntblk;			/* starting blk of link cnt table */
daddr_t	n_free;				/* number of free blocks */
daddr_t	n_blks;				/* number of blocks used */
daddr_t	n_files;			/* number of files seen */
daddr_t	fmin;				/* number of lowest valid data block */
daddr_t	fmax;				/* number of blocks in the volume */

#define minsz(x,y)	(x>y ? y : x)
#define howmany(x,y)	(((x)+((y)-1))/(y))
#define roundup(x,y)	((((x)+((y)-1))/(y))*(y))
#define outrange(x)	(x < fmin || x >= fmax)
#define zapino(x)	clear((x),sizeof(DINODE))

#define setlncnt(x)	dolncnt(x,0)
#define getlncnt()	dolncnt(0,1)
#define declncnt()	dolncnt(0,2)

#define setbmap(x)	maphack(&blk_map,x,BMSET)
#define getbmap(x)	maphack(&blk_map,x,BMGET)
#define clrbmap(x)	maphack(&blk_map,x,BMCLR)

#define setfmap(x)	maphack(&free_map,x,BMSET)
#define getfmap(x)	maphack(&free_map,x,BMGET)
#define clrfmap(x)	maphack(&free_map,x,BMCLR)

#define setstate(x)	dostate(x,0)
#define getstate()	dostate(0,1)

#define ADDR	0	/* check arg: check addresses */
#define DATA	1	/* check arg: do dirscan */
#define BBLK	2	/* check arg: check blocks */
#define DIRSCAN	DATA
#define DEMPT	3	/* check arg: check dir empty */

#define ALTERD	010	/* return flag: changed */
#define KEEPON	004	/* return flag: keep checking */
#define SKIP	002	/* return flag: skip */
#define STOP	001	/* return flag: stop */
#define REM	007	/* return flag: remove */

extern DINODE	*ginode();
extern BUFAREA	*getblk();
extern BUFAREA	*tgetblk();
extern BUFAREA	*search();
extern int	dirscan();
extern int	chkblk();
extern int	chkeblk();
extern int	findino();
extern int	catch();
extern int	mkentry();
extern int	chgdd();

char	id;
dev_t	pipedev;	/*
			 * is pipedev (and != -1) iff the standard input
			 * is a pipe so never check it.
			 */

# define copy(s, t, n)		blt(t, s, n)
# define clear(t, n)		bzero(t, n)
