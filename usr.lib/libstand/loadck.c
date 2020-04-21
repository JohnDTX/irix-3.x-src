# define FILLSIZE 0x1000
static char A[1] = 'a';
static char fillA[FILLSIZE] = 0;
static char B[1] = 'b';
static char fillB[FILLSIZE] = 0;
static char C[1] = 'c';
static char fillC[0x80] = 0;
static char D[1] = 'd';
static char fillD[0x20] = 0;

loadcheck()
{
    int argc; char **argv;

    if( A[0] != 'a' )
	badload("A");
    if( !zcomp(fillA,sizeof fillA) )
	badload("fillA");
    if( B[0] != 'b' )
	badload("B");
    if( !zcomp(fillB,sizeof fillB) )
	badload("fillB");
    if( C[0] != 'c' )
	badload("C");
    if( !zcomp(fillC,sizeof fillC) )
	badload("fillC");
    if( D[0] != 'd' )
	badload("D");
    if( !zcomp(fillD,sizeof fillD) )
	badload("fillD");
}

static
badload(str)
    char *str;
{
    printf("load error on %s\n",str);
    msdelay(10000);
}

static int
zcomp(s,n)
    register char *s;
    register int n;
{
    while( --n >= 0 )
	if( *s++ != 0 )
	    return 0;
    return 1;
}
