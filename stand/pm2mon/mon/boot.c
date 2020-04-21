# include "Qdevices.h"
# include "Qglobals.h"
# include "common.h"
# include "IrisConf.h"

# include "ctype.h"

# undef DEBUG do_debug
# include "dprintf.h"


# define BASTARD_LS 1



/*
 * boot_main() --
 * monitor boot, nboot, or dboot command.
 */
boot_main(argc,argv)
    int argc;
    char **argv;
{
    extern char z[];

    register char *cmd,*spec;
    char mystr[BOOTSTRSIZE];

    if( breaked() )
	return;

    cmd = "d";
    spec = z;

    if( --argc >= 0 )
	cmd = *argv++;

    if( --argc >= 0 )
	spec = *argv++;

    if( argc > 0 )
    {
	argcnt();
	return;
    }

    switch(cmd[0])
    {
    case 'n':
	preffix("n",z,spec,mystr);
	spec = mystr;
	break;
    case 't':
	preffix("t","*",spec,mystr);
	spec = mystr;
	break;
# ifdef notdef
    case 'd':
	preffix("md","0",spec,mystr);
	spec = mystr;
	break;
# endif notdef
    }

    bootcom(spec);
}

bootcom(str)
    char *str;
{
    char mystr[BOOTSTRSIZE];
    char *prefix,*ext,*file;

    strncpy(mystr,str,sizeof mystr);
    mystr[sizeof mystr - 1] = 000;
    strcpy(_commdat->bootstr,mystr);

dprintf(("bootcom(%s)\n",mystr));
    splitspec(mystr,&prefix,&ext,&file);
    if( *prefix == 000 )
	switdefs(&prefix,&ext,&file);
    if( *file == 000 )
	file = "defaultboot";

# ifdef BASTARD_LS
    {
	extern char *rindex();
	register char *sp;

	if( (sp = rindex(file,'/')) != 0 )
	    sp++;
	else
	    sp = file;
	if( strcmp(sp,"*") == 0 )
	{
	    *sp = 000;
	    lscomm(mystr);
	    return;
	}
    }
# endif BASTARD_LS

    callboot(prefix,ext,file);
}


/*
 * switdefs() --
 * supply boot file defaults based on switches.
 */
switdefs(_pref,_ext,_file)
    char (**_pref);
    char (**_ext);
    char (**_file);
{
    extern char z[];

    register char *p,*e,*f;

    p = "md";
    e = "0";
    f = "defaultboot";

    switch(BOOTENV(switches) & ~ENV_NOBOOT)
    {
    case ENV_FLOPPYBOOT:
	p = "mf";
	break;

    case ENV_NETBOOT0:
	f = "defaultboot0";
    case ENV_NETBOOT:
	p = "n"; e = z;
	break;

    case ENV_TAPEBOOT:
	p = "t"; e = z;
	break;

    case ENV_SMDBOOT:
	p = "ip";
	break;

    default:
	break;
    }

    if( **_pref == 000 )
	*_pref = p;
    if( **_ext == 000 )
	*_ext = e;
    if( **_file == 000 )
	*_file = f;
}

/*
 * netmassage() --
 * fix .bootstr to boot from net.
 */
netmassage()
{
    extern char z[];

    char mystr[BOOTSTRSIZE];

    preffix("n",z,_commdat->bootstr,mystr);
    strcpy(_commdat->bootstr,mystr);
}

/*
 * preffix() --
 * replace / insert prefix of a boot string.
 */
preffix(defpref,defext,src,tgt)
    char *defpref,*defext;
    char *src,*tgt;
{
    char mystr[BOOTSTRSIZE];
    char *pref,*ext,*file;

    strncpy(mystr,src,sizeof mystr);
    mystr[sizeof mystr - 1] = 000;

    splitspec(mystr,&pref,&ext,&file);
    if( *ext == 000 )
	ext = *defext == 000 ? pref : defext;
    joinspec(defpref,ext,file,tgt);
}

/* help functions */
boot_help(n)
    int n;
{
    colprint("b(oot) [DEVSPEC:][file]","bootstrap load and go");
    if( !n )
	return;
    boot_ls_help();
}

boot_ls_help()
{
    noteprint("where DEVSPEC is");
    boot_specs();
}

nboot_help()
{
    colprint("n [[HOST:]file]","netboot [from HOST]");
}

tboot_help()
{
    colprint("tb [file]","tapeboot");
}

dboot_help(n)
    int n;
{
    colprint("d [DEVSPEC:][file]","defaultboot (same as b)");
}
