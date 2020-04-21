char _Origin_[] = "System V";
/* @(#)fsck.c	1.16 */
/* $Source: /d2/3.7/src/etc/fsck/RCS/fsck.c,v $ */
static	char	*Sccsid = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 15:39:29 $ */

#ifdef lint
#undef	FsTYPE
#define FsTYPE 2
#else lint
#if FsTYPE!=1 && FsTYPE!=2
INVALID FsTYPE
#endif lint

#include <sys/types.h>
#include <sys/param.h>
#include <sys/inode.h>
#ifndef STANDALONE
#include <signal.h>
#endif STANDALONE
#include <sys/sysmacros.h>
#include <sys/filsys.h>
#include <sys/dir.h>
#include <sys/fblk.h>
/*
#include <sys/ino.h>
 */
#ifndef STANDALONE
#include <sys/stat.h>
#include <sys/reboot.h>
#include <stdio.h>
#include <ctype.h>
#include <mntent.h>
#endif STANDALONE

#define	NCHKBLK	32		/* -b # # # block checking */
daddr_t	chkblks[NCHKBLK];
int	chkblki = 0;

# ifndef BSHIFT
# define BSHIFT		(9-1+FsTYPE)
# endif  BSHIFT

# ifndef BMASK
# define BMASK		(BSIZE-1)
# endif  BMASK

# ifndef BSIZE
# define BSIZE		(1<<BSHIFT)
# endif  BSIZE

# ifndef INOPB
# define INOPB		(BSIZE/sizeof (struct dinode))
# endif  INOPB

# ifndef NINDIR
# define NINDIR		(BSIZE/sizeof (daddr_t))
# endif  NINDIR

# ifndef ROOTINO
# define ROOTINO	((ino_t)2)
# endif  ROOTINO

# ifndef SUPERB
# define SUPERB		((daddr_t)1)
# endif  SUPERB

# ifndef SUPERBOFF
# define SUPERBOFF	((off_t)SBSIZE)
# endif  SUPERBOFF

# ifndef itod
# define itod(x)	FsITOD(-1, x)
# endif  itod

# ifndef itoo
# define itoo(x)	FsITOO(-1, x)
# endif  itoo

#define	SBSIZE	512

#define NDIRECT	(BSIZE/sizeof(struct direct))
#define SPERB	(BSIZE/sizeof(short))

#define NO	0
#define YES	1

#define	MAXDUP	10		/* limit on dup blks (per inode) */
#define	MAXBAD	10		/* limit on bad blks (per inode) */

#define STEPSIZE	7	/* default step for freelist spacing */
#define CYLSIZE		400	/* default cyl size for spacing */
#define MAXCYL		1000	/* maximum cylinder size */

#define BITSPB	8		/* number bits per byte */
#define BITSHIFT	3	/* log2(BITSPB) */
#define BITMASK	07		/* BITSPB-1 */
#define LSTATE	2		/* bits per inode state */
#define STATEPB	(BITSPB/LSTATE)	/* inode states per byte */
#define USTATE	0		/* inode not allocated */
#define FSTATE	01		/* inode is file */
#define DSTATE	02		/* inode is directory */
#define CLEAR	03		/* inode is to be cleared */
#define EMPT	32		/* empty directory? */
#define SMASK	03		/* mask for inode state */

typedef struct dinode	DINODE;
typedef struct direct	DIRECT;

#define ALLOC	((dp->di_mode & IFMT) != 0)
#define DIR	((dp->di_mode & IFMT) == S_IFDIR)
#define REG	((dp->di_mode & IFMT) == S_IFREG)
#define BLK	((dp->di_mode & IFMT) == S_IFBLK)
#define CHR	((dp->di_mode & IFMT) == S_IFCHR)
#define FIFO	((dp->di_mode & IFMT) == S_IFIFO)
#ifdef	S_IFLNK
#define LNK	((dp->di_mode & IFMT) == S_IFLNK)
#else	S_IFLNK
#define LNK	0
#endif	S_IFLNK
#define SPECIAL (BLK || CHR)

#define MAXPATH	1500		/* max size for pathname string.
				 * Increase and recompile if pathname
				 * overflows.
				 */

#ifndef STANDALONE
#define NINOBLK	11		/* num blks for raw reading */
#define MAXRAW	110		/* largest raw read (in blks) */
daddr_t	startib;		/* blk num of first in raw area */
unsigned niblk;			/* num of blks in raw area */
#endif STANDALONE

struct bufarea {
	struct bufarea	*b_next;		/* must be first */
	daddr_t	b_bno;
	union {
		char	b_buf[BSIZE];		/* buffer space */
		short	b_lnks[SPERB];		/* link counts */
		daddr_t	b_indir[NINDIR];	/* indirect block */
		struct filsys b_fs;		/* super block */
		struct fblk b_fb;		/* free block */
		struct dinode b_dinode[INOPB];	/* inode block */
		DIRECT b_dir[NDIRECT];		/* directory */
	} b_un;
	char	b_dirty;
};

typedef struct bufarea BUFAREA;

BUFAREA	inoblk;			/* inode blocks */
BUFAREA	fileblk;		/* other blks in filesys */
BUFAREA	sblk;			/* file system superblock */
#define ftypeok(dp)	(REG||DIR||BLK||CHR||FIFO||LNK)
BUFAREA	*poolhead;		/* ptr to first buffer in pool */

#define initbarea(x)	(x)->b_dirty = 0;(x)->b_bno = (daddr_t)-1
#define dirty(x)	(x)->b_dirty = 1
#define inodirty()	inoblk.b_dirty = 1
#define fbdirty()	fileblk.b_dirty = 1
#define sbdirty()	sblk.b_dirty = 1

#define freeblk		fileblk.b_un.b_fb
#define dirblk		fileblk.b_un.b_dir
#define superblk	sblk.b_un.b_fs

struct filecntl {
	int	rfdes;
	int	wfdes;
	int	mod;
};

struct filecntl	dfile;		/* file descriptors for filesys */
struct filecntl	sfile;		/* file descriptors for scratch file */

typedef unsigned MEMSIZE;

MEMSIZE	memsize;		/* amt of memory we got */
#ifdef m68000
#define MAXDATA ((MEMSIZE)100*1024)
#endif
#ifdef	sgi
#undef	MAXDATA
#define MAXDATA ((MEMSIZE)400*1024)
#endif
#if STANDALONE && m68000
char memory[100*1024];
#endif STANDALONE

#define	DUPTBLSIZE	100	/* num of dup blocks to remember */
daddr_t	duplist[DUPTBLSIZE];	/* dup block table */
daddr_t	*enddup;		/* next entry in dup table */
daddr_t	*muldup;		/* multiple dups part of table */

#define MAXLNCNT	20	/* num zero link cnts to remember */
ino_t	badlncnt[MAXLNCNT];	/* table of inos with zero link cnts */
ino_t	*badlnp;		/* next entry in table */

#ifdef STANDALONE
char line[100];
#else STANDALONE
char	tflag;			/* scratch file specified */
#endif STANDALONE
char	qflag;			/* less verbose flag */
char	sflag;			/* salvage free block list */
char	csflag;			/* salvage free block list (conditional) */
char	nflag;			/* assume a no response */
char	yflag;			/* assume a yes response */
char	rplyflag;		/* any questions asked? */
char	Dirc;			/* extensive directory check */
char	fast;			/* fast check- dup blks and free list check */
#ifndef STANDALONE
char	hotroot;		/* checking root device */
char	rawflg;			/* read raw device */
char	rmscr;			/* remove scratch file when done */
#endif STANDALONE
char	fixfree;		/* corrupted free list */
char	*membase;		/* base of memory we get */
char	*blkmap;		/* ptr to primary blk allocation map */
char	*freemap;		/* ptr to secondary blk allocation map */
char	*statemap;		/* ptr to inode state table */
char	*pathp;			/* pointer to pathname position */
char	*thisname;		/* ptr to current pathname component */
char	*srchname;		/* name being searched for in dir */
char	pss2done;			/* do not check dir blks anymore */
char	pathname[MAXPATH];
char	devname[25];
char	*lfname =	"lost+found";
#ifndef STANDALONE
char	initdone;
char	scrfile[80];
#endif STANDALONE

short	*lncntp;		/* ptr to link count table */

int	cylsize;		/* num blocks per cylinder */
int	stepsize;		/* num blocks for spacing purposes */
int	badblk;			/* num of bad blks seen (per inode) */
int	dupblk;			/* num of dup blks seen (per inode) */
int	(*pfunc)();		/* function to call to chk blk */

