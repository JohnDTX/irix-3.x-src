# include "toyfs.h"
# include "efs_toyfs.h"

extern USR U;

efs_ifree(sp, inum)
    register FS *sp;
    register ino_t inum;
{
    register struct efs_toyfs *esp;

    esp = (struct efs_toyfs *)sp->fs_filsys;
    if( esp->nextfreeinum + esp->nfreeinums < EfsToyInums )
	esp->freeinums[esp->nextfreeinum + esp->nfreeinums++] = inum;
	
    sp->fs_flags |= TOY_DIRTY;
    esp->filsys.fs_tinode++;
}

ino_t
efs_ialloc(sp)
    register FS *sp;
{
    register struct efs_toyfs *esp;
    int i;
    register ino_t inum;

    esp = (struct efs_toyfs *)sp->fs_filsys;

    if( esp->nfreeinums <= 0 && efs_iscan(sp) <= 0 )
    {
	U.u_errmsg = "Out of dk inodes";
	return 0;
    }

    esp->filsys.fs_tinode--;
    --esp->nfreeinums;
    return esp->freeinums[esp->nextfreeinum++];
}

int
efs_iscan(sp)
    register FS *sp;
{
    extern I *efs_iread();

    register struct efs_toyfs *esp;
    register I *ip;
    register ino_t inum;

    esp = (struct efs_toyfs *)sp->fs_filsys;
    if( esp->nfreeinums <= 0 )
    {
	esp->nfreeinums = 0;
	esp->nextfreeinum = 0;
    }

    while( esp->iwindow < esp->maxinum
     && esp->nextfreeinum + esp->nfreeinums < EfsToyInums )
    {
	inum = esp->iwindow++;
	if( (ip = efs_iread(sp, inum)) == 0 )
	    continue;
	if( ip->i_imode == 0 )
	    esp->freeinums[esp->nextfreeinum + esp->nfreeinums++] = inum;
	toy_iput(ip);
    }

    return esp->nfreeinums;
}
