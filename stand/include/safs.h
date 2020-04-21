/*
* $Source: /d2/3.7/src/stand/include/RCS/safs.h,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:13:49 $
*/
/*
 * standalone file system definitions.
 */

# define ROOTINO	((ino_t)2)
# define FIRSTINO	((ino_t)1)
# define MAXINDIRECT	4

# define MAXADDRS	20

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
	struct { struct { off_t val[1]; } ic_size; } i_ic;
	daddr_t	addrs[MAXADDRS];

	off_t off;
};

struct dio
{
	struct direct *dirptr;
	char *dirbase;
	short dircnt;
	struct fino *dirfile;
};

# define FsbToDb(n)	((n)<<FsDblockShift)

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
char *Ibuf;