ino_t	inum;			/* inode we are currently working on */
ino_t	max_inodes;		/* number of inodes */
ino_t	parentdir;		/* i number of parent directory */
ino_t	lastino;		/* hiwater mark of inodes */
ino_t	lfdir;			/* lost & found directory */
ino_t	orphan;			/* orphaned inode */

off_t	filsize;		/* num blks seen in file */
off_t	bmapsz;			/* num chars in blkmap */

daddr_t	smapblk;		/* starting blk of state map */
daddr_t	lncntblk;		/* starting blk of link cnt table */
daddr_t	fmapblk;		/* starting blk of free map */
daddr_t	n_free;			/* number of free blocks */
daddr_t	n_blks;			/* number of blocks used */
daddr_t	n_files;		/* number of files seen */
daddr_t	fmin;			/* block number of the first data block */
daddr_t	fmax;			/* number of blocks in the volume */

#define minsz(x,y)	(x>y ? y : x)
#define howmany(x,y)	(((x)+((y)-1))/(y))
#define roundup(x,y)	((((x)+((y)-1))/(y))*(y))
#define outrange(x)	(x < fmin || x >= fmax)
#define zapino(x)	clear((x),sizeof(DINODE))

#define setlncnt(x)	dolncnt(x,0)
#define getlncnt()	dolncnt(0,1)
#define declncnt()	dolncnt(0,2)

#define setbmap(x)	domap(x,0)
#define getbmap(x)	domap(x,1)
#define clrbmap(x)	domap(x,2)

#define setfmap(x)	domap(x,0+4)
#define getfmap(x)	domap(x,1+4)
#define clrfmap(x)	domap(x,2+4)

#define setstate(x)	dostate(x,0)
#define getstate()	dostate(0,1)

#define DATA	1
#define ADDR	0
#define BBLK	2
#define ALTERD	010
#define KEEPON	04
#define SKIP	02
#define STOP	01
#define REM	07

DINODE	*ginode();
BUFAREA	*getblk();
BUFAREA	*search();
int	dirscan();
int	chkblk();
int	findino();
#ifndef STANDALONE
int	catch();
#endif STANDALONE
int	mkentry();
int	chgdd();
int	pass1();
int	pass1b();
int	pass2();
int	pass3();
int	pass4();
int	pass5();
char	id = ' ';
#ifndef STANDALONE
dev_t	pipedev = -1;	/* is pipedev (and != -1) iff the standard input
			 * is a pipe, which means we can't check pipedev! */

main(argc,argv)
int	argc;
char	*argv[];
#else STANDALONE
main()
#endif STANDALONE
{
	char filename[50];
#ifndef STANDALONE
	register i;
	register int n;
	int svargc, argvix;
	int ix;
	char **argx;
	struct stat statbuf;
	auto int fsmagic, fsbshift;
	auto char *checker;

	if ( argv[0][0] >= '0' && argv[0][0] <= '9' ) id = argv[0][0];
	setbuf(stdin,NULL);
	setbuf(stdout,NULL);
	sync();
	svargc = argc;
	for(i = 1; --argc < 0, argv[i] && *argv[i] == '-'; i++) {
		switch(argv[i][1]) {
			case 'b':
				while(argc && isdigit(argv[i+1][0])) {
					chkblks[chkblki++] = atoi(argv[++i]);
					--argc;
				}
				break;
			case 't':
			case 'T':
				tflag++;
				if(*argv[++i] == '-' || --argc <= 0)
					errexit("%c Bad -t option\n",id);
				strcpy(scrfile,argv[i]);
				if( (stat(scrfile,&statbuf) >= 0) &&
					((statbuf.st_mode & S_IFMT) != S_IFREG) )
					errexit("%c Illegal scratch file <%s>\n",
						id,scrfile);
				break;
			case 's':	/* salvage flag */
				stype(argv[i]+2);
				sflag++;
				break;
			case 'S':	/* conditional salvage */
				stype(argv[i]+2);
				csflag++;
				break;
			case 'n':	/* default no answer flag */
			case 'N':
				nflag++;
				yflag = 0;
				break;
			case 'y':	/* default yes answer flag */
			case 'Y':
				yflag++;
				nflag = 0;
				break;
			case 'q':
				qflag++;
				break;
			case 'D':
				Dirc++;
				break;
			case 'F':
			case 'f':
				fast++;
				break;
			default:
				errexit("%c %c option?\n",id,*(argv[i]+1));
		}
	}
	if(nflag && sflag)
		errexit("%c Incompatible options: -n and -s\n",id);
	if(nflag && qflag)
		errexit("%c Incompatible options: -n and -q\n",id);
#else STANDALONE
loop:
	finit();
	cylsize = stepsize = 0;		/* normally set in stype */
	sflag = csflag = nflag = yflag = Dirc = fast = 0;
	printf("\n\nStandalone fsck\n\n");
	printf("-s? (unconditionally rebuild free list)  ");
	gets(line);
	if (line[0] == 'y') {
		sflag++;
		printf("blocks-per-cyl:blocks-to-skip (optional)  ");
		gets(line);
		if (line[0] != '\0')
			stype(line);
	}
	printf("-S? (rebuild free list, but only if no discrepancies)  ");
	gets(line);
	if (line[0] == 'y') {
		csflag++;
		printf("blocks-per-cyl:blocks-to-skip (optional)  ");
		gets(line);
		if (line[0] != '\0')
			stype(line);
		printf("'no' response will be assumed to all questions\n");
	}
	if (!sflag && !csflag) {
		printf("-n? (do not open for writing)  ");
		gets(line);
		if (line[0] == 'y') {
			nflag++;
			yflag = 0;
		}
	}
	printf("-y?  ");
	gets(line);
	if (line[0] == 'y') {
		yflag++;
		nflag = 0;
	}
	printf("-D?  ");
	gets(line);
	if (line[0] == 'y')
		Dirc++;
	printf("-f?  ");
	gets(line);
	if (line[0] == 'y')
		fast++;
#endif STANDALONE
	if(sflag && csflag)
		sflag = 0;
	if(csflag) nflag++;

#ifndef STANDALONE
	if (argc == 0) {	/* use fstab as default checklist */
		register FILE *fstabp;
		register struct mntent *mntp;

		if ((fstabp = setmntent(MNTTAB,"r")) == NULL) {
			errexit("Can't open checklist file: %s\n", MNTTAB);
		}
		n = 0;
		while ((mntp = getmntent(fstabp)) != NULL) {
			if (!strcmp(mntp->mnt_type, MNTTYPE_BELL))
				n++;
		}
		argx = (char **)calloc(svargc + n + 1, sizeof *argx);
		if (argx == NULL) {
			errexit("Out of memory\n");
		}
		for (n = 0; n < svargc; n++) {
			argx[n] = argv[n];
		}
		rewind(fstabp);
		while ((mntp = getmntent(fstabp)) != NULL) {
			char *name, *strchr();

			if (hasmntopt(mntp, MNTOPT_NOFSCK) != NULL
			    || strcmp(mntp->mnt_type, MNTTYPE_BELL)) {
				continue;
			}
			/*
			 * If the filesystem is not mounted on /, then
			 * we prefer to check the raw device.
			 */
			if (strcmp(mntp->mnt_dir, "/")
			    && (name = hasmntopt(mntp, MNTOPT_RAW)) != NULL
			    && (name = strchr(name, '=')) != NULL) {
				name++;
			} else {
				name = mntp->mnt_fsname;
			}
			argx[n] = (char *)malloc(strlen(name) + 1);
			(void) strcpy(argx[n], name);
			argc++, n++;
		}
		argx[n] = NULL;
		svargc = n;
		argv = argx;
		endmntent(fstabp);
	}
	ix = argvix = svargc - argc;		/* position of first fs argument */
	while(argc > 0) {
		initbarea(&sblk);
		if(checksb(argv[ix]) == NO) {
			argc--; ix++;
			continue;
		}
		if (get_superblk_type(sblk.b_un.b_buf,
				&fsmagic, &fsbshift) == 0
		 && !(fsmagic == FsMAGIC && fsbshift == BSHIFT)
		 && get_fsck(fsmagic, fsbshift, &checker) == 0)
		{
			close(dfile.rfdes);
			if(argvix < svargc - argc) {
				for(n = 0; n < argc; n++)
					argv[argvix + n] = argv[svargc - argc + n];
				argv[argvix + n] = NULL;
			}
			if(execvp(checker,argv) == -1)
				errexit("%c %sCannot exec %s\n",
					id,devname,checker);
		}
		if(!initdone) {
			initmem();
			initdone++;
		}
		check(argv[ix++]);
		argc--;
	}
	exit(0);
#else STANDALONE
#define	getfs(f)	(printf("\nfilesystem:  "), gets(f))
	memsize = MAXDATA;
	membase = memory;
	for (getfs(filename); filename[0] != '\0'; getfs(filename)) {
		initbarea(&sblk);
		if(checksb(filename) == NO)
			continue;
#if FsTYPE==2
		if(superblk.s_magic != FsMAGIC ||
		(superblk.s_magic == FsMAGIC && superblk.s_type == Fs1b)) {
			error("%c %s not a 1024-byte file system\n",
				id,filename);
			continue;
		}
#else
		if(superblk.s_magic == FsMAGIC && superblk.s_type == Fs2b) {
			error("%c %s not a 512-byte file system\n",
				id,filename);
			continue;
		}
#endif
		check(filename);
	}
	goto loop;
#endif STANDALONE
}


