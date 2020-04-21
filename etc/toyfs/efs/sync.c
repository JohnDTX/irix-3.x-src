# include "toyfs.h"
# include "efs_toyfs.h"

extern USR U;

efs_sync(sp)
    register FS *sp;
{
    efs_bmsync(sp);
    efs_supersync(sp);
}

efs_supersync(sp)
    register FS *sp;
{
    extern B *toy_getblk();
    extern long efs_cksum();

    register B *bp;
    register struct efs_toyfs *esp;

    if( (sp->fs_flags & (TOY_DIRTY|TOY_INVAL)) != TOY_DIRTY )
	return;

    if( (bp = toy_getblk(sp, SUPERB)) == 0 )
	return;

    esp = (struct efs_toyfs *)sp->fs_filsys;
    bzero(bp->b_addr, BBSize);
    esp->filsys.fs_dirty = 0;
    esp->filsys.fs_checksum
	    = efs_cksum((unsigned short *)&esp->filsys
		    , (unsigned short *)(&esp->filsys.fs_checksum)
			    - (unsigned short *)&esp->filsys);
    *(struct efs *)bp->b_addr
	    = ((struct efs_toyfs *)sp->fs_filsys)->filsys;
    bp->b_flags |= TOY_DIRTY;
    toy_bwrite(sp, bp);
}

int
efs_bmsync(sp)
    register FS *sp;
{
    extern off_t lseek();

    register struct efs_toyfs *esp;

    esp = (struct efs_toyfs *)sp->fs_filsys;
    if( lseek(sp->fs_fd, (off_t)BITMAPOFF, 0) < 0
     || write(sp->fs_fd, esp->bbitmap, esp->bmsize) < 0 )
    {
	U.u_errmsg = "BIT MAP WRITE ERROR";
	return -1;
    }
    return 0;
}
