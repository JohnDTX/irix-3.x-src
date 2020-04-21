#include <sys/types.h>
#include <sys/stat.h>
#include "idb.h"
#include "inst.h"

extern char	usrdev [];
extern char	rootdev [];

char *
devnm (path)
	char		*path;
{
	static char	buff [Strsize];
	char		*argv [32];
	int		argc;
	Memset		*mset;

	argv [0] = "/etc/devnm";
	argv [1] = path;
	argv [2] = NULL;
	if (vline (2, argv, buff) < 0) {
		return (NULL);
	}
	if ((argc = words (buff, argv, mset = idb_newset ())) != 2) {
		idb_dispose (mset); return (NULL);
	}
	if (strcmp (argv [0], "devnm:") == 0) {
		idb_dispose (mset); return (NULL);
	}
	if (strncmp (argv [0], "/dev/", 5) != 0) {
		strcpy (buff, "/dev/");
	} else {
		strcpy (buff, "");
	}
	strcat (buff, argv [0]);
	idb_dispose (mset);
	return (buff);
}
