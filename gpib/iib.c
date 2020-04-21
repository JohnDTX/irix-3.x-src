#
/*
 * iib.c --
 * initialize ib driver from IBTAB file.
 */
# include "sys/param.h"
# include "sys/types.h"
# include "sys/sysmacros.h"
# include "sys/stat.h"
# include "sys/ioctl.h"


# include "ibtab.h"
# include "sys/ib_ioctl.h"


# include "stdio.h"
# include "ctype.h"


char *progname = "iib";

char argbotch;
char vflg = 0;
char rflg = 0;
char iflg = 0;
char *usage = "usage:  %s [-ifrv ...]";

# define DEBUG main_debug
# ifdef DEBUG
# define dprintf(x)	if(DEBUG)errwarn x
int DEBUG = 0;
# else  DEBUG
# define dprintf(x)
# endif DEBUG

main(rgc,rgv)
    int rgc;
    char **rgv;
{
    register char *ap;

    /*get progname for error messages*/
    if( --rgc >= 0 )
	progname = *rgv++;

    /*get flags*/
    argbotch = 0;
    while( rgc > 0 && *(ap = *rgv)  == '-' )
    {
	rgc--; rgv++; ap++;
	while( *ap != 000 )
	switch(*ap++)
	{
# ifdef DEBUG
	case 'd':
	    DEBUG = 1;
	    break;
# endif DEBUG

	case 'f':
	    if( --rgc < 0 )
	    {
		argbotch++;
		errwarn("missing -f {ibtab}");
		break;
	    }
	    setibfile(*rgv++);
	    break;

	case 'i':
	    iflg++;
	    break;

	case 'r':
	    if( getuid() != 0 )
	    {
		errwarn("-r permission denied");
		break;
	    }
	    rflg++;
	    break;

	case 'v':
	    vflg++;
	    break;

	default:
	    errwarn("illegal flag -%c",ap[-1]);
	    argbotch++;
	    break;
	}
    }
    if( rgc > 0 )
    {
	errwarn("arg cnt");
	argbotch++;
    }
    if( argbotch )
	errexit(usage,progname);

    doit();

    exit(0);
}

struct sstash
{
    struct stat sstat;
    int sfd;
    struct sstash *sptr;
};
struct sstash *xhd = 0;

int recall(sp)
    register struct stat *sp;
{
    register struct sstash *xp;

    for( xp = xhd; xp != 0; xp = xp->sptr )
	if( xp->sstat.st_ino == sp->st_ino
	 && xp->sstat.st_dev == sp->st_dev )
	    return xp->sfd;
    return -1;
}

stash(sp,sfd)
    register struct stat *sp;
    int sfd;
{
    register struct sstash *xp;

    if( (xp = (struct sstash*)malloc(sizeof*xp)) == 0 )
	errexit("out of core");
    xp->sstat = *sp;
    xp->sfd = sfd;

    xp->sptr = xhd;
    xhd = xp;
}

int unstash(_fd)
    int *_fd;
{
    register struct sstash *xp;
    if( (xp = xhd) == 0 )
	return -1;
    xhd = xp->sptr;
    *_fd = xp->sfd;
    free(xp);
    return 0;
}

doit()
{
    extern struct ibtab *getibent();

    register struct ibtab *ip;
    int flags,V;
    static struct sgnode sg1;
    static struct stat stat1;
    int fd;

    if( setibent() < 0 )
	scerrexit("bad ibtab file");

    /*loop over entries*/
    while( (ip = getibent()) != 0 )
    {
	if( ip->ibt_node < 0 )
	{
	    errwarn("skipping bad entry (%s)",ip->ibt_file);
	    continue;
	}
	if( ibnflags(ip->ibt_flags,&flags) < 0 )
	{
	    errwarn("bad flags (%s), skipping entry (%s)"
		    ,ip->ibt_flags,ip->ibt_file);
	    continue;
	}
	if( vflg )
	{
	    printibent(ip,flags);
	    printf("\n");
	    fflush(stdout);
	}
	if( iflg )
	{
	    printibent(ip,flags);
	    printf("?");
	    fflush(stdout);
	    if( !okay() )
		continue;
	}
	sg1.slotno = ip->ibt_node;
	sg1.node.n_flags = flags;
	sg1.node.n_tag1 = ip->ibt_tag;
	sg1.node.n_ppr = ip->ibt_ppr;
# ifdef notdef
	sg1.node.n_talkresp = ip->ibt_T;
	sg1.node.n_lstnresp = ip->ibt_L;
	sg1.node.n_tctlresp = ip->ibt_C;
	sg1.node.n_erroresp = ip->ibt_E;
	sg1.node.n_idleresp = ip->ibt_I;
# endif notdef

	/*
	 *remember which control files we've seen
	 *for possible IBIOINIT / IBIOSTART.
	 */
	if( stat(ip->ibt_cfile,&stat1) < 0 )
	{
	    scerrwarn("can't stat %s (for %s)"
		    ,ip->ibt_cfile,ip->ibt_file);
	    continue;
	}
	if( (fd = recall(&stat1)) < 0 )
	{
	    dprintf(("opening %s",ip->ibt_cfile));
	    if( (fd = open(ip->ibt_cfile,0)) < 0 )
	    {
		scerrwarn("can't open %s (for %s)"
			,ip->ibt_cfile,ip->ibt_file);
		continue;
	    }
	    if( rflg )
	    {
		dprintf(("initing %d",fd));
		if( ioctl(fd,IBIOINIT,0) < 0 )
		{
		    scerrwarn("ioctl(...INIT) failed (for %s)"
			    ,ip->ibt_file);
		    continue;
		}
	    }
	    stash(&stat1,fd);
	}
	dprintf(("setting %d using %d",sg1.slotno,fd));
	if( ioctl(fd,IBIOSETNODE,&sg1) < 0 )
	{
	    scerrwarn("ioctl(...SETNODE) failed (for %s)"
		    ,ip->ibt_file);
	}
    }

    while( unstash(&fd) == 0 )
    {
	V = 1;
	dprintf(("starting %d",fd));
	if( ioctl(fd,IBIOSTART,&V) < 0 )
	{
	    scerrwarn("ioctl(...START) failed");
	}
	close(fd);
    }
    endibent();
}

printhdr()
{
    printf("%-15s %-15s %4s %6s %3s %4s\n"
	    ,"File"
	    ,"Cfile"
	    ,"Node"
	    ,"Flags"
	    ,"Tag"
	    ,"Ppr"
	    ,""
		);
}

printibent(ip,flags)
    register struct ibtab *ip;
    int flags;
{
    static int didhd;
    if( !didhd )
    {
	printhdr();
	didhd++;
    }
    printf("%-15s %-15s %4d 0x%04x %3d 0x%02x"
	    ,ip->ibt_file
	    ,ip->ibt_cfile
	    ,ip->ibt_node
	    ,flags
	    ,ip->ibt_tag
	    ,ip->ibt_ppr
			);
    fflush(stdout);
}

/*
 * okay() --
 * return TRUE iff input looks like "yes" .
 */
okay()
{
    char junk[100];
    register char *ap;
    fgets(junk,sizeof junk,stdin);
    for( ap = junk; isspace(*ap); )
	ap++;
    return *ap == 'y' || *ap == 'Y';
}
