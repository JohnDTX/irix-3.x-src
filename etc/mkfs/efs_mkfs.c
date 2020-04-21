#
/*
 * efs - mkfs.c --
 * make an efs file system.
 *
 *	mkfs [specs] device proto
 *
 *		proto == blocks[:inodes]
 *			 protofile
 *
 *		specs == -C cgisize:cginodes
 *			 -D protodev
 *			 -G heads sectors
 */

# include "toyfs.h"

extern USR U;

# include "sys/dir.h"
# include "sys/dklabel.h"

# include "sys/signal.h"
# include "sys/stat.h"
# ifdef MKFS_FSTAB
# include "fstab.h"
# endif MKFS_FSTAB

# include "stdio.h"
# include "bitmap.h"


# define DEBUG mkfs_debug
# define ROOTUID	0
# ifdef DEBUG
extern short DEBUG;
#	define dprintf(x)	(DEBUG?printf x:0)
#	define ASSERT(c)	if(!(c))_assert("c",__FILE__,__LINE__)
# else  DEBUG
#	define dprintf(x)
#	define ASSERT(c)
# endif DEBUG
short mkfs_debug = 0;

# define HowMany(n, m)		(((n)+(m)-1)/(m))
# define RoundUp(n, m)		(HowMany(n, m) * (m))
# define RoundDown(n, m)	((n) - (n)%(m))

# define MEMPAGESHIFT	12
# define MEMPAGESIZE	(1<<MEMPAGESHIFT)
# define MEMPAGEMASK	(MEMPAGESIZE-1)

# define EFS_SUPER	SUPERB
# define EFS_BITMAP	BITMAPB
# define MAXINO		65535L

# define LOSTFOUNDBBS	10
# define NBBS		64

float BELL_IFRAG = 0.8;		/* fraction of bell fs's inodes */
int BELL_BLOCKSPERFILE = 8;	/* blocks per inode */
int DFL_CGFSIZE = 4000;		/* default cg size */
int DFL_PREALLOC = 16;		/* default write prealloc */


char *progname = "mkfs";
char *usage = "\
usage:  mkfs [specs] device proto";

int Errs = 0;

char *Filsys = 0;		/* device argument */
char *Proto = 0;		/* proto argument */
char *DefaultFilsys = 0;	/* filsys for defaults */
char verbose = 1;		/* flag - verbose operation */
char superonly = 0;		/* flag - superblock only */
char cgalign = 0;		/* flag - align cg's on cyl boundary */

int cggrain = MEMPAGESIZE/BBSIZE;
int igrain = MEMPAGESIZE/BBSIZE;
int bmgrain = MEMPAGESIZE/BBSIZE;

/* ----- imports from mkfs.c */
# define GetProtoMode(x)	gmode()
# define GetProtoString(x)	getstr()
# define GetProtoNumber(x)	getnum()
# define ProtoFILE		fin
# define ProtoInput		charp
# define ProtoString		string
# define ZeroBuf		work1
extern char *charp;
extern getstr();
extern bootcopy();
extern long getnum();
extern FILE *fin;
extern char string[];
extern char work0[];
extern char work1[];
/* ----- */


int ifd,ofd;
struct efs *efs_super;
FS *toy_filsys;

char *BitMap = 0;

/* ----- parameters */
long Cgs = 0;
long FirstCg = 0;
long InodeBlocksPerCg = 0;
long BlocksPerCg = 0;
long Inodes;
long InodesPerCg = 0;
long ArgBlocks = 0;
long Blocks = 0;
long Cyls = 0;
long BitMapBlocks = 0;
long BlocksPerCyl = 0;
long Heads = 0;
long Sectors = 0;
/* ----- */


main(argc, argv)
	int argc; char **argv;
{
	extern char *rindex();

	int oargc; char **oargv;
	register char *ap;

	oargc = argc; oargv = argv;

	argc--;
	progname = *argv++;
	if ((ap = rindex(progname, '.')) != 0
	 && strcmp(ap, ".bell") == 0)
		exit(bell_main(oargc, oargv));

	efs_main(oargc, oargv);
	exit(-1);
}

