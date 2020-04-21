#
/*
 * monitor.c --
 * command loop of prom monitor.
 */
/*
 *----- MOD HIST -----
 *	??????? -
 *	10dec84 - D. Fong -
 *	reorganized, in order to regularize booting.
 *-----
 */
# include "Qglobals.h"
# include "setjmp.h"

# define PROMSTATIC


PROMSTATIC	jmp_buf mon_out;
PROMSTATIC	unsigned long *mon_frame;
PROMSTATIC	char *mon_prompt;

monitor(frameptr)
    unsigned long *frameptr;
{
    unsigned long *oldframe;

    oldframe = mon_frame;

    mon_frame = frameptr;

    if( setjmp(/*&*/mon_out,0) )
    {
	mon_frame = oldframe;
	return 0;
    }

    newline();
    for( ;; )
	shellcom(0);
}

shellcom(str)
    char *str;
{
    char **argv; int argc;
    int (*cmdfunc)();

    prompt();
    if( str != 0 )
    {
	printf("%s\n",str);
	ungetstr(str);
    }
    readargs(&argc,&argv);

    if( argc <= 0 )
	return;

    if( cmdlookup(*argv,&cmdfunc) < 0 )
    {
	/*
	badcmd(*argv);
	 */
	/* kluge for backward compatibility */
	debugfunc(argc,argv);
	return;
    }

    (*cmdfunc)(argc,argv);
}


int
breaked()
{
    if( mon_frame == 0 )
	return 0;

    printf("? Sorry, must first restart (r)\n");
    return -1;
}

mon_exit() { longjmp(/*&*/mon_out,1); }

prompt() { printf("%s> ",mon_frame == 0 ? "iris" : "*iris"); }
