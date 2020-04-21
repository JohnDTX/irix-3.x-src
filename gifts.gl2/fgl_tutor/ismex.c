/*
 *	ismex - 
 *		See if the window manager is running.
 *
 *				Paul Haeberli - 1985
 *
 */
#include "grioctl.h"

main()
{
    register short i, j;

    if (ismex())
	exit(1);
    else
	exit(0);
}
