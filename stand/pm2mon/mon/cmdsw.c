/*
 * cmdsw.c --
 * monitor command switch.
 */
struct cmddesc
{
    char *cmdname;
    short minmatch;
    int (*cmdfunc)();
    int (*helpfunc)();
};

extern int debug_main(),debug_help();
extern int ls_main(),ls_help();
extern int boot_main(),boot_help();
# define help_main cmdhelp
extern int help_main(),help_help();
# define term_main termulate
extern int term_main(),term_help();
# define restart_main warmboot
extern int restart_main(),restart_help();
# define advent_main adventure
extern int advent_main(),advent_help();
# define nboot_main boot_main
extern int nboot_main(),nboot_help();
# define dboot_main boot_main
extern int dboot_main(),dboot_help();
# define tboot_main boot_main
extern int tboot_main(),tboot_help();
extern int sw_main(),sw_help();

struct cmddesc cmdbin[] =
{
	{"boot",1,	boot_main,boot_help},
	{"nboot",1,	nboot_main,nboot_help},
	{"d",1,		dboot_main,dboot_help},
	{"tboot",2,	tboot_main,tboot_help},
	{"ls",2,	ls_main,ls_help},
	{"r",1,		restart_main,restart_help},
	{"termulate",1,	term_main,term_help},
	{"adventure",3,	advent_main,advent_help},
	{"debug",3,	debug_main,debug_help},
	{"help",1,	help_main,help_help},
	{"switches",2,	sw_main,sw_help},
	{0},
};

int
cmdlookup(cmd,_cmdfunc)
    char *cmd;
    int (**_cmdfunc)();
{
    register struct cmddesc *rp;
    register int cmdlen;

    cmdlen = strlen(cmd);

    for( rp = cmdbin; rp->cmdname != 0; rp++ )
	if( cmdlen >= rp->minmatch
	 && strncmp(cmd,rp->cmdname,cmdlen) == 0 )
	{
	    *_cmdfunc = rp->cmdfunc;
	    return 0;
	}

    return -1;
}


/*
 * cmdhelp() --
 * print help messages (by calling help routines).
 * call with 0 for terse help message, 1 for verbose.
 */
cmdhelp(argc,argv)
    int argc;
    char **argv;
{
    register struct cmddesc *rp;
    register char *cmd;
    register int cmdlen;
    register int found;

    argc--; argv++;

    if( argc > 0 )
    {
	cmd = *argv;
	cmdlen = strlen(cmd);

	found = 0;
	for( rp = cmdbin; rp->cmdname != 0; rp++ )
	    if( cmdlen >= rp->minmatch
	     && strncmp(cmd,rp->cmdname,cmdlen) == 0 )
	    {
		found++;
		(*rp->helpfunc)(1);
	    }

	if( !found )
	    printf("No help is listed under \"%s\"\n",cmd);
	return;
    }

    printf("iris PM2 Prom Monitor commands:\n");

    for( rp = cmdbin; rp->cmdname != 0; rp++ )
	(*rp->helpfunc)(0);
}


badcmd(cmd)
    register char *cmd;
{
    printf("\
\"%s\" is not a legal command\n\
Type h for help\n",cmd);
}