efs_main(argc, argv)
	int argc; char **argv;
{
	extern char *cook_dev();

	extern char *skipcnum();

	register char *ap, *cp;
	char argbotch;

	argc--; argv++;

	argbotch = 0;
	while (argc > 0 && *(ap = *argv) == '-') {
		argc--; argv++; ap++;

		while (*ap != 000)
		switch (*ap++) {
		case 'd':
			mkfs_debug = !mkfs_debug;
			break;

		case 'z':
			superonly = 1;
			break;

		case 'v':
			break;

		case 'G':
			if ((argc -= 2) < 0) {
				errwarn("missing -G {heads sectors}");
				argbotch--;
				break;
			}
			if (!is_a_number(*argv++, &Heads))
				argbotch--;
			if (!is_a_number(*argv++, &Sectors))
				argbotch--;
			break;

		case 'D':
			if (--argc < 0) {
				errwarn("missing -D {dev}");
				argbotch--;
			}
			DefaultFilsys = *argv++;
			break;

		case 'C':
			if (--argc < 0) {
				errwarn("missing -C {cgsize:cginodes}");
				argbotch--;
				break;
			}

			InodesPerCg = 0;
			cp = *argv++;
			cp = skipcnum(cp, 10, &BlocksPerCg);
			if (*cp == ':') {
				cp++;
				if (!is_a_number(cp, &InodesPerCg))
					argbotch--;
			}
			else
			if (*cp != 000) {
				num_complaint(cp);
				argbotch--;
			}
			break;

		default:
			errwarn("unknown flag %c", ap[-1]);
			argbotch--;
			break;
		}
	}

	if (--argc >= 0)
		Filsys = *argv++;
	if (--argc >= 0)
		Proto = *argv++;

	if (Filsys == 0)
		argbotch--;

	if (argbotch)
		errexit(usage);

	if (DefaultFilsys == 0)
		DefaultFilsys = Filsys;
	DefaultFilsys = cook_dev(DefaultFilsys);

	if ((ofd = creat(Filsys, IRWXRWXRWX)) < 0)
		scerrexit("can't creat filsys %s", Filsys);

	ProtoInput = "d--777 0 0 $ ";
	if (Proto != 0) {
		ArgBlocks = 0;
		Inodes = 0;
		cp = Proto;
		cp = skipcnum(cp, 10, &ArgBlocks);
		if (*cp == ':') {
			cp++;
			cp = skipcnum(cp, 10, &Inodes);
		}
		if (*cp != 000) {
			ArgBlocks = 0;
			Inodes = 0;
		}

		if (ArgBlocks == 0) {
			ProtoInput = 0;

			if ((ProtoFILE = fopen(Proto, "r")) == 0)
				errexit("can't open protofile %s", Proto);
			GetProtoString(ProtoFILE);
			bootcopy(ProtoString, ofd);
			ArgBlocks = GetProtoNumber(ProtoFILE);
			Inodes = GetProtoNumber(ProtoFILE);
		}
	}

	if (argbotch)
		errexit(usage);

	fillin_efs_defaults();

	if ((ifd = open(Filsys, 0)) < 0)
		scerrexit("can't read filsys %s!!", Filsys);

	build_clean_efs();

	build_efs_files();

	update_efs();

	exit(0);
}

/*
 * fillin_efs_defaults --
 * fill in default parameters, integrating
 * information from various sources:
 *	1. args from command line
 *	2. fstab
 *	3. disk label
 *	4. internal formula
 *
 * specifically,
 *	Heads
 *	Sectors
 *
 *	Blocks (Cyls)
 *	Inodes
 *
 *	BlocksPerCg
 *	InodeBlocksPerCg (InodesPerCg)
 *
 * for right now, the following assumptions hold:
 *	a cg must start and end on a cggrain boundary
 *	the last cg must be as big or bigger than cgsize.
 *	cgisize must be a multiple of igrain.
 */
