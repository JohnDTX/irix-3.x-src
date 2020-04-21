# include "toyfs.h"
# include "efs_toyfs.h"

extern USR U;

int
efs_umount(sp)
    register FS *sp;
{
    register struct efs_toyfs *esp;
    register char *bbitmap;

    if( sp == 0 )
	return -1;

    esp = (struct efs_toyfs *)sp->fs_filsys;
    bbitmap = esp->bbitmap;

    toy_fsfree(sp);

    if( bbitmap != 0 )
	free(bbitmap);

    return 0;
}
