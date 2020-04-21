# include "toyfs.h"
# include "bell_toyfs.h"
# define RONLY
# include "toyops.h"
FUNCDEFS(bell);
DECL(bell);

extern struct toyparams bell_params;

# ifndef FIRSTINO
# define FIRSTINO	((ino_t)1)
# endif  FIRSTINO

# ifndef ROOTINO
# define ROOTINO	((ino_t)2)
# endif  ROOTINO
extern USR U;

FS *
bell_mount(devname)
    char *devname;
{
    extern FS *toy_fsalloc();

    register FS *sp;
    register struct bell_toyfs *esp;

    if( (sp = toy_fsalloc(devname, 0, &bell_params)) == 0 )
	return 0;

    esp = (struct bell_toyfs *)(sp->fs_filsys);
    if( esp->filsys.s_magic != FsMAGIC )
    {
	U.u_errmsg = "invalid superblock magic number";
	toy_fsfree(sp);
	return 0;
    }

    sp->fs_firstino = FIRSTINO;
    sp->fs_ninos = ((esp->filsys.s_isize - FsITOO(&esp->filsys, FIRSTINO))
	    << FSBShift) / sizeof (struct dinode);
    sp->fs_ops = bell_rops;

    sp->fs_flags &= ~TOY_INVAL;
    return sp;
}
