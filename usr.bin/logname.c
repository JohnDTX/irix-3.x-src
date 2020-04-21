char _Origin_[] = "System V";

static char Sccsid[] = "@(#)logname.c	1.3";

#include <stdio.h>

main()
{
	char *name, *cuserid();

	name = cuserid((char *)NULL);
	if (name == NULL)
		return (1);
	(void) puts (name);
	return (0);
}
