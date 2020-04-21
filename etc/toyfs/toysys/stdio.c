# include "toyfs.h"


extern USR U;

TOYIOB *
toy_fopen(ip)
    register I *ip;
{
    extern F *toy_openi();
    extern char *toy_malloc();

    register TOYIOB *iobp;

    if( (iobp = (TOYIOB *)toy_malloc(sizeof *iobp)) == 0
     || (iobp->fd = toy_openi(ip)) == 0
     || (iobp->base = toy_malloc(iobp->basesize = 1024)) == 0 )
    {
	toy_fclose(iobp);
	return 0;
    }

    iobp->nleft = 0;
    iobp->nused = 0;
    return iobp;
}

toy_fclose(iobp)
    register TOYIOB *iobp;
{
    if( iobp == 0 )
	return -1;
    if( iobp->base != 0 )
	free(iobp->base);
    if( iobp->fd != 0 )
	toy_close(iobp->fd);
    free((char *)iobp);
}

int
toy_ffill(iobp)
    register TOYIOB *iobp;
{
    if( (iobp->nleft = toy_read(iobp->fd,iobp->base,iobp->basesize)) <= 0 )
	return -1;
    iobp->ptr = iobp->base;
    iobp->nleft--;
    return *iobp->ptr++;
}

int
toy_fflush(iobp)
    register TOYIOB *iobp;
{
    if( iobp->nused <= 0 )
	return 0;

    if( toy_write(iobp->fd,iobp->base,iobp->nused) != iobp->nused )
    {
	iobp->nused = 0;
	iobp->ptr = iobp->base;
	return -1;
    }

    iobp->nused = 0;
    iobp->ptr = iobp->base;
    return iobp->nused;
}

off_t
toy_fseek(iobp, offset)
    register TOYIOB *iobp;
    off_t offset;
{
    register F *fp;
    register int bufcount;
    register off_t bufoff;

    fp = iobp->fd;

    if( iobp->nleft > 0 )
    {
	iobp->nleft += (iobp->ptr - iobp->base);
	iobp->ptr = iobp->base;
	bufoff = fp->f_offset - iobp->nleft;
	if( bufoff <= offset && offset < fp->f_offset )
	{
	    bufcount = offset - bufoff;
	    iobp->nleft -= bufcount;
	    iobp->ptr += bufcount;
	    return fp->f_offset - iobp->nleft;
	}
    }

    if( iobp->nused > 0 )
	toy_fflush(iobp);

    iobp->nused = 0;
    iobp->nleft = 0;
    toy_seek(fp, offset);
    return offset;
}

off_t
toy_ftell(iobp)
    register TOYIOB *iobp;
{
    return iobp->fd->f_offset - (iobp->nleft > 0 ? iobp->nleft : 0);
}
