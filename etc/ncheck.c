# define NCHECK_CHECKLIST
# include "toyfs.h"
# include "sys/dir.h"

# include "stdio.h"

extern USR U;

# define lengthof(x)	(sizeof(x)/sizeof*(x))

# define DEBUG ncheck_debug
# ifdef DEBUG
# define ifdebug(x)	(DEBUG?(x):0)
# define dprintf(x)	(DEBUG?printf x:0)
# else  DEBUG
# define ifdebug(x)
# define dprintf(x)
# endif DEBUG
short ncheck_debug = 0;

char dot[DIRSIZ] = ".";
char dotdot[DIRSIZ] = "..";

# define dotname(dp)	(*(long *)(dp)->d_name == *(long *)dot \
			|| *(long *)(dp)->d_name == *(long *)dotdot)
# define MAXLINE	256
# include <mntent.h>
char *checklist = MNTTAB;
char line[MAXLINE];

# define MAXICHUNK 32
int build_chunk = 0;
int attach_chunk = 0;
int print_chunk = 0;

# define XINOSHIFT	7
# define XINOMASK	(NXINOHASH-1)
# define NXINOHASH	(1<<XINOSHIFT)
# define xinohash(x)	((x)&XINOMASK)
struct xino
{
    struct xino *chain;
    ino_t ino;
};
struct xino *xino_buckets[NXINOHASH];

# define INOBITS	(BITSPERBYTE*sizeof (ino_t))

# define DIRINOSHIFT	10
# define NDIRINOHASH	(1<<DIRINOSHIFT)
# define DIRINOMASK	(NDIRINOHASH-1)
# define dirinohash(x)	((x)&DIRINOMASK)

struct dirino
{
    struct dirino *chain;
    struct dirino *sortchain;
    ino_t pino;
    struct direct dent;
};
struct dirino *dir_buckets[NDIRINOHASH];

struct dirbucket
{
    struct dirino *head, *tail;
};
struct dirbucket sortq;
int n_dirs;
int n_inos;

char aflag = 0;
char iflag = 0;
char sflag = 0;

char *progname = "ncheck";
char *usage = "usage:  ncheck [-a] [-i numbers] [-s] devices";

main(argc, argv)
    int argc;
    char **argv;
{
    extern char *skipcnum();

    char argbotch;
    register char *ap, *cp;
    long x;
    FILE *Cfile;

    argbotch = 0;
    argc--; argv++;
    while( argc > 0 && *(ap = *argv) == '-' )
    {
	argc--; argv++; ap++;
	while( *ap != 000 )
	switch( *ap++ )
	{
	case 'D':
	    ncheck_debug++;
	    break;

	case 'a':
	    aflag++;
	    break;

	case 'i':
	    while( argc > 0 && *skipcnum(*argv, 10, &x) == 000 )
	    {
		new_xino((ino_t)x);
		iflag = 1;
		argc--; argv++;
	    }
	    break;

	case 's':
	    sflag++;
	    break;

	default:
	    errwarn("unknown flag %c", ap[-1]);
	    argbotch--;
	    break;
	}
    }

    if( argbotch )
	errexit(usage);

    if( argc > 0 )
    {
	while( --argc >= 0 )
	    ncheck(*argv++);
	exit(0);
    }
# ifdef NCHECK_CHECKLIST
    {
	register struct mntent *mntp;

	if( (Cfile = setmntent(checklist, "r")) == 0 )
	    errexit("can't open %s", checklist);
	while( (mntp = getmntent(Cfile)) != 0 ) {
	    char *ckname, *strrchr();

	    if (strcmp(mntp->mnt_type, MNTTYPE_EFS)
		&& strcmp(mntp->mnt_type, MNTTYPE_BELL))
		continue;
	    ckname = hasmntopt(mntp, MNTOPT_RAW);
	    if (ckname && (ckname = strrchr(ckname, '=')))
		ckname++;
	    else
		ckname = mntp->mnt_fsname;
	    ncheck(ckname);
	}
	endmntent(Cfile);
    }
# else  NCHECK_CHECKLIST
    errexit(usage);
# endif NCHECK_CHECKLIST

    exit(0);
}

