# ifndef __TOYFS__

# define __TOYFS__
/*
 * header file for toy fs library.
 */

# include "sys/param.h"
# include "sys/fs.h"

# define BITMAPB	EFS_BITMAPBB
# define BITMAPOFF	EFS_BITMAPBOFF
# define CGIMIN(fs, cg)	EFS_CGIMIN(fs, cg)
# define ITOD(fs, i)	EFS_ITOBB(fs, i)
# define ITOO(fs, i)	EFS_ITOO(fs, i)

# include "sys/inode.h"


/*
 * struct toybuf is analogous to the kernel's
 * struct buf.  it contains enough parameters
 * to be useful for different fs types.  toy
 * bufs are doubly linked in a chain headed
 * by a toy file system structure.
 */
struct toybuf
{
    struct toybuf *b_forw;		/* forward link in buf chain */
    struct toybuf *b_back;		/* backward link in buf chain */
    int b_flags;			/* dirty, etc */
    short b_count;			/* reference count */
    daddr_t b_blkno;			/* file system block number */
    int b_bcount;			/* buffer size */
    int b_resid;			/* leftover size */
    char *b_addr;			/* buffer data */
};
typedef struct toybuf B;

/*
 * struct toyinode is analogous to the kernel's
 * struct inode.  it contains enough parameters
 * to be useful for different fs types.  in particular,
 * .i_irdev, .i_imode, .i_inlink, .i_iuid, and .i_igid
 * are the "common" parts of an inode.  .i_dinode is a
 * pointer to fs-dependent stuff, probably containing
 * the inode as it appears on the volume.  toy inodes
 * are doubly linked in a chain headed by a toy file
 * system structure (.i_fs).
 */
struct toyinode
{
    struct toyinode *i_forw;		/* forward link in inode chain */
    struct toyinode *i_back;		/* backward link in inode chain */
    short i_flags;			/* dirty, etc */
    short i_count;			/* reference count */
    struct toyfs *i_fs;			/* pointer to toy file system */
    char *i_dinode;			/* fs-dependent representation */

    ino_t i_number;			/* inumber */
    dev_t i_irdev;			/* device number if special */

    unsigned short i_imode;		/* icommon */
    short i_inlink;			/* icommon */
    unsigned short i_iuid;		/* icommon */
    unsigned short i_igid;		/* icommon */
    off_t i_isize;			/* icommon */

    time_t i_iatime;			/* itimes */
    time_t i_imtime;			/* itimes */
    time_t i_ictime;			/* itimes */
};
typedef struct toyinode I;

#define	IRWXRWXRWX	PERMMSK

/*
 * struct toyfile is analogous to the kernel's
 * struct file.
 */
struct toyfile
{
    I *f_ip;				/* pointer to toy inode */
    off_t f_offset;			/* r/w offset */
};
typedef struct toyfile F;

/*
 * struct toyiob is analogous to stdio's FILE type.
 */
struct toyiob
{
    char *ptr;				/* next data */
    char *base;				/* data buffer */
    int basesize;			/* size of data buffer */
    int nused;				/* how much used (writing) */
    int nleft;				/* how much left (reading) */
    F *fd;				/* pointer to toy file */
};
typedef struct toyiob TOYIOB;

/*
 * struct toydir is analogous to the kernel's
 * struct direct.  it contains enough parameters
 * to be useful for different fs types.
 */
struct toydir
{
    char *d_name;			/* pointer to component */
    short d_len;			/* length (ignored) */
    long d_ino;				/* inumber */
    off_t d_offset;			/* offset within directory */
};
typedef struct toydir TOYDIR;

struct toyops
{
    struct toyfs *(*fs_mount)();	/* mount */
    int (*fs_umount)();			/* umount */
    int (*fs_sync)();			/* sync */

    I *(*fs_imake)();			/* creat */
    I *(*fs_iread)();			/* read inode */
    int (*fs_iwrite)();			/* write inode */
    int (*fs_itrunc)();			/* truncate inode */

