/*
**	irismon - the iris monitor promgram - PM2.1 version
**
*/
#include "pmII.h"
#include "Qglobals.h"
#include "remprom.h"
#include "IrisConf.h"
#include "common.h"

extern char *promversion;



/*
**	main - this is the iris monitor promgram
**
*/
main()
{
    register short boottype;

    boottype = BOOTENV(switches);

    if (boottype != ENV_SLAVE
     && _commdat->reboot == MAGIC_REBOOT_VALUE)
    {
	/* reset it so we dont try to boot it after failure! */
	_commdat->reboot = 0;

	if (_commdat->boottype == MAGIC_NETBOOT)
	    netmassage();
	noisyboot(_commdat->bootstr);
    }	

    if (AUTOBOOT(boottype))
	do_autoboot(boottype);

    if (boottype == ENV_SLAVE)
	slaveboot();

    monitor(0);
}

do_autoboot(boottype)
	int boottype;
{
	register int vflag;

	vflag = VERBOSE(switches);

	switch (boottype)
	{
    	case ENV_FLOPPYBOOT:
		noisyboot("mf0:");
    		break;

    	case ENV_DISKBOOT:
		noisyboot("md:");
    		break;

	case ENV_SMDBOOT:
		noisyboot("ip0:");
    		break;

    	case ENV_NETBOOT:
		noisyboot("n:");
    		break;

    	case ENV_NETBOOT0:
		noisyboot("n:defaultboot0");
    		break;

    	case ENV_TAPEBOOT:
		noisyboot("t:");
    		break;

	case ENV_488BOOT:
		for (;;)
		{
			noisyboot("g:");
			wait488dec();
		}
		break;

	case ENV_TERMULATE:
        	printf("\n\
iris termulator %s\n",promversion);
    		termulate();
    		break;

    	case ENV_MONITOR:
        	printf("\n\
iris monitor %s\n",promversion);
    		break;

	case ENV_DONTTOUCH:
		if (vflag)
		{
			/* problem ! cant allow netbooting, etc */
			monitor(0);
			printf("\n\
MUST reset switch 3 (debug) to reboot; press <CR> when ready:");
			getstr();
			delayed_reboot();
			warmboot();
		}
		break;

    	default:
    		if (vflag)
			printf("\n\
undefined auto bootenv %d selected\n",
    				boottype);
    		break;
	}
}

/*
 * noisyboot() --
 * noisily boot from the device named by s.
 */
static
noisyboot(s)
    char *s;
{
    char junk[5 + BOOTSTRSIZE];

    strcpy(junk+0,"b ");
    strncpy(junk+2,s,sizeof junk-3);
    junk[sizeof junk - 1] = 000;

    shellcom(junk);
}
