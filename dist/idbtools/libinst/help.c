#include <stdio.h>
#include "inst.h"

help (msgid)
	char		*msgid;
{
	static FILE	*helpf = NULL;
	char		*p, line [Strsize];
	int		len;

	if (helpf == NULL && (helpf = fopen (helpfile, "r")) == NULL) {
		fprintf (stderr, "Can't open help file\n");
		return;
	}
	len = strlen (msgid);
	rewind (helpf);
	while ((p = fgets (line, sizeof (line), helpf)) != NULL) {
		if (*line == '@' && strlen (line + 2) - 1 == len &&
		    strncmp (line + 2, msgid, len) == 0) break;
	}
	if (p == NULL) {
		fprintf (stderr, "Can't find help message %s\n", msgid);
		return;
	}
	while (fgets (line, sizeof (line), helpf) != NULL && *line != '@') {
		printf ("%s", line);
	}
}
