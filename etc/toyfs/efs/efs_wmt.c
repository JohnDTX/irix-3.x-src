# include "toyfs.h"
# include "efs_toyfs.h"
# undef  RONLY
# include "toyops.h"
FUNCDEFS(efs);
DECL(efs);

extern struct toyparams efs_params;

# ifndef FIRSTINO
# define FIRSTINO	((ino_t)1)
# endif  FIRSTINO

# ifndef ROOTINO
# define ROOTINO	((ino_t)2)
# endif  ROOTINO
extern USR U;

FS *
efs_wmount(devname)
    char *devname;
{
    extern FS *toy_fsalloc();

    register FS *sp;
    register struct efs_toyfs *esp;

    if( (sp = toy_fsalloc(devname, 1, &efs_params)) == 0 )
	return 0;

    esp = (struct efs_toyfs *)sp->fs_filsys;
    if( esp->filsys.fs_magic != EFS_MAGIC )
    {
	U.u_errmsg = "invalid superblock magic number";
	toy_fsfree(sp);
	return 0;
    }
    esp->filsys.fs_ipcg = esp->filsys.fs_cgisize * EFS_INOPBB;

    sp->fs_ops = efs_wops;
    sp->fs_firstino = FIRSTINO;
    sp->fs_ninos = ((esp->filsys.fs_cgisize * esp->filsys.fs_ncg)
	    << BBShift) / sizeof (struct efs_dinode) - FIRSTINO;

    esp->bbitmap = 0;
    esp->brotor = 0;
    esp->irotor = 0;
    esp->maxinum = esp->filsys.fs_ncg * esp->filsys.fs_cgisize
						 << EFS_INOPBBSHIFT;
    /*
     * if the file system will be written, 
     * get the block and inode bitmaps.
     */
    if( efs_bminit(sp) < 0 || efs_iminit(sp) < 0 )
    {
	efs_umount(sp);
	return 0;
    }
    esp->filsys.fs_dirty = 1;

    sp->fs_flags &= ~TOY_INVAL;
    return sp;
}

int
efs_bminit(sp)
    register FS *sp;
{
    extern off_t lseek();
    extern char *toy_malloc();

    register int mapsize;
    register struct efs_toyfs *esp;

    esp = (struct efs_toyfs *)sp->fs_filsys;

    esp->nbbitmap = esp->filsys.fs_bmsize*BITSPERBYTE;
    mapsize = (esp->filsys.fs_bmsize + BBSize - 1) & ~BBMask;
    esp->bmsize = mapsize;
    if( (esp->bbitmap = toy_malloc(mapsize)) == 0 )
	return -1;
    if( lseek(sp->fs_fd, (off_t)BITMAPOFF, 0) < 0
     || read(sp->fs_fd, esp->bbitmap, mapsize) < 0 )
    {
	U.u_errmsg = "BIT MAP READ ERROR";
	return -1;
    }

    return 0;
}

int
efs_iminit(sp)
    register FS *sp;
{
    register struct efs_toyfs *esp;

    esp = (struct efs_toyfs *)sp->fs_filsys;
    esp->iwindow = ROOTINO;
    esp->nfreeinums = 0;
    return 0;
}
