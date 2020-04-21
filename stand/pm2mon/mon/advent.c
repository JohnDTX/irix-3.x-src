# include "pmII.h"
# include "Qdevices.h"
# include "Qglobals.h"
# include "common.h"


# define PROMSTATIC


# define TESTORG	0x40000

# define STRswitch(line)	if(0)
# define STRcaseof(str)		} else if(strcmp(str,line)==0) {
# define STRdefault		} else {

PROMSTATIC	short adv_didlook;
/*
**	adventure - fake a game of adventure
**
*/
adventure(argc,argv)
    int argc; char **argv;
{
    long aa[10];
    register char *line;
    register int first;

    argc--; argv++;

    first = argc > 0;

    if( !first )
	adv_look();

    while(1) {
	if( !first )
	    printf("> ") , readargs(&argc,&argv);

	if( --argc < 0 )
	    return;
	line = *argv++;

	STRswitch(line)
	{
	STRcaseof("look")
	    adv_didlook = 0;
	    adv_look();

# ifdef notdef
	STRcaseof("echo")
            printf("ECHO SERVE (reset machine to stop).\n");
	    echoserve();

	STRcaseof("jam")
		aa[0] = 2;
		aa[1] = -1;
		if( numargs(argc,argv,aa,0,2) < 0 )
		    continue;
		jam(aa[0],aa[1]);

	STRcaseof("write")
	    aa[0] = aa[2] = 1;
	    aa[1] = 1024;
	    if( numargs(argc,argv,aa,0,3) < 0 )
		continue;
            printf("WRITE TEST (type any character to quit).\n");
	    testwrite(aa[0],aa[1],aa[2]);

	STRcaseof("grok")
	    pnetstat();
# endif notdef

	STRcaseof("snoop")
	    argc++; argv--;
	    snoop_main(argc,argv);

	STRcaseof("xx")
	    argc++; argv--;
	    xx_main(argc,argv);

	STRcaseof("read")
	    pnetaddr();
	    
	STRcaseof("debug")
	    aa[0] = do_debug;
	    if( numargs(argc,argv,aa,0,1) < 0 )
		continue;
	    do_debug = aa[0];
	    adv_debug();
	    printf("debug $%x\n",(unsigned char)do_debug);

	STRcaseof("poof")
# include "m68vectors.h"
	    if( *EVEC_TRAPE < (long)ROM0 )
		*EVEC_TRAPE = (long)ROM0 + 2*sizeof (long); /* reset vec */
	    warmboot();

	STRcaseof("r")
# include "IrisConf.h"
	    aa[0] = switches;
	    aa[1] = do_debug;
	    if( numargs(argc,argv,aa,0,2) < 0 )
		continue;
	    do_debug = aa[1];
	    soft_restart(aa[0],do_debug);

	STRcaseof("when")
	    extern char *compdate;
	    printf("compiled %s\n",compdate);

# ifdef notdef
	STRcaseof("msdelay")
	    extern short millibuzz;
	    aa[0] = 10000;
	    aa[2] = aa[1] = millibuzz;
	    if( numargs(argc,argv,aa,0,2) < 0 )
		continue;
	    millibuzz = aa[1];
	    msdelay(aa[0]);
	    millibuzz = aa[2];

	STRcaseof("fill")
	    aa[4] = 0xFFFF; aa[5] = 0xFFFF;
	    if( numargs(argc,argv,aa,4,6) < 0 )
		continue;
	    glfill(aa[0],aa[1],aa[2],aa[3],aa[4],aa[5]);

	STRcaseof("mapcolor")
	    if( numargs(argc,argv,aa,4,4) < 0 )
		continue;
	    glmapcolor(aa[0],aa[1],aa[2],aa[3]);
# endif notdef

	STRcaseof("radix")
	    extern int def_in_radix;
	    if( numargs(argc,argv,aa,1,1) < 0 )
		continue;
	    def_in_radix = aa[0];

	STRcaseof("setbaud")
	    if( numargs(argc,argv,aa,2,2) < 0 )
		continue;
	    setbaud(aa[0],aa[1]);

	STRdefault
            printf("YOU ARE AT THE BOTTOM OF A DEEP PIT.\n");
	}

	if (first)
	    return;
    }
}

advent_help(n)
    int n;
{
    colprint("adv(enture) [CMD]","misc features");
    if( !n )
	return;
    noteprint("Good luck!");
}

adv_debug()
{
/* misc. debugging goes here */
}

adv_look()
{
    printf("\
YOU ARE AT THE BOTTOM OF A DEEP PIT.\n");
    if( !adv_didlook )
	printf("\
THERE IS A YELLOW CABLE OVERHEAD.\n\
A SLIPPERY NETWORK PROTOCOL DESCENDS TO YOUR LEFT.\n\
\n\
THERE IS A CRUMPLED NOTE ON THE FLOOR.\n");
    adv_didlook = 1;
}
