# include "Qdevices.h"
# include "Qglobals.h"
# include "remprom.h"
# include "common.h"

# include "ctype.h"
# include "Xns.h"

# undef DEBUG do_debug
# include "dprintf.h"

# define BASTARD_SNOOP

snoop_main(argc,argv)
    int argc;
    char **argv;
{
    if( breaked() )
	return;

    SET_USER_PROGRAM;

    argc--; argv++;

    if( argc <= 0 )
	snoop("/");
    else

    while( --argc >= 0 )
	snoop(*argv++);

    CLEAR_USER_PROGRAM;
}


/*
 * snoop() --
 * print replies of hosts having a certain file.
 */
int
snoop(file)
    char *file;
{
    extern int net_oldpri;

# ifdef BASTARD_SNOOP
    char str[3 + sizeof _commdat->bootstr];
    Xhost boothost;
    char *info;

    bootstr(str,sizeof str,"*",file);

    net_oldpri = spl1();

    setbcast();

    if( bcast(str) < 0 )
    {
	splx(net_oldpri);
	return -1;
    }

    while( getreply(&boothost,&info) >= 0 )
	printf("<%s>\n",info+2);

    splx(net_oldpri);

# endif BASTARD_SNOOP
    return 0;
}
