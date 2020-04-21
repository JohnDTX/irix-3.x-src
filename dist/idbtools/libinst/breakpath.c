#include "inst.h"
#include "idb.h"
#include <ctype.h>

breakpath (path, argv, mset)
	char		*path;
	char		*argv [];
	Memset		*mset;
{
	char		*p, word [Strsize];
	int		argc;

	argc = 0;
	while (*path != '\0') {
		while (*path == '/') ++path;
		if (*path == '\0') break;
		p = word;
		while (*path != '\0' && *path != '/') *p++ = *path++;
		*p = '\0';
		if (*word == 0) break;
		if (strcmp (word, "..") == 0 && argc > 0) {
			--argc;
		} else if (strcmp (word, ".") != 0) {
			argv [argc++] = idb_stash (word, mset);
		}
	}
	if (argc == 0) argv [argc++] = idb_stash (".", mset);
	return (argc);
}