ncheck(name)
    char *name;
{
    extern FS *toy_ropenfs();

    register I *ip;
    register FS *sp;

    printf("%s:\n", name);
    if( (sp = toy_ropenfs(name)) == 0 )
    {
	toy_errwarn("can't open file system %s", name);
	return;
    }

    dprintf((" INODE SCAN:\n"));
    /* try to read inodes in large chunks */
    if( build_chunk == 0 )
	build_chunk = MAXICHUNK;
    toy_ichunkinit(sp, build_chunk);
    build_xlist(sp);

    ifdebug(dump_sortchain());
    ifdebug(dump_xlist());

    dprintf((" ATTACH SCAN:\n"));
    /* recompute chunk if dirs are sparse */
    if( attach_chunk == 0 )
	attach_chunk = ((long)MAXICHUNK*n_dirs*2)/(n_inos+1);
    if( attach_chunk > MAXICHUNK )
	attach_chunk = MAXICHUNK;
    toy_ichunkinit(sp, attach_chunk);
    attach_xlist(sp);

    dprintf((" PRINT SCAN:\n"));
    if( print_chunk == 0 )
	print_chunk = attach_chunk;
    toy_ichunkinit(sp, print_chunk);
    print_xlist(sp);

    toy_closefs(sp);
}

build_xlist(sp)
    register FS *sp;
{
    extern I *toy_iget();

    register ino_t curino, lastino;
    register I *ip;
    int ftype;

    n_inos = sp->fs_ninos;
    lastino = sp->fs_firstino + sp->fs_ninos;
    for( curino = sp->fs_firstino; curino < lastino; curino++ )
    {
	if( (ip = toy_iget(sp, curino)) == 0 )
	    continue;
	ftype = ip->i_imode & IFMT;
	if( sflag )
	    if( ftype == IFCHR || ftype == IFBLK || ip->i_imode&(ISUID|ISGID) )
		new_xino(curino);
	if( ftype == IFDIR )
	{
	    n_dirs++;
	    new_compino(curino);
	}
	toy_iput(ip);
    }
}

attach_xlist(sp)
    register FS *sp;
{
    register struct dirino *cp;

    for( cp = sortq.head; cp != 0; cp = cp->sortchain )
	attach_scan(sp, cp->dent.d_ino);
}

attach_scan(sp, ino)
    FS *sp;
    ino_t ino;
{
    extern struct dirino *dirlookup();

    extern I *toy_iget();
    extern TOYIOB *toy_opendir();
    extern TOYDIR *toy_readdir();

    register I *ip;
    register TOYIOB *Dirp;
    register TOYDIR *dp;
    register struct dirino *cp;

    if( (ip = toy_iget(sp, ino)) == 0 )
	return;
    if( (Dirp = toy_opendir(ip)) == 0 )
    {
	toy_iput(ip);
	return;
    }
    while( (dp = toy_readdir(Dirp)) != 0 )
    {
	if( dp->d_ino == 0 )
	    continue;
	if( dotname(dp) )
	    continue;
	if( (cp = dirlookup(dp->d_ino)) == 0 )
	    continue;
	strncpy(cp->dent.d_name, dp->d_name, DIRSIZ);
	cp->pino = ino;
    }
    toy_closedir(Dirp);
    toy_iput(ip);
}

print_xlist(sp)
    register FS *sp;
{
    register struct dirino *cp;

    for( cp = sortq.head; cp != 0; cp = cp->sortchain )
	print_scan(sp, cp->dent.d_ino);
}

