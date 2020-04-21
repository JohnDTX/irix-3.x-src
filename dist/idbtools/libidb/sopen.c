#define library
#include "idb.h"

FILE *
sopen (string, mode)
	register char	*string;
	char		*mode;
{
	FILE		*iop;
	char		tmpfname [1024];
	register int	c;

	if (string == NULL) return (NULL);
	sprintf (tmpfname, "/tmp/tmp.%d", getpid ());
	if ((iop = fopen (tmpfname, "w")) == NULL) {
		perror (tmpfname); return (NULL);
	}
	while (c = *string++) putc (c, iop); 
	fclose (iop);
	if ((iop = fopen (tmpfname, "r")) == NULL) {
		perror (tmpfname);
	}
	unlink (tmpfname);
	return (iop);
}
