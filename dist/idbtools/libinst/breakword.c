#include "inst.h"
#include "idb.h"
#include <ctype.h>

breakword (word, ch, lim, argv, mset)
	char		*word;
	int		ch;
	int		lim;
	char		*argv [];
	Memset		*mset;
{
	char		*p, wordbuff [Strsize];
	int		argc;

	argc = 0;
	while (*word != '\0' && *word != lim) {
		p = wordbuff;
		while (*word != '\0' && *word != ch && *word != lim)
			*p++ = *word++;
		*p = '\0';
		if (*word == 0) break;
		argv [argc++] = idb_stash (word, mset);
	}
	return (argc);
}
