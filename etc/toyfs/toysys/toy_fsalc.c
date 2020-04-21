# include "toyfs.h"
# include "efs_toyfs.h"
# include "sys/stat.h"

extern USR U;

FS *
toy_fsalloc(devname, wflag, paramp)
    char *devname;
    int wflag;
    struct toyparams *paramp;
{
    extern off_t lseek();
    extern char *newstr();
    extern char *raw_dev(), *cook_dev();
    extern char *toy_malloc();

    struct stat stat1;
    register FS *sp;
    int supersize;

    supersize = paramp->fs_isbsize;
    if( supersize < SECTORSIZE )
	supersize = SECTORSIZE;
    if( wflag )
	wflag = 2;

    supersize += sizeof *sp;
    if( (sp = (FS *)toy_malloc(supersize)) == 0 )
	return 0;

    sp->fs_filsys = (char *)(sp + 1);
    sp->fs_flags = TOY_INVAL|TOY_RONLY;
    sp->fs_dev = -1;
    sp->fs_fd = -1;
    sp->fs_devname = 0;

    sp->fs_bshift = paramp->fs_bshift;
    sp->fs_bsize = 1<<sp->fs_bshift;
    sp->fs_bmask = sp->fs_bsize - 1;

    if( wflag )
	sp->fs_flags &= ~TOY_RONLY;

    toy_iinit(sp);
    toy_binit(sp);

    /* NOT raw_dev(devname) */
    if( (sp->fs_devname = newstr(devname)) == 0
     || (sp->fs_fd = open(sp->fs_devname, wflag)) < 0 )
    {
	if( sp->fs_devname != 0 )
	    free(sp->fs_devname);
	sp->fs_devname = newstr(devname);
	if( (sp->fs_fd = open(sp->fs_devname, wflag)) < 0 )
	{
	    U.u_errmsg = "can't open dev";
	    toy_fsfree(sp);
	    return 0;
	}
    }

    stat(cook_dev(devname), &stat1);
    sp->fs_dev = stat1.st_rdev;

    if( lseek(sp->fs_fd, (off_t)paramp->fs_sboff, 0) < 0
     || read(sp->fs_fd, sp->fs_filsys, paramp->fs_dsbsize) < 0 )
    {
	U.u_errmsg = "superblock read error";
	toy_fsfree(sp);
	return 0;
    }

    return sp;
}

toy_fsfree(sp)
    register FS *sp;
{
    if( sp == 0 )
	return;

    toy_sync(sp);
    toy_ikill(sp);
    toy_bkill(sp);

    close(sp->fs_fd);

    if( sp->fs_devname != 0 )
	free(sp->fs_devname);
    free((char *)sp);
}
