# include "toyfs.h"
# include "bell_toyfs.h"

# include "sys/dir.h"
extern USR U;

/*
 * put it in the given place.
 */
int
bell_writedir(iobp, dep)
    register TOYIOB *iobp;
    register TOYDIR *dep;
{
    register struct bell_toyinode *dip;
    struct direct e1;

    dip = (struct bell_toyinode *)iobp->fd->f_ip->i_dinode;
    toy_fflush(iobp);

    toy_seek(iobp->fd,
	    dep->d_offset >= 0 ? dep->d_offset : iobp->fd->f_ip->i_isize);
    e1.d_ino = dep->d_ino;
    strncpy(e1.d_name, dep->d_name, DIRSIZ);
    return toy_write(iobp->fd, (char *)&e1, sizeof e1);
}
