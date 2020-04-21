#include "idb.h"
#include "inst.h"
#include <sys/utsname.h>

char *
machname ()
{
	static struct utsname	uts;

	if (uts.machine [0] == '\0' && uname (&uts) < 0) {
		uts.machine [0] = '\0';		/* just making sure */
		return (NULL);
	}
	return (uts.machine);
}
