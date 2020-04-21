# include "IrisConf.h"
# define PROMSTATIC

sw_main(argc,argv)
    int argc; char **argv;
{
    long xswitches;

    argc--; argv++;
    xswitches = switches;

    if( --argc >= 0 )
    {
	if( !isnum(*argv++,&xswitches) )
	    return;
    }
    if( argc > 0 )
    {
	argcnt();
	return;
    }

    swit_detail((unsigned short)xswitches);
}


# define N_BOOTENV	(ENV_SMDBOOT>>1)
char *bdstrings[] =
{
    "floppy", "monitor", "net", "termulator",
    "netboot0", "tape", "GPIB", "disk", "smd"
};

sw_help(n)
{
    extern short HSpeeds[];
    extern char z[];

    extern char *atobinary();

    register int i;

    colprint("sw(itches) [N]","examine config switches");
    if( !n )
	return;
    colprint("BBBBAVHH; where open is 1, closed is 0",z);
    colprint(" BBBB","boot environment;");
    for( i = 0; i < N_BOOTENV; i++ )
	printf("%29s%s:%s\n", z, atobinary(i,4), bdstrings[i]);
    colprint(" A","autoboot flag; 0:yes 1:no");
    colprint(" V","verbose flag; 0:no 1:yes");
    colprint(" HH","termulator hostspeed;");
    for( i = 0; i < (1<<2); i++ )
	printf("%29s%s:%d\n", z, atobinary(i,2), HSpeeds[i]);
}

swit_detail(u)
    register unsigned short u;
{
    extern short HSpeeds[];

    register short bootenv;

    printf("switches = 0x%x\n", u);
    bootenv = BOOTENV(u);

    if( VERBOSE(u) )
	printf(" (verbose)");
    if( AUTOBOOT(bootenv) )
	printf(" (autoboot)");
    if( bootenv == (ENV_MONITOR|ENV_NOBOOT) )
	printf(" (default dc HI)");

    printf("  hostspeed %d",HSpeeds[HOSTSPEED(u)]);

    bootenv >>= 1;
    printf("  bootdev %s\n",
	    bootenv >= N_BOOTENV ? "unknown" : bdstrings[bootenv]);
}

PROMSTATIC
char atobjunk[8*sizeof (int) + 1];
char *
atobinary(n, w)
    int n;
    int w;
{
    register char *bp;
    register int i;

    for( bp = atobjunk+sizeof atobjunk-1 , i = sizeof atobjunk; --i > 0; )
    {
	*--bp = (n&01) + '0';
	n >>= 1;
    }
    return atobjunk+sizeof atobjunk- 1 - w;
}
