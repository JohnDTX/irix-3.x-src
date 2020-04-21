# include "toyfs.h"
# include "bell_toyfs.h"

extern USR U;

int
bell_umount(sp)
    register FS *sp;
{
    if( sp == 0 )
	return -1;

    toy_fsfree(sp);
    return 0;
}