fillin_efs_defaults()
{
# ifdef MKFS_FSTAB
	extern struct fstab *getfsdevice();
	register struct fstab *fp;
# endif MKFS_FSTAB
	struct disk_label label;
	auto int mypart;

	/*
	 * if there is a default filsys specified,
	 * get its fstab and label info for possible
	 * use later on.
	 */
	mypart = -1;
# ifdef MKFS_FSTAB
	fp = 0;
# endif MKFS_FSTAB
	if (DefaultFilsys != 0) {
# ifdef MKFS_FSTAB
		fp = getfsdevice(DefaultFilsys);
# endif MKFS_FSTAB
		getdklabel(DefaultFilsys, &label, &mypart);
	}

	/*
	 * work out the disk geometry, especially cylsize.
	 */
# ifdef MKFS_FSTAB
	if (Heads == 0)
		if (fp != 0) Heads = fp->fs_heads;
# endif MKFS_FSTAB
	if (Heads == 0)
		if (mypart >= 0) Heads = label.d_heads;
# ifdef MKFS_FSTAB
	if (Sectors == 0)
		if (fp != 0) Sectors = fp->fs_sectors;
# endif MKFS_FSTAB
	if (Sectors == 0)
		if (mypart >= 0) Sectors = label.d_sectors;
	BlocksPerCyl = Heads*Sectors;
	if (BlocksPerCyl == 0)
		errexit("cylsize == 0!");

	if (cgalign)
		cggrain = BlocksPerCyl;

	/*
	 * now determine raw size of file system.
	 * round down to cggrain boundary.
	 */
# ifdef MKFS_FSTAB
	if (ArgBlocks == 0)
		if (fp != 0) ArgBlocks = fp->fs_blocks;
# endif MKFS_FSTAB
	if (ArgBlocks == 0)
		if (mypart >= 0) ArgBlocks = label.d_map[mypart].d_size;
# ifdef MKFS_FSTAB
	if (ArgBlocks == 0)
		if (fp != 0) ArgBlocks = fp->fs_cyls * BlocksPerCyl;
# endif MKFS_FSTAB
	Blocks = ArgBlocks;
	Blocks = RoundDown(Blocks, cggrain);

	/*
	 * determine the size needed for the bitmap,
	 * and concurrently the size left for cg's.
	 */
	BitMapBlocks = HowMany(Blocks, BITSPERBYTE*BBSIZE);
	FirstCg = BITMAPB+BitMapBlocks;
	FirstCg = RoundUp(FirstCg, bmgrain);
	if (FirstCg >= Blocks)
		errexit("size %ld too small", Blocks);

	/*
	 * determine number and size of cg's (heuristic).
	 */
# ifdef MKFS_FSTAB
	if (BlocksPerCg == 0)
		if (fp != 0) BlocksPerCg = fp->fs_cgfsize;
# endif MKFS_FSTAB
	if (BlocksPerCg == 0)
		BlocksPerCg = DFL_CGFSIZE;
	if (BlocksPerCg > Blocks - FirstCg)
		BlocksPerCg = Blocks - FirstCg;
	BlocksPerCg = RoundDown(BlocksPerCg, cggrain);
	if (BlocksPerCg == 0)
		errexit("impossible cg layout (cgfsize == 0)");

	Cgs = HowMany(Blocks - FirstCg, BlocksPerCg);
	if (Cgs == 0)
		errexit("impossible cg layout (ncg == 0)");
 
	/*
	 * having determined how many cg's,
	 * re-distribute them evenly.
	 * throw away leftover blocks.
	 */
	BlocksPerCg = (Blocks - FirstCg)/Cgs;
	BlocksPerCg = RoundDown(BlocksPerCg, cggrain);
	Blocks = FirstCg + BlocksPerCg * Cgs;

	/*
	 * determine cgisize (heuristic).
	 * align to igrain.
	 */
	if (InodesPerCg == 0)
		InodesPerCg = Inodes/Cgs;
# ifdef MKFS_FSTAB
	if (InodesPerCg == 0)
		if (fp != 0) InodesPerCg = fp->fs_cgisize*EFS_INOPBB;
# endif MKFS_FSTAB
	if (InodesPerCg == 0)
		InodesPerCg = (BlocksPerCg/BELL_BLOCKSPERFILE)*BELL_IFRAG;

	Inodes = InodesPerCg*Cgs;
	if (Inodes >= MAXINO)
		InodesPerCg = MAXINO/Cgs - (EFS_INOPBB-1);
	InodeBlocksPerCg = HowMany(InodesPerCg, EFS_INOPBB);
	InodeBlocksPerCg = RoundDown(InodeBlocksPerCg, igrain);
	if (InodeBlocksPerCg == 0)
		InodeBlocksPerCg = igrain;
	InodesPerCg = InodeBlocksPerCg*EFS_INOPBB;
	Inodes = InodesPerCg*Cgs;

	printf("isize = %ld\n", Inodes / EFS_INOPBB);
	if (verbose) {
		printf("filsys = %s, size = %lu:%lu, unused %ld\n",
				Filsys, Blocks, Inodes, ArgBlocks - Blocks);
		printf("heads = %lu, sectors = %lu\n",
				Heads, Sectors);
		printf("cgsize = %lu:%lu, firstcg = %lu, ncgs = %lu\n",
				BlocksPerCg, InodesPerCg, FirstCg, Cgs);
	}

	if (InodeBlocksPerCg >= BlocksPerCg)
		errexit("impossibly many inodes per cg!");
}

