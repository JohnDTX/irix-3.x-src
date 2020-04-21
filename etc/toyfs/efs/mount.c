# include "toyfs.h"
# include "efs_toyfs.h"
# define RONLY
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
efs_mount(devname)
    char *devname;
{
    extern FS *toy_fsalloc();

    register FS *sp;
    register struct efs_toyfs *esp;

    if( (sp = toy_fsalloc(devname, 0, &efs_params)) == 0 )
	return 0;

    esp = (struct efs_toyfs *)sp->fs_filsys;
    if( esp->filsys.fs_magic != EFS_MAGIC )
    {
	U.u_errmsg = "invalid superblock magic number";
	toy_fsfree(sp);
	return 0;
    }
    esp->filsys.fs_ipcg = esp->filsys.fs_cgisize * EFS_INOPBB;

    sp->fs_ops = efs_rops;
    sp->fs_firstino = FIRSTINO;
    sp->fs_ninos = ((esp->filsys.fs_cgisize * esp->filsys.fs_ncg)
	    << BBShift) / sizeof (struct efs_dinode) - FIRSTINO;

    esp->bbitmap = 0;
    esp->brotor = 0;
    esp->irotor = 0;
    esp->maxinum = esp->filsys.fs_ncg * esp->filsys.fs_cgisize
						 << EFS_INOPBBSHIFT;

    sp->fs_flags &= ~TOY_INVAL;
    return sp;
}
