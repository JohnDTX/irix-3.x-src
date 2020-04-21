static char Sccsid[]="@(#)rename.c	3.2";
# include "errno.h"
# include "fatal.h"
# include "signal.h"
/*
	rename (unlink/link)
	Calls xlink() and xunlink().
*/

rename(oldname,newname)
char *oldname, *newname;
{
	extern int errno;
	int (* holdsig[3])();
	int retval;
	
	/*Ignor signals 01 02 03 */
	holdsig[0] = signal(SIGHUP,SIG_IGN);
	holdsig[1] = signal(SIGINT,SIG_IGN);
	holdsig[2] = signal(SIGQUIT,SIG_IGN);
	if (unlink(newname) < 0 && errno != ENOENT)
		retval = xunlink(newname);

	if (xlink(oldname,newname) == Fvalue)
		retval = -1;
	retval = (xunlink(oldname));
	/*re establish signals */
	signal(SIGHUP,holdsig[0]);
	signal(SIGINT,holdsig[1]);
	signal(SIGQUIT,holdsig[2]);
	return(retval);
}
