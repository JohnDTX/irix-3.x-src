#include <sys/types.h>
#include <sys/stat.h>
#include "idb.h"
#include "inst.h"

extern char	usrdev [];
extern char	rootdev [];

umountfs (dev)
	char		*dev;
{
	char		buff [Strsize], *argv [32];
	int		argc, e;
	Memset		*mset;

	argv [0] = "/etc/umount";
	argv [1] = dev;
	argv [2] = NULL;
	if (vline (2, argv, buff) < 0) return (-1);
	if ((argc = words (buff, argv, mset = idb_newset ())) == 0 ||
	    argc == 3 && strcmp (argv [1], "not") == 0 &&
	    strcmp (argv [2], "mounted") == 0) {	/* how quaint... */
		e = 0;
	} else {
		e = -1;
	}
	idb_dispose (mset);
	return (e);
}
