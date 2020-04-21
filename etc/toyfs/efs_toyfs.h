# include "bitmap.h"

# define BBShift		BBSHIFT
# define BBSize			BBSIZE
# define BBMask			(BBSize-1)
# define ExtsPerInode		EFS_DIRECTEXTENTS

# define ExtShift		3	/* lg sizeof (struct extent *) */
# define ExtsPerBBShift		(BBShift-ExtShift)
# define ExtsPerBB		(1<<ExtsPerBBShift)
# define ExtsPerBBMask		(ExtsPerBB-1)
# define InodesPerBB		EFS_INOPBB
# define InodesPerBBShift	EFS_INOPBBSHIFT

# define MaxExtLength		255

# define ISQUISHED		ISVTX

# define di_nx			di_numextents
# define di_xp			di_u.di_extents

# define EfsToyInums		20

struct efs_toyfs
{
    struct efs filsys;
    char *bbitmap;
    long nbbitmap;
    int bmsize;
    daddr_t brotor;
    ino_t irotor;
    ino_t freeinums[EfsToyInums];
    short nextfreeinum;
    short nfreeinums;
    ino_t iwindow;
    ino_t maxinum;
};
struct efs_toyinode
{
    struct efs_dinode inode;
};
