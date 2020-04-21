#define library
#include "idb.h"

typedef struct Idmap {
	char	*name;
	short	id;
} Idmap;

Idmap		uid [1024];
int		nuid = -1;

Idmap		gid [1024];
int		ngid = -1;

int
idb_uid (name)
	char		*name;
{
	register int	i;

	for (i = 0; i < nuid; ++i) {
		if (strcmp (name, uid [i].name) == 0) return (uid [i].id);
	}
	return (9999);
}

int
idb_gid (name)
	char		*name;
{
	register int	i;

	for (i = 0; i < ngid; ++i) {
		if (strcmp (name, gid [i].name) == 0) return (gid [i].id);
	}
	return (9999);
}

char *
idb_uname (u)
	int		u;
{
	register int	i;

	for (i = 0; i < nuid; ++i) {
		if (uid [i].id == u) return (uid [i].name);
	}
	return ("???");
}

char *
idb_gname (g)
	int		g;
{
	register int	i;

	for (i = 0; i < ngid; ++i) {
		if (gid [i].id == g) return (gid [i].name);
	}
	return ("???");
}

idb_passwd (passwd, group)
	char		*passwd;
	char		*group;
{
	int		i;

	for (i = 0; i < nuid; ++i) free (uid [i].name);
	for (i = 0; i < ngid; ++i) free (gid [i].name);
	if ((nuid = loadmap (passwd, uid)) < 0 ||
	    (ngid = loadmap (group, gid)) < 0) return (-1);
	return (0);
}

loadmap (fname, map)
	char		*fname;
	register Idmap	*map;
{
	FILE		*f;
	char		*p, *namend, name [1024];
	int		n, c, id;

	if ((f = fopen (fname, "r")) == NULL) return (-1);
	n = 0;
	namend = name + sizeof (name);
	while ((c = getc (f)) != EOF) {
		p = name;
		while (c != ':' && c != EOF) {
			if (p >= namend) break;
			*p++ = c;
			c = getc (f);
		}
		*p = '\0';
		if (c == EOF) break;
		while ((c = getc (f)) != ':' && c != EOF) ;
		if (c == EOF) break;
		id = 0;
		while ((c = getc (f)) >= '0' && c <= '9') {
			id = id * 10 + c - '0';
		}
		if (c == ':') {
			map [n].name = idb_stash (name, NULL);
			map [n].id = id;
			++n;
		}
		while (c != '\n' && c != EOF) c = getc (f);
	}
	fclose (f);
	return (n);
}
