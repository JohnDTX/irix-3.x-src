#include "inst.h"
#include "idb.h"
#include <ctype.h>

words (line, argv, mset)
	char		*line;
	char		*argv [];
	Memset		*mset;
{
	char		*p, word [Strsize];
	int		argc;

	argc = 0;
	while (*line) {
		while (isspace (*line)) ++line;
		if (!*line) break;
		p = word;
		while (*line && !isspace (*line)) *p++ = *line++;
		*p = '\0';
		argv [argc++] = idb_stash (word, mset);
	}
	argv [argc] = NULL;
	return (argc);
}