build_clean_efs()
{
	build_efs_super();
	if (superonly)
		exit(0);
	build_efs_bitmap();
	build_efs_cgs();
}

build_efs_super()
{
	register struct efs *sp;

	efs_super = (struct efs *)work0;
	sp = efs_super;

	bzero((char *)sp, BBSIZE);
	sp->fs_size = Blocks;
	sp->fs_sectors = Sectors;
	sp->fs_heads = Heads;

	sp->fs_magic = EFS_MAGIC;
	sp->fs_dirty = 1;
	sp->fs_prealloc = DFL_PREALLOC;
	sp->fs_bmsize = HowMany(Blocks, BITSPERBYTE);

	sp->fs_ncg = Cgs;

	sp->fs_firstcg = FirstCg;

	sp->fs_cgisize = InodeBlocksPerCg;
	sp->fs_cgfsize = BlocksPerCg;
	sp->fs_checksum = 0;
	sp->fs_ipcg = InodesPerCg;
	sp->fs_tinode = Inodes - 1;
	sp->fs_tfree = Blocks - (FirstCg + InodeBlocksPerCg*Cgs);

	efs_bwrite(EFS_SUPER, (char *)sp);
}

build_efs_bitmap()
{
	extern char *malloc();

	register int i;

	i = BitMapBlocks << BBSHIFT;
	if ((BitMap = malloc(i)) == 0)
		errexit("Bit map too large!");

	bzero(BitMap + i - BBSIZE, BBSIZE);
	bffff(BitMap, (int)efs_super->fs_bmsize);

	efs_clr_nbits(BitMap, 0L, (int)FirstCg);

	for (i = 0; i < efs_super->fs_ncg; i++) {
		efs_clr_nbits(BitMap,
				(long)CGIMIN(efs_super, i),
				(int)efs_super->fs_cgisize);
	}

	if (lseek(ofd, (off_t)BITMAPOFF, 0) < 0
	 || write(ofd, BitMap, BitMapBlocks << BBSHIFT) < 0)
		errexit("Bit map write error!");

	free(BitMap);
	BitMap = 0;
}

update_efs()
{
	if (toy_closefs(toy_filsys) < 0)
		toy_errexit("update error!");
}

build_efs_cgs()
{
	extern char *malloc();

	register int cgno;
	char *zeros;

	if ((zeros = malloc(BBSIZE*NBBS)) == 0)
		errexit("out of core!");
	bzero(zeros, BBSIZE*NBBS);

	for (cgno = 0; cgno < efs_super->fs_ncg; cgno++) {
		efs_clr_cg(cgno, zeros, NBBS, efs_super->fs_cgisize);
	}

	/* pre-extend in case it's a toy fs */
	efs_bwrite(Blocks-1, zeros);
	free(zeros);
}

