/*
 * standalone file system definitions.
 */

# define ROOTINO	((ino_t)2)
# define FIRSTINO	((ino_t)1)
# define MAXINDIRECT	4

# define MAXADDRS	24
# define MAXFSBSIZE	1024

struct gendir
{
    long d_ino;
    char *d_name;
    short d_len;
};

struct fino
{
	unsigned short flag;
	long inum;
	short naddrs;
	daddr_t	addrs[MAXADDRS];

	off_t off;
	off_t isize;
};

struct dio
{
	struct direct *dirptr;
	char *dirbase;
	short dircnt;
	struct fino *dirfile;
};

# define FsbToDb(n)	((n)<<FsDblockShift)
# define FsbToBytes(n)	((n)<<FsBlockShift)

PROMSTATIC
	short NaddrsPerBlock;
	short NinodesPerBlock;
	short FsBlockSize;
	short FsDblockShift;
	short FsBlockShift;
	short FsBlockOffMask;
	int (*fs_bread)();
	struct fino *(*fs_iget)();
	int (*fs_readi)();
	struct gendir *(*fs_dread)();
