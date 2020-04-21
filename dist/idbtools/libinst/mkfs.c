#include <sys/types.h>
#include <sys/stat.h>
#include "idb.h"
#include "inst.h"

extern char	usrdev [];
extern char	rootdev [];

mkfs (dev)
	char		*dev;
{
	char		*argv [3];

	if (umountfs (dev) < 0) {
		fprintf (stderr, "Can't umount %s\n", dev);
		return (-1);
	}
	argv [0] = "/etc/mkfs";
	argv [1] = dev;
	argv [2] = NULL;
	return (run (2, argv, Silent) < 0 ? -1 : 0);
}
