#
/*
 * toyls --
 *	toyls [-lrv] device [file ...]"
 */
# include "toyfs.h"

# include "sys/stat.h"

# include "stdio.h"

FS *myfs;
char Lflag = 0;
char lflag = 1;
char rflag = 0;
char vflag = 0;
char *device = 0;

char catflag = 0;
char lsflag = 0;
char mvflag = 0;
char rmflag = 0;
char *cmdname = 0;
int (*toyfunc)();
extern int toycat(), toyls(), toycreat(), toyrm();

char *usage = "usage:  %s [-lrv] device [file ...]";

main(rgc, rgv)
    int rgc;
    char **rgv;
{
    extern char *basename();

    register char *ap;
    char argbotch;

    if( --rgc < 0 )
	errexit("Very funny");
    cmdname = basename(*rgv++);
    if( strcmp(cmdname, "toycat") == 0 )
    {
	toyfunc = toycat;
	catflag = 1;
    }
    else
    if( strcmp(cmdname, "toycreat") == 0 )
    {
	toyfunc = toycreat;
	mvflag = 1;
    }
    else
    if( strcmp(cmdname, "toyrm") == 0 )
    {
	toyfunc = toyrm;
	rmflag = 1;
    }
    else
    {
	toyfunc = toyls;
	lsflag = 1;
    }
    
    argbotch = 0;

    while( rgc > 0 )
    {
	ap = *rgv;
	if( *ap != '-' )
	{
	    if( device == 0 )
		device = ap;
	    else
		break;
	    rgc--; rgv++;
	    continue;
	}

	rgc--; rgv++; ap++;
	while( *ap != 000 )
	switch(*ap++)
	{
	case 'H':
	    Lflag = 0;
	    break;

	case 'L':
	    Lflag = 1;
	    break;

	case 'l':
	    lflag++;
	    break;

	case 'r':
	    rflag++;
	    break;

	case 'v':
	    vflag++;
	    break;

	default:
	    errwarn("unknown flag %c", ap[-1]);
	    argbotch--;
	    break;
	}
    }

    if( argbotch )
	errexit(usage, cmdname);

    toy_init();

    if( opentoy(device) < 0 )
	toy_errexit("toy open botch");
 
    while( --rgc >= 0 )
	(*toyfunc)(myfs, *rgv++, lflag);

    if( closetoy() < 0 )
	toy_errexit("toy close botch");
    exit(0);
}

int
opentoy(device)
    char *device;
{
    extern FS *toy_openfs();

    if( (myfs = toy_openfs(device, mvflag||rmflag)) < 0 )
	return -1;
    return 0;
}

int
closetoy()
{
    return toy_closefs(myfs);
}

toycat(myfs, name)
    FS *myfs;
    char *name;
{
    extern F *argopen();

    F *fp;
    int n;
    char b[1024];

    if( vflag )
	printf("%s:\n",name);

    if( (fp = argopen(myfs, name)) == 0 )
	return;

    while( (n = toy_read(fp, b, sizeof b)) > 0 )
    {
	if( write(1, b, n) != n )
	    scerrwarn("write error");
    }
    if( n < 0 )
	toy_errwarn("read error");

    toy_close(fp);
    return 0;
}

toyrm(myfs, name)
    FS *myfs;
    char *name;
{
    if( vflag )
	printf("%s:\n",name);

    if( toy_unlink(myfs, name) < 0 )
    {
	toy_errwarn("can't unlink %s", name);
	return;
    }
}

toycreat(myfs, tgt)
    FS *myfs;
    char *tgt;
{
    extern F *toy_creat();

    char b[1024];
    int cnt;
    register I *ip;
    register F *fp;

    if( vflag )
	printf("%s:\n", tgt);

    if( (fp = toy_creat(myfs, tgt, IRWXRWXRWX)) == 0 )
    {
	toy_errwarn("can't creat %s", tgt);
	return;
    }
    while( (cnt = read(0, b, sizeof b)) > 0 )
    {
	if( toy_write(fp, b, cnt) != cnt )
	    toy_errwarn("write error");
    }
    if( toy_close(fp) < 0 )
	toy_errwarn("close error");
}

toyls(myfs, name, level)
    FS *myfs;
    char *name;
    int level;
{
    extern F *argopen();
    extern I *toy_iget();
    extern TOYIOB *toy_opendir();
    extern TOYDIR *toy_readdir();

    register F *fp;
    register I *eip;
    register TOYIOB *iobp;
    register TOYDIR *dep;

    if( vflag )
	printf("%s:\n", name);

    if( (fp = argopen(myfs, name)) == 0 )
	return;

    toy_idump(fp->f_ip, level, stdout);

    printf(" %s\n", name);

    if( rflag && (fp->f_ip->i_imode & S_IFMT) == S_IFDIR )
    {
	printf("-----\n");
	if( (iobp = toy_opendir(fp->f_ip)) == 0 )
	{
	    toy_errwarn("can't open dir %s?", name);
	    toy_close(fp);
	    return;
	}
	while( (dep = toy_readdir(iobp)) != 0 )
	    if( dep->d_ino != 0 )
	    {
		if( (eip = toy_iget(myfs, dep->d_ino)) == 0 )
		{
		    toy_errwarn("can't get inum %d?", dep->d_ino);
		    continue;
		}
		toy_idump(eip, level, stdout);
		printf(" %.14s\n", dep->d_name);
		toy_iput(eip);
	    }
	toy_closedir(iobp);
    }

    toy_close(fp);
    return 0;
}

F *
argopen(myfs, name)
    FS *myfs;
    char *name;
{
    extern I *toy_iget();
    extern F *toy_openi(), *toy_open();

    register I *ip;
    register F *fp;
    long x;

    if( iscnum(name, &x) )
    {
	if( (ip = toy_iget(myfs, x)) == 0 )
	{
	    toy_errwarn("inum %s not found", name);
	    return 0;
	}
	fp = toy_openi(ip);
	toy_iput(ip);
	if( fp == 0 )
	{
	    toy_errwarn("openi error on %s", name);
	    return 0;
	}
    }
    else
    {
	if( (fp = toy_open(myfs, name)) == 0 )
	{
	    toy_errwarn("%s not found", name);
	    return 0;
	}
    }

    return fp;
}
