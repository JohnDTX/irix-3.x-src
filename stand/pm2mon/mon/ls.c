# include "Qdevices.h"
# include "Qglobals.h"
# include "common.h"

/*
 * ls.c --
 * monitor ls command.
 *
 * this should be done with a switch.
 */
ls_main(argc,argv)
    int argc;
    char **argv;
{
    argc--; argv++;

    if( argc <= 0 )
    {
	boot_specs();
	return;
    }

    if( breaked() )
	return;

    while( --argc >= 0 )
	lscomm(*argv++);
}

lscomm(str)
    char *str;
{
    register int oldpri;

    extern int disk_read(),tape_read();

    char *pref,*ext,*file;
    int (*openfunc)(),(*closefunc)(),(*readfunc)();

    splitspec(str,&pref,&ext,&file);

    if( *pref == 000 )
    {
	extern char z[];

	char *junk;
	junk = z;
	switdefs(&pref,&ext,&junk);
    }

printf("Ls: %s.%s:%s\n",pref,ext,file);

    oldpri = spl5();
    SET_USER_PROGRAM;

    if( bootlookup(pref,&openfunc,&closefunc,&readfunc) < 0
     || !(readfunc == disk_read || readfunc == tape_read) )
	printf("Can't ls from device \"%s\"\n",pref);
    else
    if( readfunc == disk_read )
	disk_list(openfunc,ext,file);
    else
	tape_list(openfunc,ext,file);

    CLEAR_USER_PROGRAM;
    splx(oldpri);
}

ls_help(n)
    int n;
{
    colprint("ls [DEVSPEC:][file]","list files");
    if( !n )
	return;
    boot_ls_help();
}