efs_clr_cg(cgno, zeros, nbbs, cgisize)
	register int cgno;
	char *zeros;
	int nbbs;
	int cgisize;
{
	register daddr_t bn;
	register int cgrem, n;

	bn = CGIMIN(efs_super, cgno);
	for (cgrem = efs_super->fs_cgisize; cgrem > 0;) {
		n = cgrem < nbbs ? cgrem : nbbs;

		if (lseek(ofd, bn<<BBSHIFT, 0) < 0)
			errexit("seek error!");
		if (write(ofd, zeros, n<<BBSHIFT) < 0)
			errexit("cginode write error!");

		cgrem -= n;
		bn += n;
	}
}

int
efs_bwrite(bn, buf)
	daddr_t bn;
	char *buf;
{
	extern off_t lseek();

dprintf(("bwrite(%d)\n", bn));
	if (lseek(ofd, bn<<BBSHIFT, 0) < 0)
		return -1;
	if (write(ofd, buf, BBSIZE) != BBSIZE)
		return -1;
	return 0;
}

build_efs_files()
{
	extern FS *toy_openfs();
	extern I *toy_iget();
	int mode, uid, gid;

	umask(0);
	toy_init();

	if ((toy_filsys = toy_openfs(Filsys, 2)) == 0)
		toy_errexit("can't build efs files");

	mode = GetProtoMode(ProtoFILE);
	uid = GetProtoNumber(ProtoFILE);
	gid = GetProtoNumber(ProtoFILE);

	build_efs_root(mode, uid, gid);

	toy_sync(toy_filsys);

	efs_protodir((ino_t)ROOTINO);

	build_efs_lostfound();
}

build_efs_root(mode, uid, gid)
	int mode, uid, gid;
{
	extern I *toy_imake();

	register I *ip;
	ino_t rootino;
	TOYDIR d1;

	mode &= IRWXRWXRWX;
	/*
	 * make the first file.
	 * don't let go of it until it has at least 1 link.
	 */
	for (;;) {
		if ((ip = toy_imake(toy_filsys, IFDIR|mode, 0)) == 0)
			toy_errexit("can't create root!");
# ifdef DEBUG
		if( DEBUG ) { toy_idump(ip, 1, stdout); printf("\n"); }
		dprintf((" mode 0%o number %d\n", ip->i_imode, ip->i_number));
# endif DEBUG
		rootino = ip->i_number;
		if (rootino >= ROOTINO)
			break;
	}
	ip->i_iuid = uid;
	ip->i_igid = gid;
	ip->i_flags |= TOY_DIRTY;

	if (rootino != ROOTINO)
		toy_errexit("can't allocate ROOTINO");

	d1.d_ino = ROOTINO;
	d1.d_name = ".";
	d1.d_len = DIRSIZ;
	d1.d_offset = 0;

	if (toy_direnter(toy_filsys, ROOTINO, &d1) < 0)
		toy_errexit("can't form /.");
	ip->i_inlink ++;

	if (toy_link(toy_filsys, "/.", "/..") < 0)
		toy_errexit("can't create /..");

	toy_sync(toy_filsys);
}

build_efs_lostfound()
{
	extern F *toy_mknod();
	register F *fp;
	register int i;

	if ((fp = toy_mknod(toy_filsys, "/lost+found", IFDIR|IRWXRWXRWX, 0))
				 == 0)
		toy_errexit("can't create lost+found!");

	for (i = LOSTFOUNDBBS; --i >= 0;)
		toy_write(fp, ZeroBuf, BBSIZE);

	toy_close(fp);

	if (toy_link(toy_filsys, "/lost+found", "/lost+found/.") < 0
	 || toy_link(toy_filsys, "/", "/lost+found/..") < 0)
		toy_errexit("can't form /lost+found");
}

