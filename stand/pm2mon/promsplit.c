# include "sys/types.h"
# include "sys/stat.h"

# include "stdio.h"

# define K1	(1<<10)

char namebuf[512];

PROMSIZE = 16*K1;
main(argc,argv)
    int argc; char **argv;
{
    argc--; argv++;

    while( --argc >= 0 )
	promsplit(*argv++);
}

promsplit(name)
    char *name;
{
    extern FILE *promfopen();

    struct stat stat1;
    long filesize;
    int nwrite;
    FILE *A,*B,*F;
    int promno;

    if( stat(name,&stat1) < 0 )
	return errwarn("can't stat %s",name);
    if( (F = fopen(name,"r")) == 0 )
	return errwarn("can't open %s",name);
    filesize = stat1.st_size/2;

    promno = 0;
    while( filesize > 0 )
    {
	if( (A = promfopen(name,promno+1)) == 0 )
	    return -1;
	if( (B = promfopen(name,promno+0)) == 0 )
	    return -1;
	promno += 2;

	nwrite = filesize;
	if( nwrite > PROMSIZE )
	    nwrite = PROMSIZE;
	filesize -= nwrite;

	while( --nwrite >= 0 )
	{
	    putc(getc(F),A);
	    putc(getc(F),B);
	}

	fclose(A);
	fclose(B);

    }

    return 0;
}

FILE *promfopen(name,n)
    char *name;
    int n;
{
    extern char *promfile();

    register FILE *F;
    register char *cp;

    cp = promfile(name,n);
    if( (F = fopen(cp,"w")) == 0 )
    {
	errwarn("can't creat %s",name);
	return 0;
    }
    return F;
}
char *promfile(name,n)
    char *name;
    int n;
{
    sprintf(namebuf,"%s.%d",name,n);
    return namebuf;
}

int
errwarn(a)
    struct { int x[5]; } a;
{
    fprintf(stderr,a);
    fprintf(stderr,"\n");
    fflush(stderr);
    return -1;
}
