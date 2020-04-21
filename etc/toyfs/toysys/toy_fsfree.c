# include "toyfs.h"

extern USR U;

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