bffff(tgt, n)
	register char *tgt;
	int n;
{
	while (--n >= 0)
		*tgt++ = 0xFF;
}

efs_protodir(pino)
	ino_t pino;
{
	ino_t oldcurino;

	oldcurino = U.u_curino;
	U.u_curino = pino;

	while (efs_protofile(pino) == 0)
		;

	U.u_curino = oldcurino;
}

/*
 * efs_protofile() --
 * given a starting directory.
 */
int
efs_protofile(pino)
	ino_t pino;
{
	extern F *toy_mknod();

	extern ino_t efs_mkdir();

	register F *fp;
	ino_t ino;
	int mode, uid, gid, dmajor, dminor;
	char name[512];


	GetProtoString(ProtoFILE);
	if (strcmp(ProtoString, "$") == 0)
		return -1;
	strncpy(name, ProtoString, sizeof name);

	/*
	 * get mode, uid and gid
	 */
	mode = GetProtoMode();
	uid = GetProtoNumber();
	gid = GetProtoNumber();

	switch (mode&IFMT) {

	case IFREG:
		/*
		 * regular file
		 * contents is a file name
		 */
		GetProtoString(ProtoFILE);
		efs_regfilecopy(ProtoString, name);
		break;

	case IFBLK:
	case IFCHR:
		/*
		 * special file
		 * content is maj/min types
		 */
		dmajor = GetProtoNumber(ProtoFILE);
		dminor = GetProtoNumber(ProtoFILE);
		if ((fp = toy_mknod(toy_filsys, name,
				mode, makedev(dmajor, dminor))) == 0)
			toy_errexit("can't mknod %s", name);
		toy_close(fp);
		break;

	case IFDIR:
		/*
		 * directory
		 * call recursively
		 */
		ino = efs_mkdir(name);
		efs_protodir(ino);
		break;
	}

	return 0;
}

efs_regfilecopy(src, tgt)
	char *src, *tgt;
{
	extern F *toy_creat();

	register F *fp;
	int fd;
	int cnt;
	char buf[BBSIZE];

	if ((fd = open(src, 0)) < 0)
		errexit("can't open %s", src);

	if ((fp = toy_creat(toy_filsys, tgt, IRWXRWXRWX)) == 0)
		toy_errexit("can't create %s", tgt);
	while ((cnt = read(fd, buf, sizeof buf)) > 0) {
		if (toy_write(fp, buf, cnt) < 0)
			toy_errexit("write error on %s", tgt);
	}
	toy_close(fp);
	close(fd);
}

efs_clr_nbits(b, bn, n)
	register char *b;
	long bn;
	int n;
{
	while (--n >= 0) {
		CLRBIT(b, bn);
		bn++;
	}
}

ino_t
efs_mkdir(name)
	char *name;
{
	extern char *malloc();

	extern F *toy_mknod();
	register F *fp;
	register int i;
	ino_t ino;
	char *xname, *xp;

	i = strlen(name);
	if ((xname = malloc(i + 3 + 1)) == 0)
		errexit("out of core!");
	strcpy(xname, name);
	xp = xname + i;

	if ((fp = toy_mknod(toy_filsys, name, IFDIR|IRWXRWXRWX, 0))
				 == 0)
		toy_errexit("can't create %s!", name);

	ino = fp->f_ip->i_number;

	toy_close(fp);

	strcpy(xp, "/.");
	if (toy_link(toy_filsys, name, xname) < 0)
		toy_errexit("can't form %s!", xname);

	strcpy(xp, "/..");
	if (toy_link(toy_filsys, ".", xname) < 0)
		toy_errexit("can't form /lost+found");

	return ino;
}

is_a_number(s, ip)
	char *s;
	long *ip;
{
	extern char *skipcnum();

	if (*skipcnum(s, 10, ip) != 000) {
		num_complaint(s);
		return 0;
	}
	return 1;
}

num_complaint(s)
	char *s;
{
	errwarn("\"%s\" is not a number", s);
}
