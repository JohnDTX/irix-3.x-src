/* Derive a device name given a base device name (e.g. that of the root device)
 * and the desired file system.  The given name should be of the form
 * "...d0s0" or "...0a".  (I.e. the routine understands 2000 series, 3000
 * series, and at least early 4D series device names.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include "idb.h"
#include "inst.h"

extern char	usrdev [];
extern char	rootdev [];

derivedev (base, dev, fs)
	char		*base;
	char		*dev;
	int		fs;
{
	char		*pfs;

	strcpy (dev, base);
	pfs = dev + strlen (dev) - 1;
	if (filematch (base, "*d[0-9]s[0-9]")) {
		switch (fs) {
		case Root:
			*pfs = '0'; break;
		case Usr:
			*pfs = '6'; break;
		}
	} else if (filematch (base, "*[0-9][a-h]")) {
		switch (fs) {
		case Root:
			*pfs = 'a'; break;
		case Usr:
			if (filematch (base, "*si[0-9][a-h]")) {
				*pfs = 'f'; break;
			} else {
				*pfs = 'c'; break;
			}
		}
	} else {
		return (-1);
	}
	if (strcmp (base, dev) == 0) {
		fprintf (stderr, "Can't install on current root\n");
		return (-1);
	}
	return (0);
}
