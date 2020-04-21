# define MEMCMDS

# define PROMSTATIC

/*
 * debug.c --
 * monitor debugging functions.
 */
# include "Qglobals.h"
# include "ndbg.h"
# include "pm2.1.h"
# include "pmIImacros.h"
# include "common.h"



debug_main(argc,argv)
    int argc;
    char **argv;
{
    argc--; argv++;

    if( argc <= 0 )
	for( ;; )
	{
	    printf("debug>");
	    readargs(&argc,&argv);
	    if(debugfunc(argc,argv)<0) return;
	}
    debugfunc(argc,argv);
}


int
debugfunc(argc,argv)
    int argc;
    char **argv;
{
    extern char *index();

    extern unsigned long *mon_frame;

    register char *cmd;
    register unsigned long *fp;
    char *ptr;
    long aa[5];
    char *bwlsizes = "b1w2l4m2p2";
    register int bwl;

    if( argc <= 0 )
	return -1;

    fp = mon_frame;
    argc--; cmd = *argv++;

    /* quit */
    if( strcmp(cmd,"q") == 0 )
    {
		return -1;
    }

    /* help! */
    if( strcmp(cmd,"h") == 0 )
    {
		debug_help(1);
		return 0;
    }

    /* break */
    if( strcmp(cmd,"break") == 0 )
    {
		if( argc > 0 )
		    return argcnt();

		if( ISMICROSW )
		{
		    TermComm();
		    ScreenComm();
		}

		asm("	trap	#0xF	");
		warmboot();
		return 0;
    }

# ifdef MEMCMDS
    if( strcmp(cmd,"mapin") == 0 )
    {
	    aa[3] = 1;
	    if( numargs(argc,argv,aa,3,4) < 0 )
		return 0;
	    mapin(aa[0],aa[1],aa[2],aa[3]);
	    return 0;
    }

    if( strcmp(cmd,"bcopy") == 0 )
    {
	    if( numargs(argc,argv,aa,3,3) < 0 )
		return 0;
	    bcopy(aa[0],aa[1],aa[2]);
	    return 0;
    }

    if( strcmp(cmd,"bzero") == 0 )
    {
	    if( numargs(argc,argv,aa,2,2) < 0 )
		return 0;
	    bzero(aa[0],aa[1]);
	    return 0;
    }

    if( strcmp(cmd,"launch") == 0 )
    {
	    aa[1] = (long)aa;
	    if( numargs(argc,argv,aa,1,2) < 0 )
		return 0;
	    LaunchStack(aa[0],aa[1],0);
	    return 0;
    }

# ifdef DEBUG
    if( strcmp(cmd,"bcmp") == 0 )
    {
	    if( numargs(argc,argv,aa,3,3) < 0 )
		return 0;
	    cmpmem(aa[0],aa[1],aa[2]);
	    return 0;
    }
# endif DEBUG
# endif MEMCMDS

    if( strcmp(cmd,"num") == 0 )
    {
	while( --argc >= 0 )
	{
	    cmd = *argv++;
	    if( !isnum(cmd,aa) )
		illegalnum(cmd);
	    else
		printf("\"%s\" = %o octal, %d decimal, %x hex\n"
			,cmd,aa[0],aa[0],aa[0]);
	}
	return 0;
    }

    /* dump memory */
    if( cmd[0] == 'd' && cmd[1] == 'm'
     && (cmd[2] == 000 || index("bwl",cmd[2]) != 0) )
    {
		aa[1] = 0xFFFFFF;	/* a big count */
		if( numargs(argc,argv,aa,1,2) < 0 )
		    return 0;
		ptr = (char *)aa[0];

		if( cmd[2] == 000 )
		{
		    dump_memory(ptr,aa[1]);
		}
		else
		{
		    bwl = index(bwlsizes,cmd[2])[1] - '0';
		    wdumpmem(ptr,bwl,aa[1]);
		}

		return 0;
    }

    /* examine / replace register */
    if( isreg(cmd) )
    {
		if( argc > 0 )
		{
		    argc--;
		    if( strcmp(*argv++,"=") != 0 )
		    {
			printf("Missing \"=\"\n");
			return 0;
		    }
		}
		aa[0] = 0;
		if( numargs(argc,argv,aa,0,1) < 0 )
		    return 0;

		regcmd(fp,cmd,argc>0,aa[0]);
		return 0;
    }

    /* dump saved registers */
    if( strcmp(cmd,"dr") == 0 )
    {
		if( argc > 0 )
		    return argcnt();

		if( fp == 0 )
		    return nosaved();

		display_regs(fp);
		return 0;
    }

    /* go / continue */
    if( strcmp(cmd,"g") == 0 )
    {
		extern int bstart[];

		if( argc > 1 )
		    return argcnt();

		if( fp == 0 )
		    return nosaved();

		*(fp+SPOFFSET) = (unsigned long)bstart;
		cmd = "c";
		/* fall through */
    }

    if( strcmp(cmd,"c") == 0 )
    {
		if( fp == 0 )
		{
		    if( argc > 0 )
			return argcnt();
		    mon_exit();
		    return 0;
		}

		aa[0] = *(fp+PCOFFSET);
		if( numargs(argc,argv,aa,0,1) < 0 )
		    return 0;
		*(fp+PCOFFSET) = aa[0];

		mon_exit();
		return 0;
    }

    if( strcmp(cmd,"du") == 0 )
    {
		if( numargs(argc,argv,aa,1,1) < 0 )
		    return 0;

		uartcmd(aa[0]);
		return 0;
    }

    /* examine or write various */
    if( (cmd[0] == 'e' || cmd[0] == 'w') && cmd[2] == 000
     && index("bwlmp",cmd[1]) != 0 )
    {
		if( numargs(argc,argv,aa,1,2) < 0 )
		    return 0;
		ptr = (char *)aa[0];

		/* examine or write mem / pagemap / protmap */
		bwl = index(bwlsizes,cmd[1])[1] - '0';

		if( cmd[1] == 'p' )
		    ptr = (char *)(PROTMAP+(short)ptr);
		if( cmd[1] == 'm' )
		    ptr = (char *)(PAGEMAP+(short)ptr);

		if( cmd[0] == 'w' )
		{
		    if( argc != 2 )
			return argcnt();

		    memwrite(ptr,aa[1],bwl);
		}
		else
		{
		    if( argc != 1 )
			return argcnt();

		    mempoke(ptr,bwl,0);
		}
		return 0;
    }

    /* failure */
    badcmd(cmd);
    return 0;
}



