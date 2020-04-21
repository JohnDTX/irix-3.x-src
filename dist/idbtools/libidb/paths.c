/* idb_rpath() and idb_spath() -- absolutize relative path names
 *
 * idb_rpath (name) transforms the given name, which is assumed to be
 * relative to rbase, into an absolute path name.  idb_spath is similar
 * for sbase.  In either case, the new pathname is squeezed of extraneous
 * trailing slashes, dots, dotdots, and so on, and is returned in a
 * static buffer.
 */

#define library
#include "idb.h"

char *
squeeze (buff)
	char		*buff;
{
	char		*v [128], *head, *p, first;
	int		c, i;
	Memset		*set;

	set = idb_newset ();
	c = 0;
	first = *buff;
	p = head = buff;
	while (*p) {
		if (*p == '/') {
			if (p > head) {
				*p = '\0';
				if (strcmp (head, "..") == 0) {
					if (c > 0) --c;
				} else if (strcmp (head, ".") != 0) {
					v [c++] = idb_stash (head, set);
				}
			}
			head = p + 1;
		}
		++p;
	}
	if (p > head) {
		if (strcmp (head, "..") == 0) {
			if (c > 0) --c;
		} else if (strcmp (head, ".") != 0) {
			v [c++] = idb_stash (head, set);
		}
	}
	if (first == '/') strcpy (buff, "/");
	else strcpy (buff, "");
	for (i = 0; i < c; ++i) {
		strcat (buff, v [i]);
		if (i < c - 1) strcat (buff, "/");
	}
	if (strlen (buff) == 0) strcpy (buff, ".");
	idb_freeset (set);
	return (buff);
}

char *
idb_rpath (name)
	char		*name;
{
	static char	buff [1024];

	strcpy (buff, rbase);
	strcat (buff, "/");
	strcat (buff, name);
	return (squeeze (buff));
}

char *
idb_spath (name)
	char		*name;
{
	static char	buff [1024];

	strcpy (buff, sbase);
	strcat (buff, "/");
	strcat (buff, name);
	return (squeeze (buff));
}