/* VARARGS */
error(s1,s2,s3,s4)
char *s1, *s2, *s3, *s4;
{
	printf(s1,s2,s3,s4);
}


errexit(s1,s2,s3,s4)
char *s1, *s2, *s3, *s4;
{
	error(s1,s2,s3,s4);
#ifndef STANDALONE
	exit(8);
#else STANDALONE
	exit();
#endif STANDALONE
}

#ifndef STANDALONE
initmem()
{
	register n;
	struct stat statbuf;
	int (*sg)();
	char *sbrk();

	/* memsize = (MEMSIZE)sbrk(sizeof(int)); */
	/* memsize = MAXDATA - memsize - sizeof(int); */
	memsize = MAXDATA;
	while(memsize >= 2*sizeof(BUFAREA) &&
		(membase = sbrk(memsize)) == (char *)-1)
		memsize -= 1024;
	if(memsize < 2*sizeof(BUFAREA))
		errexit("%c Can't get memory\n",id);
	for(n = 1; n < NSIG; n++) {
		if(n == SIGCLD || n == SIGPWR)
			continue;
		sg = signal(n,catch);
		if(sg != SIG_DFL)
			signal(n,sg);
	}
	/* Check if standard input is a pipe. If it is, record pipedev so
	 * we won't ever check it */
	if ( fstat( 0, &statbuf) == -1 )
		errexit("%c Can't fstat standard input\n", id);
	if ( (statbuf.st_mode & S_IFMT) == S_IFIFO ) pipedev = statbuf.st_dev;
}
#endif STANDALONE