/* help routines */
debug_help(n)
    int n;
{
    colprint("deb(ug) DEBUGCMD","for debugging info type h debug");
    if( !n )
	return;

    noteprint("NOTE:  a number is a string of hex, decimal, or octal, digits;");
    noteprint("optionally preceded by 0x, 0t, or 0o, respectively;");
    noteprint("optionally preceded by + or -.  default radix is hex.");

    db_gen_help();

    db_brk_help();
}

db_gen_help()
{
    printf("General debugging commands:\n");
    scolprint("e{bwl} ADDR","edit bytes, words, or longs starting at ADDR");
    scolprint("w{bwl} ADDR VAL","write byte, word, or long VAL at ADDR");
    scolprint("dm{bwl} ADDR [LEN]","dump LEN bytes, words, or longs starting at ADDR");
    scolprint("bcopy ADDR TGT LEN","copy LEN bytes from ADDR to TGT");
    scolprint("bzero ADDR LEN","zero LEN bytes starting at ADDR");
    scolprint("em VP","edit memory map starting from page VP");
    scolprint("ep VP","edit protection map starting from page VP");
    scolprint("launch ADDR [STACK]","start execution at ADDR [with STACK]");
    scolprint("mapin VP PP PROT [N]","map N pages from VP to PP with PROT");
    scolprint("num [N]","print N in octal, decimal, and hex");
    scolprint("du N","display regs of duart N");
    scolprint("h","print this help message");
}

db_brk_help()
{
    printf("Break state debugging commands:\n");
    scolprint("break","warm restart");
    scolprint("c [ADDR]","continue at ADDR");
    scolprint("g [ADDR]","reset stack and run at ADDR");
    scolprint("dr","display regs from saved state");
    scolprint("REG [= VAL]","examine REG [store VAL if present]");
	noteprint("where REG is");
	noteprint("{ad}N   saved value of data or address reg N");
	noteprint("pc      saved value of pc");
	noteprint("sr      saved value of status reg");
	noteprint("x{cesx} configuration, exception, status, or context reg");
	noteprint("x{bq}   mouse buttons, or mouse quadrature reg");
}

# ifdef DEBUG
cmpmem(a,b,n)
    register unsigned char *a,*b;
    int n;
{
    register int failures;
    register int i;

    failures = 10;

    for( i = 0; i < n; i++ )
    {
	if( *a != *b )
	{
	    printf(" $%x: %x != %x\n",i,*a,*b);
	    if( --failures < 0 )
		break;
	}
	a++,b++;
    }
}
# endif DEBUG
