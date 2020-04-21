# include "toyfs.h"

# define MAXLNKDEPTH 8

extern USR U;

# define EOS		'\0'		/* end of string chr */
# define SLASHC		'/'		/* component separator chr */
# define MAXCOMP	100

/*
 * control structure for buffered input.
 * cf. "stdio.h" .
 */
typedef struct
{
	unsigned char *ptr;
	short resid;		/*# bytes left in buf*/
	char *base;
	off_t offset;		/*reading offset*/
	char b[100];
	unsigned char peekc;
	F *fd;
} STRBUF;

# define namei_getc(x)	( --(x)->resid<0?lchar_fill(x):*(x)->ptr++ )


extern TOYDIR *toy_readdir();

short toy_namei_nofollow = 0;

I *
toy_namei(sp, name, slend)
    FS *sp;
    char *name;
    int slend;
{
    extern I *toy_iget();
    extern TOYIOB *toy_opendir();
    extern F *toy_openi();

    register ino_t curino;
    register I *ip;
    I *pip;
    register unsigned char c;
    TOYIOB *iobp;

    STRBUF nstack[MAXLNKDEPTH+1];
    register STRBUF *np;
    register int ndepth, loopcnt;

    U.u_errmsg = 0;

    loopcnt = MAXLNKDEPTH;
    ndepth = 0;
    np = nstack + MAXLNKDEPTH;

    np->resid = 0x7FFF;
    np->ptr = (unsigned char *)name;

    c = namei_getc(np);

    if( c == SLASHC )
    {
	curino = ROOTINO;
	while( c == SLASHC )
	    c = namei_getc(np);
    }
    else
    {
	curino = U.u_curino;
    }

    if( (ip = toy_iget(sp, curino)) == NULL )
	goto err_out;

    for( ;; )
    {
	if( U.u_errmsg != 0 )
	    goto err_putout;

	if( c == EOS )
	{
	    if( ndepth == 0 )
		return ip;

	    ndepth--;
	    lchar_pop(np++);
	    c = np->peekc;
	    continue;
	}

	/*
	 * There is another component.
	 * Gather it into the comp buffer.
	 * Discard extra SLASHC's .
	 * Discard chrs in excess of DIRSIZ .
	 */
	{
	    register char *cp;

	    cp = U.u_compbuf;
	    while( c != SLASHC && c != EOS )
	    {
		if( cp < U.u_compbuf + sizeof U.u_compbuf - 1 )
		    *cp++ = c;
		c = namei_getc(np);
	    }
	    *cp++ = EOS;
	    while( c == SLASHC )
		c = namei_getc(np);
	}

	/*
	 * Pop back through any depleted levels to get the next real
	 * chr of pathname.
	 */
	while( ndepth > 0 && c == EOS )
	{
	    ndepth--;
	    lchar_pop(np++);
	    c = np->peekc;
	}

	U.u_pino = curino;

	/*
	 * Scan the directory.
	 */
	{
	    register TOYDIR *dep;

	    U.u_slotoff = -1;
	    if( (iobp = toy_opendir(ip)) == NULL )
		goto err_putout;

	    while( (dep = toy_readdir(iobp)) != 0 )
	    {
		if( (curino = dep->d_ino) == 0 )
		{
		    if( U.u_slotoff < 0 )
			U.u_slotoff = dep->d_offset;
		    continue;
		}
		if( strncmp(U.u_compbuf, dep->d_name, dep->d_len) == 0 )
		{
		    U.u_complen = dep->d_len;
		    U.u_slotoff = dep->d_offset;
		    break;
		}
	    }

	    toy_closedir(iobp);

	    if( dep == NULL )
		goto not_found;
	}

found:
	/*
	 * Move on to the next inode and component.
	 * The next inode might be a relative symlink, so retain
	 * the current inode until this possibility is checked.
	 */
	pip = ip;

	if( (ip = toy_iget(sp, curino)) == NULL )
	{
	    ip = pip;
	    goto err_putout;
	}

	/*
	 * If it's not a symlink, or if on the final component and
	 * symlinks are not being followed at the end, just drop
	 * the previous inode and continue.
	 */
	if( (ip->i_imode&IFMT) != IFLNK
	 || toy_namei_nofollow
	 || (!slend && c == EOS) )
	{
	    toy_iput(pip);
	    continue;
	}

	/*
	 * Following a symbolic link.
	 * Avoid infinite loops in a primitive way.
	 */
	if( --loopcnt < 0 )
	{
	    U.u_errmsg = "Symbolic links too deep";
	    toy_iput(ip);
	    ip = pip;
	    goto err_putout;
	}

	/*
	 * Push the current level of pathname input.
	 * Set up to read the symlink as part of the pathname.
	 */
	np->peekc = c;
	ndepth++;
	np--;

	np->resid = 0; np->offset = 0;
	if( (np->fd = toy_openi(ip)) == NULL )
	    goto err_putout;
	toy_iput(ip);	/* ... so that toy_close() will iput discard */

	/*
	 * Determine where to re-start.
	 * Resume tracing the (extended) pathname.
	 */
	c = namei_getc(np);

	if( c == SLASHC )
	{
	    toy_iput(pip);

	    curino = ROOTINO;
	    while( c == SLASHC )
		c = namei_getc(np);

	    if( (ip = toy_iget(sp, curino)) == NULL )
		goto not_found;
	}
	else
	{
	    ip = pip;
	}
    }

not_found:
    {
	toy_closedir(iobp);
	if( U.u_errmsg != 0 )
	    goto err_putout;

	U.u_errmsg = "No such file or directory";
    }

err_putout:
    /*
     * Common code for error return from namei().
     * Release resources and return NULL .
     */
    toy_iput(ip);

err_out:
    while( --ndepth >= 0 )
	lchar_pop(np++);

    U.u_lastcomp = c == EOS;
    return NULL;
}

/*
 * lchar_fill() --
 * pathname fill routine for symlinks.
 */
int
lchar_fill(np)
    register STRBUF *np;
{
    np->base = np->b;
    if( (np->resid = toy_read(np->fd, np->base, sizeof np->b)) >= 0 )
    {
	np->offset += np->resid;

	/* if there is room put trailing EOS at end. */
	if( np->resid < sizeof np->b )
	{
	    np->offset ++;
	    np->ptr[np->resid++] = EOS;
	}
	np->ptr = (unsigned char *)np->base;
    }

    return --np->resid < 0 ? EOS : *np->ptr++ ;
}

lchar_pop(np)
    register STRBUF *np;
{
    register I *ip;

    if( np->fd != NULL )
    {
	toy_close(np->fd);
	np->fd = NULL;
    }
}