check(dev)
char *dev;
{
	register DINODE *dp;
	register n;
	register ino_t *blp;
	ino_t savino;
	daddr_t blk;
	BUFAREA *bp1, *bp2;

#ifndef STANDALONE
	if(pipedev != -1) {
		strcpy(devname,dev);
		strcat(devname,"\t");
	}
	else
#endif STANDALONE
		devname[0] = '\0';
	if(setup(dev) == NO)
		return;


	printf("%c %s** Phase 1 - Check Blocks and Sizes\n",id,devname);
	pfunc = pass1;
	for(inum = 1; inum <= max_inodes; inum++) {
		if((dp = ginode()) == NULL)
			continue;
		if(ALLOC) {
			lastino = inum;
			if(!ftypeok(dp)) {
				printf("%c %sUNKNOWN FILE TYPE I=%u",id,devname,inum);
				if(dp->di_size)
					printf(" (NOT EMPTY)");
				if(reply("CLEAR") == YES) {
					zapino(dp);
					inodirty();
				}
				continue;
			}
			n_files++;
			if(setlncnt(dp->di_nlink) <= 0) {
				if(badlnp < &badlncnt[MAXLNCNT])
					*badlnp++ = inum;
				else {
					printf("%c %sLINK COUNT TABLE OVERFLOW",id,devname);
					if(reply("CONTINUE") == NO)
						errexit("");
				}
			}
			setstate(DIR ? DSTATE : FSTATE);
			badblk = dupblk = 0;
			filsize = 0;
			ckinode(dp,ADDR);
			if((n = getstate()) == DSTATE || n == FSTATE)
				sizechk(dp);
		}
		else if(dp->di_mode != 0) {
			printf("%c %sPARTIALLY ALLOCATED INODE I=%u",id,devname,inum);
			if(dp->di_size)
				printf(" (NOT EMPTY)");
			if(reply("CLEAR") == YES) {
				zapino(dp);
				inodirty();
			}
		}
	}


	if(enddup != &duplist[0]) {
		printf("%c %s** Phase 1b - Rescan For More DUPS\n",id,devname);
		pfunc = pass1b;
		for(inum = 1; inum <= lastino; inum++) {
			if(getstate() != USTATE && (dp = ginode()) != NULL)
				if(ckinode(dp,ADDR) & STOP)
					break;
		}
	}
#ifndef STANDALONE
	if(rawflg) {
		if(inoblk.b_dirty)
			bwrite(&dfile,membase,startib,niblk*BSIZE);
		inoblk.b_dirty = 0;
		if(poolhead) {
			clear(membase,niblk*BSIZE);
			for(bp1 = poolhead;bp1->b_next;bp1 = bp1->b_next);
			bp2 = &((BUFAREA *)membase)[(niblk*BSIZE)/sizeof(BUFAREA)];
			while(--bp2 >= (BUFAREA *)membase) {
				initbarea(bp2);
				bp2->b_next = bp1->b_next;
				bp1->b_next = bp2;
			}
		}
		rawflg = 0;

	}
#endif STANDALONE


if(!fast) {
	printf("%c %s** Phase 2 - Check Pathnames\n",id,devname);
	inum = ROOTINO;
	thisname = pathp = pathname;
	pfunc = pass2;
	switch(getstate()) {
		case USTATE:
			errexit("%c %sROOT INODE UNALLOCATED. TERMINATING.\n",id,devname);
		case FSTATE:
			printf("%c %sROOT INODE NOT DIRECTORY",id,devname);
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
			printf("%c %sDUPS/BAD IN ROOT INODE\n",id,devname);
			if(reply("CONTINUE") == NO)
				errexit("");
			setstate(DSTATE);
			descend();
	}


	pss2done++;
	printf("%c %s** Phase 3 - Check Connectivity\n",id,devname);
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


	printf("%c %s** Phase 4 - Check Reference Counts\n",id,devname);
	pfunc = pass4;
	for(inum = ROOTINO; inum <= lastino; inum++) {
		switch(getstate()) {
			case FSTATE:
				if(n = getlncnt())
					adjust((short)n);
				else {
					for(blp = badlncnt;blp < badlnp; blp++)
						if(*blp == inum) {
							if((dp = ginode()) &&
							dp->di_size) {
								if((n = linkup()) == NO)
								   clri("UNREF",NO);
								if (n == REM)
								   clri("UNREF",REM);
							}
							else
								clri("UNREF",YES);
							break;
						}
				}
				break;
			case DSTATE:
				clri("UNREF",YES);
				break;
			case CLEAR:
				clri("BAD/DUP",YES);
		}
	}
	if(max_inodes - n_files != superblk.s_tinode) {
		printf("%c %sFREE INODE COUNT WRONG IN SUPERBLK",id,devname);
		if (qflag) {
			superblk.s_tinode = max_inodes - n_files;
			sbdirty();
			printf("\n%c %sFIXED\n",id,devname);
		}
		else if(reply("FIX") == YES) {
			superblk.s_tinode = max_inodes - n_files;
			sbdirty();
		}
	}
	flush(&dfile,&fileblk);


}	/* if fast check, skip to phase 5 */
	printf("%c %s** Phase 5 - Check Free List ",id,devname);
	if(sflag || (csflag && rplyflag == 0)) {
		printf("(Ignored)\n");
		fixfree = 1;
	}
	else {
		printf("\n");
		if(freemap)
			copy(blkmap,freemap,(MEMSIZE)bmapsz);
		else {
			for(blk = 0; blk < fmapblk; blk++) {
				bp1 = getblk(NULL,blk);
				bp2 = getblk(NULL,blk+fmapblk);
				copy(bp1->b_un.b_buf,bp2->b_un.b_buf,BSIZE);
				dirty(bp2);
			}
		}
		badblk = dupblk = 0;
		freeblk.df_nfree = superblk.s_nfree;
		for(n = 0; n < NICFREE; n++)
			freeblk.df_free[n] = superblk.s_free[n];
		freechk();
		if(badblk)
			printf("%c %s%d BAD BLKS IN FREE LIST\n",id,devname,badblk);
		if(dupblk)
			printf("%c %s%d DUP BLKS IN FREE LIST\n",id,devname,dupblk);

		if(fixfree == 0) {
			if((n_blks+n_free) != (fmax-fmin)) {
#ifndef STANDALONE
				printf("%c %s%ld BLK(S) MISSING\n",id,devname,
					fmax-fmin-n_blks-n_free);
#else STANDALONE
				printf("%c %s%D BLK(S) MISSING\n",id,devname,
					fmax-fmin-n_blks-n_free);
#endif STANDALONE
				fixfree = 1;
			}
			else if(n_free != superblk.s_tfree) {
				printf("%c %sFREE BLK COUNT WRONG IN SUPERBLK",id,devname);
				if(qflag) {
					superblk.s_tfree = n_free;
					sbdirty();
					printf("\n%c %sFIXED\n",id,devname);
				}
				else if(reply("FIX") == YES) {
					superblk.s_tfree = n_free;
					sbdirty();
				}
			}
		}
		if(fixfree) {
			printf("%c %sBAD FREE LIST",id,devname);
			if(qflag && !sflag) {
				fixfree = 1;
				printf("\n%c %sSALVAGED\n",id,devname);
			}
			else if(reply("SALVAGE") == NO)
				fixfree = 0;
		}
	}

	if(fixfree) {
		printf("%c %s** Phase 6 - Salvage Free List\n",id,devname);
		makefree();
		n_free = superblk.s_tfree;
	}


#ifdef STANDALONE
	printf("%c %s%D files %D blocks %D free\n",id,devname,
#else STANDALONE
#if FsTYPE==2
	printf("%c %s%ld files %ld K used %ld K free\n", id, devname,
#else
	printf("%c %s%ld files %ld blocks %ld free\n",id,devname,
#endif
#endif STANDALONE
		n_files,n_blks,n_free);
#ifndef STANDALONE
	if(dfile.mod) {
		time(&superblk.s_time);
		sbdirty();
	}
#endif
	ckfini();
#ifndef STANDALONE
	sync();
	if(dfile.mod && hotroot) {
		printf("%c %s***** REBOOTING UNIX . . . *****\n",id,devname);
		sleep(2);
		reboot(RB_AUTOBOOT | RB_NOSYNC);
	}
#endif
	if(dfile.mod)
		printf("%c %s***** FILE SYSTEM WAS MODIFIED *****\n",id,devname);
}


ckinode(dp,flg)
register DINODE *dp;
register flg;
{
	register daddr_t *ap;
	register ret;
	int (*func)(), n;
	daddr_t	iaddrs[NADDR];

	if(SPECIAL)
		return(KEEPON);
	l3tol(iaddrs,dp->di_addr,NADDR);
	switch(flg) {
		case ADDR:
			func = pfunc;
			break;
		case DATA:
			func = dirscan;
			break;
		case BBLK:
			func = chkblk;
	}
	for(ap = iaddrs; ap < &iaddrs[NADDR-3]; ap++) {
		if(*ap && (ret = (*func)(*ap,((ap == &iaddrs[0]) ? 1 : 0))) & STOP)
			if(flg != BBLK)
				return(ret);
	}
	for(n = 1; n < 4; n++) {
		if(*ap && (ret = iblock(*ap,n,flg)) & STOP) {
			if(flg != BBLK)
				return(ret);
		}
		ap++;
	}
	return(KEEPON);
}

iblock(blk,ilevel,flg)
daddr_t blk;
register ilevel;
{
	register daddr_t *ap;
	register n;
	int (*func)();
	BUFAREA ib;

	if(flg == BBLK)		func = chkblk;
	else if(flg == ADDR) {
		func = pfunc;
		if(((n = (*func)(blk)) & KEEPON) == 0)
			return(n);
	}
	else
		func = dirscan;
	if(outrange(blk))		/* protect thyself */
		return(SKIP);
	initbarea(&ib);
	if(getblk(&ib,blk) == NULL)
		return(SKIP);
	ilevel--;
	for(ap = ib.b_un.b_indir; ap < &ib.b_un.b_indir[NINDIR]; ap++) {
		if(*ap) {
			if(ilevel > 0)
				n = iblock(*ap,ilevel,flg);
			else
				n = (*func)(*ap,0);
			if(n & STOP && flg != BBLK)
				return(n);
		}
	}
	return(KEEPON);
}

chkblk(blk,flg)
register daddr_t blk;
{
	register DIRECT *dirp;
	register char *ptr;
	int zerobyte, baddir = 0, dotcnt = 0;

	if(outrange(blk))
		return(SKIP);
	if(getblk(&fileblk, blk) == NULL)
		return(SKIP);
	for(dirp = dirblk; dirp <&dirblk[NDIRECT]; dirp++) {
		ptr = dirp->d_name;
		zerobyte = 0;
		while(ptr <&dirp->d_name[DIRSIZ]) {
			if(zerobyte && *ptr) {
				baddir++;
				break;
			}
			if(flg) {
				if(ptr == &dirp->d_name[0] && *ptr == '.' &&
					*(ptr + 1) == '\0') {
					dotcnt++;
					if(inum != dirp->d_ino) {
						printf("%c %sNO VALID '.' in DIR I = %u\n",
							id,devname,inum);
						baddir++;
					}
					break;
				}
				if(ptr == &dirp->d_name[0] && *ptr == '.' &&
					*(ptr + 1) == '.' && *(ptr + 2) == '\0') {
					dotcnt++;
					if(!dirp->d_ino) {
						printf("%c %sNO VALID '..' in DIR I = %u\n",
							id,devname,inum);
						baddir++;
					}
					break;
				}
			}
			if(*ptr == '/') {
				baddir++;
				break;
			}
			if(*ptr == NULL) {
				if(dirp->d_ino && ptr == &dirp->d_name[0]) {
					baddir++;
					break;
				}
				else
					zerobyte++;
			}
			ptr++;
		}
	}
	if(flg && dotcnt < 2) {
		printf("%c %sMISSING '.' or '..' in DIR I = %u\n",id,devname,inum);
#ifndef STANDALONE
		printf("%c %sBLK %ld ",id,devname,blk);
#else STANDALONE
		printf("%c %sBLK %D ",id,devname,blk);
#endif STANDALONE
		pinode();
		printf("\n%c %sDIR=%s\n\n",id,devname,pathname);
		return(YES);
	}
	if(baddir) {
		printf("%c %sBAD DIR ENTRY I = %u\n",id,devname,inum);
#ifndef STANDALONE
		printf("%c %sBLK %ld ",id,devname,blk);
#else STANDALONE
		printf("%c %sBLK %D ",id,devname,blk);
#endif STANDALONE
		pinode();
		printf("\n%c %sDIR=%s\n\n",id,devname,pathname);
		return(YES);
	}
	return(KEEPON);
}

pass1(blk)
register daddr_t blk;
{
	register i;
	register daddr_t *dlp;

	if(chkblki)
		for(i = 0; i < chkblki; i++)
			if(blk == chkblks[i])
				printf("%c %sBLK=%u in I=%u\n", id, devname,
					blk, inum);
	if(outrange(blk)) {
		blkerr("BAD",blk);
		if(++badblk >= MAXBAD) {
			printf("%c %sEXCESSIVE BAD BLKS I=%u",id,devname,inum);
			if(reply("CONTINUE") == NO)
				errexit("");
			return(STOP);
		}
		return(SKIP);
	}
	if(getbmap(blk)) {
		blkerr("DUP",blk);
		if(++dupblk >= MAXDUP) {
			printf("%c %sEXCESSIVE DUP BLKS I=%u",id,devname,inum);
			if(reply("CONTINUE") == NO)
				errexit("");
			return(STOP);
		}
		if(enddup >= &duplist[DUPTBLSIZE]) {
			printf("%c %sDUP TABLE OVERFLOW.",id,devname);
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
		n_blks++;
		setbmap(blk); 
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
			printf("%c %sDIR pathname too deep\n",id,devname);
			printf("%c %sIncrease MAXPATH and recompile.\n",
			id,devname);
			printf("%c %sDIR pathname is <%s>\n",
			id,devname,pathname);
			ckfini();
#ifndef STANDALONE
			exit(4);
#else STANDALONE
			exit();
#endif STANDALONE
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


pass5(blk)
register daddr_t blk;
{
	if(outrange(blk)) {
		fixfree = 1;
		if(++badblk >= MAXBAD) {
			printf("%c %sEXCESSIVE BAD BLKS IN FREE LIST.",id,devname);
			if(reply("CONTINUE") == NO)
				errexit("");
			return(STOP);
		}
		return(SKIP);
	}
	if(getfmap(blk)) {
		fixfree = 1;
		if(++dupblk >= DUPTBLSIZE) {
			printf("%c %sEXCESSIVE DUP BLKS IN FREE LIST.",id,devname);
			if(reply("CONTINUE") == NO)
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


blkerr(s,blk)
daddr_t blk;
char *s;
{
#ifndef STANDALONE
	printf("%c %s%ld %s I=%u\n",id,devname,blk,s,inum);
#else STANDALONE
	printf("%c %s%D %s I=%u\n",id,devname,blk,s,inum);
#endif STANDALONE
	setstate(CLEAR);	/* mark for possible clearing */
}


descend()
{
	register DINODE *dp;
	register char *savname;
	off_t savsize;

	setstate(FSTATE);
	if((dp = ginode()) == NULL)
		return;
	if(Dirc && !pss2done)
		ckinode(dp,BBLK);
	savname = thisname;
	*pathp++ = '/';
	savsize = filsize;
	filsize = dp->di_size;
	ckinode(dp,DATA);
	thisname = savname;
	*--pathp = 0;
	filsize = savsize;
}


dirscan(blk)
register daddr_t blk;
{
	register DIRECT *dirp;
	register char *p1, *p2;
	register n;
	DIRECT direntry;

	if(outrange(blk)) {
		filsize -= BSIZE;
		return(SKIP);
	}
	for(dirp = dirblk; dirp < &dirblk[NDIRECT] &&
		filsize > 0; dirp++, filsize -= sizeof(DIRECT)) {
		if(getblk(&fileblk,blk) == NULL) {
			filsize -= (&dirblk[NDIRECT]-dirp)*sizeof(DIRECT);
			return(SKIP);
		}
		p1 = &dirp->d_name[DIRSIZ];
		p2 = &direntry.d_name[DIRSIZ];
		while(p1 > (char *)dirp)
			*--p2 = *--p1;
		if((n = (*pfunc)(&direntry)) & ALTERD) {
			if(getblk(&fileblk,blk) != NULL) {
				p1 = &dirp->d_name[DIRSIZ];
				p2 = &direntry.d_name[DIRSIZ];
				while(p1 > (char *)dirp)
					*--p1 = *--p2;
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


direrr(s)
char *s;
{
	register DINODE *dp;
	int n;

	printf("%c %s%s ",id,devname,s);
	pinode();
	if((dp = ginode()) != NULL && ftypeok(dp)) {
		printf("\n%c %s%s=%s",id,devname,DIR?"DIR":"FILE",pathname);
		if(DIR) {
			if(dp->di_size > EMPT) {
				if((n = chkempt(dp)) == NO) {
					printf(" (NOT EMPTY)\n");
				}
				else if(n != SKIP) {
					printf(" (EMPTY)");
					if(!nflag) {
						printf(" -- REMOVED\n");
						sleep(2);
						return(YES);
					}
					else
						printf("\n");
				}
			}
			else {
				printf(" (EMPTY)");
				if(!nflag) {
					printf(" -- REMOVED\n");
					sleep(2);
					return(YES);
				}
				else
					printf("\n");
			}
		}
		else if(REG||LNK)
			if(!dp->di_size) {
				printf(" (EMPTY)");
				if(!nflag) {
					printf(" -- REMOVED\n");
					sleep(2);
					return(YES);
				}
				else
					printf("\n");
			}
	}
	else {
		printf("\n%c %sNAME=%s",id,devname,pathname);
		if (dp) {
			if(!dp->di_size) {
				printf(" (EMPTY)");
				if(!nflag) {
					printf(" -- REMOVED\n");
					sleep(2);
					return(YES);
				}
				else
					printf("\n");
			}
			else
				printf(" (NOT EMPTY)\n");
		} else {
			printf(" (NON-EXISTENT)");
			if(!nflag) {
				printf(" -- REMOVED\n");
				sleep(2);
				return(YES);
			}
			else
				printf("\n");
		}
	}
	return(reply("REMOVE"));
}


adjust(lcnt)
register short lcnt;
{
	register DINODE *dp;
	register n;

	if((dp = ginode()) == NULL)
		return;
	if(dp->di_nlink == lcnt) {
		if((n = linkup()) == NO)
			clri("UNREF",NO);
		if(n == REM)
			clri("UNREF",REM);
	}
	else {
		printf("%c %sLINK COUNT %s",id,devname,
			(lfdir==inum)?lfname:(DIR?"DIR":"FILE"));
		pinode();
		printf("\n%c %sCOUNT %d SHOULD BE %d",id,devname,
			dp->di_nlink,dp->di_nlink-lcnt);
		if(reply("ADJUST") == YES) {
			dp->di_nlink -= lcnt;
			inodirty();
		}
	}
}


clri(s,flg)
char *s;
{
	register DINODE *dp;
	int n;

	if((dp = ginode()) == NULL)
		return;
	if(flg == YES) {
		if(!FIFO || !qflag || nflag) {
			printf("%c %s%s %s",id,devname,s,DIR?"DIR":"FILE");
			pinode();
		}
		if(DIR) {
			if(dp->di_size > EMPT) {
				if((n = chkempt(dp)) == NO) {
					printf(" (NOT EMPTY)\n");
				}
				else if(n != SKIP) {
					printf(" (EMPTY)");
					if(!nflag) {
						printf(" -- REMOVED\n");
						clrinode(dp);
						return;
					}
					else
						printf("\n");
				}
			}
			else {
				printf(" (EMPTY)");
				if(!nflag) {
					printf(" -- REMOVED\n");
					clrinode(dp);
					return;
				}
				else
					printf("\n");
			}
		}
		if(REG||LNK)
			if(!dp->di_size) {
				printf(" (EMPTY)");
				if(!nflag) {
					printf(" -- REMOVED\n");
					clrinode(dp);
					return;
				}
				else
					printf("\n");
			}
			else
				printf(" (NOT EMPTY)\n");
		if (FIFO && !nflag) {
			if(!qflag)	printf(" -- CLEARED");
			printf("\n");
			clrinode(dp);
			return;
		}
	}
	if(flg == REM)	clrinode(dp);
	else if(reply("CLEAR") == YES)
		clrinode(dp);
}


clrinode(dp)		/* quietly clear inode */
register DINODE *dp;
{

	n_files--;
	pfunc = pass4;
	ckinode(dp,ADDR);
	zapino(dp);
	inodirty();
}

chkempt(dp)
register DINODE *dp;
{
	register daddr_t *ap;
	register DIRECT *dirp;
	daddr_t blk[NADDR];
	int size;

	size = minsz(dp->di_size, (NADDR - 3) * BSIZE);
	l3tol(blk,dp->di_addr,NADDR);
	for(ap = blk; ap < &blk[NADDR - 3], size > 0; ap++) {
		if(*ap) {
			if(outrange(*ap)) {
				printf("chkempt: blk %d out of range\n",*ap);
				return(SKIP);
			}
			if(getblk(&fileblk,*ap) == NULL) {
				printf("chkempt: Can't find blk %d\n",*ap);
				return(SKIP);
			}
			for(dirp=dirblk; dirp < &dirblk[NDIRECT],
			size > 0; dirp++) {
				if(dirp->d_name[0] == '.' &&
				(dirp->d_name[1] == '\0' || (
				dirp->d_name[1] == '.' &&
				dirp->d_name[2] == '\0'))) {
					size -= sizeof(DIRECT);
					continue;
				}
				if(dirp->d_ino)
					return(NO);
				size -= sizeof(DIRECT);
			}
		}
	}
	if(size <= 0)	return(YES);
	else	return(NO);
}

setup(dev)
char *dev;
{
#ifndef STANDALONE
	register n;
	register BUFAREA *bp;
	daddr_t bcnt, nscrblk;
	dev_t rootdev;
	extern dev_t pipedev;	/* non-zero iff standard input is a pipe,
				 * which means we can't check pipedev */
#else STANDALONE
	char	fname[7], fpack[7];
#endif STANDALONE
	register MEMSIZE msize;
	char *mbase;
	off_t smapsz, lncntsz, totsz;
#ifndef STANDALONE
	struct {
		daddr_t	tfree;
		ino_t	tinode;
		char	fname[6];
		char	fpack[6];
	} ustatarea;
	struct stat statarea;

	if(stat("/",&statarea) < 0)
		errexit("%c %sCan't stat root\n",id,devname);
	rootdev = statarea.st_dev;
	if(stat(dev,&statarea) < 0) {
		error("%c %sCan't stat %s\n",id,devname,dev);
		return(NO);
	}
	hotroot = 0;
	rawflg = 0;
	if((statarea.st_mode & S_IFMT) == S_IFBLK) {
		if(rootdev == statarea.st_rdev)
			hotroot++;
		else if(ustat(statarea.st_rdev,&ustatarea) >= 0) {
			if(!nflag) {
				error("%c %s%s is a mounted file system, ignored\n",
					id,devname,dev);
				return(NO);
			}
			hotroot++;
		}
		if ( pipedev == statarea.st_rdev )
		{	error( "%c %s%s is pipedev, ignored", id,
					devname, dev);
			return(NO);
		}
	}
	else if((statarea.st_mode & S_IFMT) == S_IFCHR)
		rawflg++;
	else {
		error("%c %s%s is not a block or character device\n",id,devname,dev);
		return(NO);
	}
#endif STANDALONE
	printf("\n%c %s",id,dev);
	if((nflag && !csflag) || (dfile.wfdes == -1))
		printf(" (NO WRITE)");
	printf("\n");
	pss2done = 0;
	fixfree = 0;
	dfile.mod = 0;
	n_files = n_blks = n_free = 0;
	muldup = enddup = &duplist[0];
	badlnp = &badlncnt[0];
	lfdir = 0;
	rplyflag = 0;
	initbarea(&fileblk);
	initbarea(&inoblk);
	sfile.wfdes = sfile.rfdes = -1;
#ifndef STANDALONE
	rmscr = 0;
#endif STANDALONE
	if(getblk(&sblk,SUPERB) == NULL) {
		ckfini();
		return(NO);
	}
	max_inodes = ((ino_t)superblk.s_isize - (SUPERB+1)) * INOPB;
	fmax = superblk.s_fsize;		/* first invalid blk num */

	fmin = (daddr_t)superblk.s_isize;
	bmapsz = roundup(howmany(fmax,BITSPB),sizeof(*lncntp));

	if(fmin >= fmax || 
		(max_inodes/INOPB) != ((ino_t)superblk.s_isize-(SUPERB+1))) {
#ifndef STANDALONE
		error("%c %sSize check: fsize %ld isize %d\n",id,devname,
			superblk.s_fsize,superblk.s_isize);
#else STANDALONE
		error("%c %sSize check: fsize %D isize %d\n",id,devname,
			superblk.s_fsize,superblk.s_isize);
#endif STANDALONE
		ckfini();
		return(NO);
	}
#ifndef STANDALONE
	printf("%c %sFile System: %.6s Volume: %.6s\n\n",id,devname,
		superblk.s_fname,superblk.s_fpack);
#else STANDALONE
	strncpy(fname, superblk.s_fname, 6);	fname[6] = '\0';
	strncpy(fpack, superblk.s_fpack, 6);	fpack[6] = '\0';
	printf("%c %sFile System: %s Volume: %s\n\n",id,devname, fname, fpack);
#endif STANDALONE

	smapsz = roundup(howmany((long)(max_inodes+1),STATEPB),sizeof(*lncntp));
	lncntsz = (long)(max_inodes+1) * sizeof(*lncntp);
	if(bmapsz > smapsz+lncntsz)
		smapsz = bmapsz-lncntsz;
	totsz = bmapsz+smapsz+lncntsz;
	msize = memsize;
	mbase = membase;
#ifndef STANDALONE
	if(rawflg) {
		if(msize < (MEMSIZE)(NINOBLK*BSIZE) + 2*sizeof(BUFAREA))
			rawflg = 0;
		else {
			msize -= (MEMSIZE)NINOBLK*BSIZE;
			mbase += (MEMSIZE)NINOBLK*BSIZE;
			niblk = NINOBLK;
			startib = fmax;
		}
	}
#endif STANDALONE
	clear(mbase,msize);
	if((off_t)msize < totsz) {
#ifndef STANDALONE
		bmapsz = roundup(bmapsz,BSIZE);
		smapsz = roundup(smapsz,BSIZE);
		lncntsz = roundup(lncntsz,BSIZE);
		nscrblk = (bmapsz+smapsz+lncntsz)>>BSHIFT;
		if(tflag == 0) {
			printf("\n%c %sNEED SCRATCH FILE (%ld BLKS)\n",id,devname,nscrblk);
			do {
				printf("%c %sENTER FILENAME:\n",id,devname);
				if((n = getline(stdin,scrfile,sizeof(scrfile))) == EOF)
					errexit("\n");
			} while(n == 0);
		}
		if(stat(scrfile,&statarea) < 0 ||
			(statarea.st_mode & S_IFMT) == S_IFREG)
			rmscr++;
		if((sfile.wfdes = creat(scrfile,0666)) < 0 ||
			(sfile.rfdes = open(scrfile,0)) < 0) {
			error("%c %sCan't create %s\n",id,devname,scrfile);
			ckfini();
			return(NO);
		}
		bp = &((BUFAREA *)mbase)[(msize/sizeof(BUFAREA))];
		poolhead = NULL;
		while(--bp >= (BUFAREA *)mbase) {
			initbarea(bp);
			bp->b_next = poolhead;
			poolhead = bp;
		}
		bp = poolhead;
		for(bcnt = 0; bcnt < nscrblk; bcnt++) {
			bp->b_bno = bcnt;
			dirty(bp);
			flush(&sfile,bp);
		}
		blkmap = freemap = statemap = (char *) NULL;
		lncntp = (short *) NULL;
		smapblk = bmapsz / BSIZE;
		lncntblk = smapblk + smapsz / BSIZE;
		fmapblk = smapblk;
#else STANDALONE
		printf("not enough memory:  msize=%d, totsz=%d\n",
			msize, totsz);
		errexit("\n");
#endif STANDALONE
	}
	else {
#ifndef STANDALONE
		if(rawflg && (off_t)msize > totsz+BSIZE) {
			niblk += (unsigned)((off_t)msize-totsz)>>BSHIFT;
#if FsTYPE==2
			if(niblk > MAXRAW / 2)
				niblk = MAXRAW / 2;
#else
			if(niblk > MAXRAW)
				niblk = MAXRAW;
#endif
			msize = memsize - (niblk*BSIZE);
			mbase = membase + (niblk*BSIZE);
		}
#endif STANDALONE
		poolhead = NULL;
		blkmap = mbase;
		statemap = &mbase[(MEMSIZE)bmapsz];
		freemap = statemap;
		lncntp = (short *)&statemap[(MEMSIZE)smapsz];
	}
	return(YES);
}

checksb(dev)
char *dev;
{
	if((dfile.rfdes = open(dev,0)) < 0) {
		error("%c %sCan't open %s\n",id,devname,dev);
		return(NO);
	}
	if((dfile.wfdes = open(dev,1)) < 0)
		dfile.wfdes = -1;
	if(getblk(&sblk,SUPERB) == NULL) {
		ckfini();
		return(NO);
	}
return(YES);
}

DINODE *
ginode()
{
	register DINODE *dp;
#ifndef STANDALONE
	register char *mbase;
#endif STANDALONE
	register daddr_t iblk;

	if(inum > max_inodes)
		return(NULL);
	iblk = itod(inum);
#ifndef STANDALONE
	if(rawflg) {
		mbase = membase;
		if(iblk < startib || iblk >= startib+niblk) {
			if(inoblk.b_dirty)
				bwrite(&dfile,mbase,startib,niblk*BSIZE);
			inoblk.b_dirty = 0;
			if(bread(&dfile,mbase,iblk,niblk*BSIZE) == NO) {
				startib = fmax;
				return(NULL);
			}
			startib = iblk;
		}
		dp = (DINODE *)&mbase[(unsigned)((iblk-startib)<<BSHIFT)];
	} else
#endif STANDALONE
	if(getblk(&inoblk,iblk) != NULL)
		dp = inoblk.b_un.b_dinode;
	else
		return(NULL);
	return(dp + itoo(inum));
}

reply(s)
char *s;
{
	char line[80];

	rplyflag = 1;
	line[0] = '\0';
	printf("\n%c %s%s? ",id,devname,s);
	if(nflag || dfile.wfdes < 0) {
		printf(" no\n\n");
		return(NO);
	}
	if(yflag) {
		printf(" yes\n\n");
		return(YES);
	}
	while (line[0] == '\0') {
#ifndef STANDALONE
		if(getline(stdin,line,sizeof(line)) == EOF)
			errexit("\n");
#else STANDALONE
		gets(line);
#endif STANDALONE
		printf("\n");
		if(line[0] == 'y' || line[0] == 'Y')
			return(YES);
		if(line[0] == 'n' || line[0] == 'N')
			return(NO);
		printf("%c %sAnswer 'y' or 'n' (yes or no)\n",id,devname);
		line[0] = '\0';
	}
return(NO);
}


#ifndef STANDALONE
getline(fp,loc,maxlen)
FILE *fp;
char *loc;
{
	register n;
	register char *p, *lastloc;

	p = loc;
	lastloc = &p[maxlen-1];
	while((n = getc(fp)) != '\n') {
		if(n == EOF)
			return(EOF);
		if(!isspace(n) && p < lastloc)
			*p++ = n;
	}
	*p = 0;
	return(p - loc);
}
#endif STANDALONE

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
		error("%c %sInvalid -s argument, defaults assumed\n",id,devname);
		cylsize = stepsize = 0;
	}
}


dostate(s,flg)
{
	register char *p;
	register unsigned byte, shift;
	BUFAREA *bp;

	byte = ((unsigned)inum)/STATEPB;
	shift = LSTATE * (((unsigned)inum)%STATEPB);
	if(statemap != NULL) {
		bp = NULL;
		p = &statemap[byte];
	}
	else if((bp = getblk(NULL,smapblk+(byte/BSIZE))) == NULL)
		errexit("%c %sFatal I/O error\n",id,devname);
	else
		p = &bp->b_un.b_buf[byte%BSIZE];
	switch(flg) {
		case 0:
			*p &= ~(SMASK<<(shift));
			*p |= s<<(shift);
			if(bp != NULL)
				dirty(bp);
			return(s);
		case 1:
			return((*p>>(shift)) & SMASK);
	}
	return(USTATE);
}


domap(blk,flg)
register daddr_t blk;
{
	register char *p;
	register unsigned n;
	register BUFAREA *bp;
	off_t byte;

	byte = blk >> BITSHIFT;
	n = 1<<((unsigned)(blk & BITMASK));
	if(flg & 04) {
		p = freemap;
		blk = fmapblk;
	}
	else {
		p = blkmap;
		blk = 0;
	}
	if(p != NULL) {
		bp = NULL;
		p += (unsigned)byte;
	}
	else if((bp = getblk(NULL,blk+(byte>>BSHIFT))) == NULL)
		errexit("%c %sFatal I/O error\n",id,devname);
	else
		p = &bp->b_un.b_buf[(unsigned)(byte&BMASK)];
	switch(flg&03) {
		case 0: /* set */
			*p |= n;
			break;
		case 1: /* get */
			n &= *p;
			bp = NULL;
			break;
		case 2: /* clear */
			*p &= ~n;
	}
	if(bp != NULL)
		dirty(bp);
	return(n);
}


dolncnt(val,flg)
short val;
{
	register short *sp;
	register BUFAREA *bp;

	if(lncntp != NULL) {
		bp = NULL;
		sp = &lncntp[(unsigned)inum];
	}
	else if((bp = getblk(NULL,lncntblk+((unsigned)inum/SPERB))) == NULL)
		errexit("%c %sFatal I/O error\n",id,devname);
	else
		sp = &bp->b_un.b_lnks[(unsigned)inum%SPERB];
	switch(flg) {
		case 0:
			*sp = val;
			break;
		case 1:
			bp = NULL;
			break;
		case 2:
			(*sp)--;
	}
	if(bp != NULL)
		dirty(bp);
	return(*sp);
}


BUFAREA *
getblk(bp,blk)
register daddr_t blk;
register BUFAREA *bp;
{
	register struct filecntl *fcp;

	if(bp == NULL) {
		bp = search(blk);
		fcp = &sfile;
	}
	else
		fcp = &dfile;
	if(bp->b_bno == blk)
		return(bp);
	if(blk == SUPERB) {
		flush(fcp,bp);
		if(lseek(fcp->rfdes,(long)SUPERBOFF,0) < 0)
			rwerr("SEEK",blk);
		else if(read(fcp->rfdes,bp->b_un.b_buf,SBSIZE) == SBSIZE) {
			bp->b_bno = blk;
			return(bp);
		}
		rwerr("READ",blk);
		bp->b_bno = (daddr_t)-1;
		return(NULL);
	}
	flush(fcp,bp);
	if(bread(fcp,bp->b_un.b_buf,blk,BSIZE) != NO) {
		bp->b_bno = blk;
		return(bp);
	}
	bp->b_bno = (daddr_t)-1;
	return(NULL);
}


flush(fcp,bp)
struct filecntl *fcp;
register BUFAREA *bp;
{
	if(bp->b_dirty) {
		if(bp->b_bno == SUPERB) {
			if(fcp->wfdes < 0) {
				bp->b_dirty = 0;
				return;
			}
			if(lseek(fcp->wfdes,(long)SUPERBOFF,0) < 0)
				rwerr("SEEK",bp->b_bno);
			else if(write(fcp->wfdes,bp->b_un.b_buf,SBSIZE) == SBSIZE) {
				fcp->mod = 1;
				bp->b_dirty = 0;
				return;
			}
			rwerr("WRITE",SUPERB);
			bp->b_dirty = 0;
			return;
		}
		bwrite(fcp,bp->b_un.b_buf,bp->b_bno,BSIZE);
	}
	bp->b_dirty = 0;
}

rwerr(s,blk)
char *s;
daddr_t blk;
{
#ifndef STANDALONE
	printf("\n%c %sCAN NOT %s: BLK %ld",id,devname,s,blk);
#else STANDALONE
	printf("\n%c %sCAN NOT %s: BLK %D",id,devname,s,blk);
#endif STANDALONE
	if(reply("CONTINUE") == NO)
		errexit("%c %sProgram terminated\n",id,devname);
}


sizechk(dp)
register DINODE *dp;
{
	off_t size, nblks;

	size = howmany(dp->di_size,BSIZE);
	nblks = size;
	size -= NADDR-3;
	while(size > 0) {
		nblks += howmany(size,NINDIR);
		size--;
		size /= NINDIR;
	}
	if(!qflag) {
		if(nblks != filsize)
			printf("%c %sPOSSIBLE %s SIZE ERROR I=%u\n\n",
				id,devname,DIR?"DIR":"FILE",inum);
		if(DIR && (dp->di_size % sizeof(DIRECT)) != 0)
			printf("%c %sDIRECTORY MISALIGNED I=%u\n\n",id,devname,inum);
	}
}


ckfini()
{
	flush(&dfile,&fileblk);
	flush(&dfile,&sblk);
	flush(&dfile,&inoblk);
	close(dfile.rfdes);
	close(dfile.wfdes);
	close(sfile.rfdes);
	close(sfile.wfdes);
#ifndef STANDALONE
	if(rmscr) {
		unlink(scrfile);
	}
#endif
}


pinode()
{
	register DINODE *dp;
	register char *p;
#ifndef STANDALONE
	char uidbuf[200];
#endif STANDALONE
	char *ctime();

	printf(" I=%u ",inum);
	if((dp = ginode()) == NULL)
		return;
	printf(" OWNER=");
#ifndef STANDALONE
	if(getpw((int)dp->di_uid,uidbuf) == 0) {
		for(p = uidbuf; *p != ':'; p++);
		*p = 0;
		printf("%s ",uidbuf);
	}
	else
#endif STANDALONE
		printf("%d ",dp->di_uid);
	printf("MODE=%o\n",dp->di_mode);
#ifndef STANDALONE
	printf("%c %sSIZE=%ld ",id,devname,dp->di_size);
	p = ctime(&dp->di_mtime);
	printf("MTIME=%12.12s %4.4s ",p+4,p+20);
#else STANDALONE
	printf("%c %sSIZE=%D ",id,devname,dp->di_size);
	p = ctime(&dp->di_mtime);
	p[16] = p[24] = '\0';
	printf("MTIME=%s EDT %s ",p+4,p+20);
#endif STANDALONE
}


copy(fp,tp,size)
register char *tp, *fp;
MEMSIZE size;
{
	while(size--)
		*tp++ = *fp++;
}


freechk()
{
	register daddr_t *ap;

	if(freeblk.df_nfree == 0)
		return;
	do {
		if(freeblk.df_nfree <= 0 || freeblk.df_nfree > NICFREE) {
			printf("%c %sBAD FREEBLK COUNT\n",id,devname);
			fixfree = 1;
			return;
		}
		ap = &freeblk.df_free[freeblk.df_nfree];
		while(--ap > &freeblk.df_free[0]) {
			if(pass5(*ap) == STOP)
				return;
		}
		if(*ap == (daddr_t)0 || pass5(*ap) != KEEPON)
			return;
	} while(getblk(&fileblk,*ap) != NULL);
}


makefree()
{
	register i, cyl, step;
	int j;
	char flg[MAXCYL];
	short addr[MAXCYL];
	daddr_t blk, baseblk;

	superblk.s_nfree = 0;
	superblk.s_flock = 0;
	superblk.s_fmod = 0;
	superblk.s_tfree = 0;
	superblk.s_ninode = 0;
	superblk.s_ilock = 0;
	superblk.s_ronly = 0;
	if(cylsize == 0 || stepsize == 0) {
		step = superblk.s_dinfo[0];
		cyl = superblk.s_dinfo[1];
	}
	else {
		step = stepsize;
		cyl = cylsize;
	}
	if(step > cyl || step <= 0 || cyl <= 0 || cyl > MAXCYL) {
		error("%c %sDefault free list spacing assumed\n",id,devname);
		step = STEPSIZE;
		cyl = CYLSIZE;
	}
	superblk.s_dinfo[0] = step;
	superblk.s_dinfo[1] = cyl;
	clear(flg,sizeof(flg));
#if FsTYPE==2
	step /= 2;
	cyl /= 2;
#endif
	i = 0;
	for(j = 0; j < cyl; j++) {
		while(flg[i])
			i = (i + 1) % cyl;
		addr[j] = i + 1;
		flg[i]++;
		i = (i + step) % cyl;
	}
	baseblk = (daddr_t)roundup(fmax,cyl);
	clear(&freeblk,BSIZE);
	freeblk.df_nfree++;
	for( ; baseblk > 0; baseblk -= cyl)
		for(i = 0; i < cyl; i++) {
			blk = baseblk - addr[i];
			if(!outrange(blk) && !getbmap(blk)) {
				superblk.s_tfree++;
				if(freeblk.df_nfree >= NICFREE) {
					fbdirty();
					fileblk.b_bno = blk;
					flush(&dfile,&fileblk);
					clear(&freeblk,BSIZE);
				}
				freeblk.df_free[freeblk.df_nfree] = blk;
				freeblk.df_nfree++;
			}
		}
	superblk.s_nfree = freeblk.df_nfree;
	for(i = 0; i < NICFREE; i++)
		superblk.s_free[i] = freeblk.df_free[i];
	sbdirty();
}


clear(p,cnt)
register char *p;
MEMSIZE cnt;
{
	while(cnt--)
		*p++ = 0;
}


BUFAREA *
search(blk)
daddr_t blk;
{
	register BUFAREA *pbp, *bp;

	for(bp = (BUFAREA *) &poolhead; bp->b_next; ) {
		pbp = bp;
		bp = pbp->b_next;
		if(bp->b_bno == blk)
			break;
	}
	pbp->b_next = bp->b_next;
	bp->b_next = poolhead;
	poolhead = bp;
	return(bp);
}


findino(dirp)
register DIRECT *dirp;
{
	register char *p1, *p2;

	if(dirp->d_ino == 0)
		return(KEEPON);
	for(p1 = dirp->d_name,p2 = srchname;*p2++ == *p1; p1++) {
		if(*p1 == 0 || p1 == &dirp->d_name[DIRSIZ-1]) {
			if(dirp->d_ino >= ROOTINO && dirp->d_ino <= max_inodes)
				parentdir = dirp->d_ino;
			return(STOP);
		}
	}
	return(KEEPON);
}


mkentry(dirp)
register DIRECT *dirp;
{
	register ino_t in;
	register char *p;

	if(dirp->d_ino)
		return(KEEPON);
	dirp->d_ino = orphan;
	in = orphan;
	p = &dirp->d_name[DIRSIZ];
	while(p != &dirp->d_name[6])
		*--p = 0;
	while(p > dirp->d_name) {
		*--p = (in % 10) + '0';
		in /= 10;
	}
	return(ALTERD|STOP);
}


chgdd(dirp)
register DIRECT *dirp;
{
	if(dirp->d_name[0] == '.' && dirp->d_name[1] == '.' &&
	dirp->d_name[2] == 0) {
		dirp->d_ino = lfdir;
		return(ALTERD|STOP);
	}
	return(KEEPON);
}


linkup()
{
	register DINODE *dp;
	register lostdir;
	register ino_t pdir;
	register ino_t *blp;
	int n;

	if((dp = ginode()) == NULL)
		return(NO);
	lostdir = DIR;
	pdir = parentdir;
	if(!FIFO || !qflag || nflag) {
		printf("%c %sUNREF %s ",id,devname,lostdir ? "DIR" : "FILE");
		pinode();
	}
	if(DIR) {
		if(dp->di_size > EMPT) {
			if((n = chkempt(dp)) == NO) {
				printf(" (NOT EMPTY)");
				if(!nflag) {
					printf(" MUST reconnect\n");
					goto connect;
				}
				else
					printf("\n");
			}
			else if(n != SKIP) {
				printf(" (EMPTY)");
				if(!nflag) {
					printf(" Cleared\n");
					return(REM);
				}
				else
					printf("\n");
			}
		}
		else {
			printf(" (EMPTY)");
			if(!nflag) {
				printf(" Cleared\n");
				return(REM);
			}
			else
				printf("\n");
		}
	}
	if(REG||LNK)
		if(!dp->di_size) {
			printf(" (EMPTY)");
			if(!nflag) {
				printf(" Cleared\n");
				return(REM);
			}
			else
				printf("\n");
		}
		else
			printf(" (NOT EMPTY)\n");
	if(FIFO && !nflag) {
		if(!qflag)	printf(" -- REMOVED");
		printf("\n");
		return(REM);
	}
	if(FIFO && nflag)
		return(NO);
	if(reply("RECONNECT") == NO)
		return(NO);
connect:
	orphan = inum;
	if(lfdir == 0) {
		inum = ROOTINO;
		if((dp = ginode()) == NULL) {
			inum = orphan;
			return(NO);
		}
		pfunc = findino;
		srchname = lfname;
		filsize = dp->di_size;
		parentdir = 0;
		ckinode(dp,DATA);
		inum = orphan;
		if((lfdir = parentdir) == 0) {
			printf("%c %sSORRY. NO lost+found DIRECTORY\n\n",id,devname);
			return(NO);
		}
	}
	inum = lfdir;
	if((dp = ginode()) == NULL || !DIR || getstate() != FSTATE) {
		inum = orphan;
		printf("%c %sSORRY. NO lost+found DIRECTORY\n\n",id,devname);
		return(NO);
	}
	if(dp->di_size & BMASK) {
		dp->di_size = roundup(dp->di_size,BSIZE);
		inodirty();
	}
	filsize = dp->di_size;
	inum = orphan;
	pfunc = mkentry;
	if((ckinode(dp,DATA) & ALTERD) == 0) {
		printf("%c %sSORRY. NO SPACE IN lost+found DIRECTORY\n\n",id,devname);
		return(NO);
	}
	declncnt();
	if((dp = ginode()) && !dp->di_nlink) {
		dp->di_nlink++;
		inodirty();
		setlncnt(getlncnt()+1);
		if(lostdir) {
			for(blp = badlncnt; blp < badlnp; blp++)
				if(*blp == inum) {
					*blp = 0L;
					break;
				}
		}
	}
	if(lostdir) {
		pfunc = chgdd;
		filsize = dp->di_size;
		ckinode(dp,DATA);
		inum = lfdir;
		if((dp = ginode()) != NULL) {
			dp->di_nlink++;
			inodirty();
			setlncnt(getlncnt()+1);
		}
		inum = orphan;
		printf("%c %sDIR I=%u CONNECTED. ",id,devname,orphan);
		printf("%c %sPARENT WAS I=%u\n\n",id,devname,pdir);
	}
	return(YES);
}


bread(fcp,buf,blk,size)
daddr_t blk;
register struct filecntl *fcp;
#ifndef STANDALONE
register MEMSIZE size;
#else STANDALONE
register int size;
#endif STANDALONE
char *buf;
{
	if(lseek(fcp->rfdes,blk<<BSHIFT,0) < 0)
		rwerr("SEEK",blk);
	else if(read(fcp->rfdes,buf,size) == size)
		return(YES);
	rwerr("READ",blk);
	return(NO);
}


bwrite(fcp,buf,blk,size)
daddr_t blk;
register struct filecntl *fcp;
#ifndef STANDALONE
register MEMSIZE size;
#else STANDALONE
register int size;
#endif STANDALONE
char *buf;
{
	if(fcp->wfdes < 0)
		return(NO);
	if(lseek(fcp->wfdes,blk<<BSHIFT,0) < 0)
		rwerr("SEEK",blk);
	else if(write(fcp->wfdes,buf,size) == size) {
		fcp->mod = 1;
		return(YES);
	}
	rwerr("WRITE",blk);
	return(NO);
}

#ifndef STANDALONE
catch()
{
	ckfini();
	exit(4);
}
#endif