print_scan(sp, ino)
    register FS *sp;
    ino_t ino;
{

    extern struct dirino *dirlookup();

    extern I *toy_iget();
    extern TOYIOB *toy_opendir();
    extern TOYDIR *toy_readdir();

    register I *ip;
    register TOYIOB *Dirp;
    register TOYDIR *dp;

    if( (ip = toy_iget(sp, ino)) == 0 )
	return;
    if( (Dirp = toy_opendir(ip)) == 0 )
    {
	toy_iput(ip);
	return;
    }
    while( (dp = toy_readdir(Dirp)) != 0 )
    {
	if( dp->d_ino == 0 )
	    continue;
	if( !aflag && dotname(dp) )
	    continue;
	if( sflag || iflag )
	{
	    if( !xlookup(dp->d_ino) )
		continue;
	}
	printf("%u\t", dp->d_ino);
	pdirname(ino);
	if( dirlookup(dp->d_ino) != 0 )
	    printf("/%.14s/.\n", dp->d_name);
	else
	    printf("/%.14s\n", dp->d_name);
    }
    toy_closedir(Dirp);
    toy_iput(ip);
}

new_xino(x)
    ino_t x;
{
    extern char *malloc();

    register struct xino **xhp, *xp;

    if( (xp = (struct xino *)malloc(sizeof *xp)) == 0 )
    {
	errwarn("no core for numbers");
	return;
    }
    xp->ino = x;

    xhp = xino_buckets + xinohash(x);
    xp->chain = *xhp;
    *xhp = xp;
}

new_compino(x)
    ino_t x;
{
    extern char *malloc();

    register struct dirino **chp;
    register struct dirino *cp;

    if( (cp = (struct dirino *)malloc(sizeof *cp)) == 0 )
    {
	errwarn("no core for directories");
	return;
    }
    cp->dent.d_ino = x;

    chp = dir_buckets + dirinohash(x);
    cp->chain = *chp;
    *chp = cp;

    cp->sortchain = 0;
    if( sortq.head == 0 )
	sortq.head = cp;
    if( sortq.tail == 0 )
	sortq.tail = cp;
    else
	sortq.tail->sortchain = cp;
    sortq.tail = cp;
}

struct dirino *
dirlookup(x)
   ino_t x;
{
    register struct dirino **chp;
    register struct dirino *cp;

    chp = dir_buckets + dirinohash(x);
    for( cp = *chp; cp != 0; cp = cp->chain )
	if( cp->dent.d_ino == x )
	    return cp;
    return 0;
}

xlookup(x)
   ino_t x;
{
    register struct xino **xhp, *xp;

    xhp = xino_buckets + xinohash(x);
    for( xp = *xhp; xp != 0; xp = xp->chain )
	if( xp->ino == x )
	    return 1;
    return 0;
}

pdirname(ino)
    ino_t ino;
{
    pname(ino, 0);
}

pname(ino, level)
    ino_t ino;
    int level;
{
    register struct dirino *cp;

    if( ino == ROOTINO )
	return;

    if( (cp = dirlookup(ino)) == 0 )
    {
	printf("???");
	return;
    }
    if( level > 10 )
    {
	printf("...");
	return;
    }

    pname(cp->pino, ++level);
    printf("/%.14s", cp->dent.d_name);
}

errwarn(a)
    struct { int x[5]; } a;
{
    fprintf(stderr, a);
    fprintf(stderr, "\n");
    fflush(stderr);
}

errexit(a)
    struct { int x[5]; } a;
{
    errwarn(a);
    exit(-1);
}

dump_xlist()
{
    register struct dirino **chp;
    register struct dirino *cp;

    register int i;

    printf("by buckets:\n");
    for( chp = dir_buckets , i = lengthof(dir_buckets); --i >= 0; chp++ )
	for( cp = *chp; cp != 0; cp = cp->chain )
	    dumpdirino(cp);
}

dump_sortchain()
{
    register struct dirino *cp;

    printf("by sortchain:\n");
    for( cp = sortq.head; cp != 0; cp = cp->sortchain )
	dumpdirino(cp);
}

dumpdirino(cp)
    register struct dirino *cp;
{
    printf("%u: %u \"%.14s\"\n",
	    cp->pino, cp->dent.d_ino, cp->dent.d_name);
}