    off_t (*fs_readi)();		/* read file */
    off_t (*fs_writei)();		/* write file */

    TOYDIR *(*fs_readdir)();		/* read dir */
    int (*fs_writedir)();		/* write dir */
    int (*fs_idump)();			/* dump inode */
};

struct toyparams
{
    int fs_iinosize;			/* internal inode size */
    int fs_dinoshift;			/* log2 disk inode size */
    int fs_isbsize;			/* internal superblock size */
    int fs_dsbsize;			/* disk superblock size */
    int fs_sboff;			/* superblock offset */
    int fs_bshift;			/* block shift */
    char *fs_type;			/* fs type identifier */
};

/*
 * struct toyfs is analogous to the kernel's
 * struct mount.  it contains enough parameters
 * to be useful for different fs types.  in particular,
 * .fs_inodes is a toy inode cache, .fs_bufs is a toy
 * buf cache, and .fs_ops is a vector of appropriate
 * fs entry points.  .fs_filsys is fs-dependent stuff,
 * probably containing the superblock as it appears on
 * the volume.
 */
struct toyfs
{
    char *fs_devname;			/* arg to openfs */
    dev_t fs_dev;			/* dev num */
    int fs_fd;				/* fd */
    short fs_flags;			/* dirty, etc */

    int fs_nbufs;			/* # bufs in cache */
    int fs_maxbufs;			/* hiwat for above */
    B fs_bufs;				/* buf cache */
    B fs_ichunk;			/* big inode read */
    int fs_ichunksize;			/* size of above */

    int fs_bsize;			/* buffer size */
    int fs_bshift;			/* log2 buffer size */
    int fs_bmask;			/* mask for buffer ops */

    int fs_ninodes;			/* # inodes in cache */
    int fs_maxinodes;			/* hiwat for above */
    I fs_inodes;			/* inode cache */

    ino_t fs_firstino;			/* first valid inumber */
    ino_t fs_ninos;			/* number of valid inumbers */

    struct toyparams fs_params;		/* fs-dependent parameters */
    struct toyops fs_ops;		/* fs-dependent entry points */

    char *fs_filsys;			/* fs-dependent representation */

    long fs_slop[10];			/* expansion */
};
typedef struct toyfs FS;

/*
 * struct toyuser is analogous to the kernel's
 * struct user.  it contains all toy globals.
 */
struct toyuser
{
    int u_inited;			/* flag - inited */
    ino_t u_curino;			/* start inode for rel path (namei) */
    char *u_errmsg;			/* last error message */

    short u_niblocks;			/* # indirect blocks (bmap) */
    daddr_t u_iblocks[8];		/* indirect blocks */

    short u_umask;			/* file mode creation mask */
    time_t u_time;			/* time for file creation */

    TOYDIR u_dent;			/* stratch dir entry */
    char u_compbuf[256];		/* scratch buf (namei) */
    int u_complen;			/* length of above (namei) */
    off_t u_slotoff;			/* offset of last comp (namei) */
    ino_t u_pino;			/* parent inode (namei) */
    char u_lastcomp;			/* flag - last comp reached (creat) */

    long u_slop[10];			/* expansion */
};
typedef struct toyuser USR;

# define TOY_INVAL	01
# define TOY_DIRTY	02
# define TOY_RONLY	04

# ifndef SECTORSHIFT
# define SECTORSHIFT	9
# endif  SECTORSHIFT

# ifndef SECTORSIZE
# define SECTORSIZE	(1<<SECTORSHIFT)
# endif  SECTORSIZE

# ifndef SECTORMASK
# define SECTORMASK	(SECTORSIZE-1)
# endif  SECTORMASK

# ifndef BUFSHIFT
# define BUFSHIFT	10
# endif  BUFSHIFT

# ifndef BUFSIZE
# define BUFSIZE	(1<<BUFSHIFT)
# endif  BUFSHIFT

# ifndef BUFMASK
# define BUFMASK	(BUFSIZE-1)
# endif  BUFMASK

# endif  __TOYFS__
