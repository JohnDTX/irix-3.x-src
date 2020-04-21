#include "idb.h"

/* bytewise io; support for reading and writing oddly shaped objects */

int
getbyte (f)
	int		f;
{
	char		byte;

	if (vread (f, &byte, 1) != 1) {
		return (-1);
	}
	return (byte & 0377);
}

putbyte (f, byte)
	int		f;
	char		byte;
{
	if (vwrite (f, &byte, 1) != 1) return (-1);
	return (0);
}

char *
getstr (f, mset)
	int		f;
	Memset		*mset;
{
	char		*p;
	short		len, getshort ();

	if ((len = getshort (f)) < 0) return (NULL);
	if (len == 0) return (idb_stash ("", mset));
	p = idb_getmem (len + 1, mset);
	if (vread (f, p, len) != len) return (NULL);
	p [len] = '\0';
	return (p);
}

putstr (f, p)
	int		f;
	char		*p;
{
	short		len;

	if (p == NULL) p = "";
	len = strlen (p);
	if (putshort (f, len) < 0) return (-1);
	if (len == 0) return (len);
	if (vwrite (f, p, len) != len) return (-1);
	return (len);
}

short
getshort (f)
	int		f;
{
	char		buff [2];

	if (vread (f, buff, 2) != 2) return (-1);
	return ((short) ((buff [0] & 0377) << 8) + (buff [1] & 0377));
}

putshort (f, t)
	int		f;
	short		t;
{
	char		buff [2];

	buff [0] = (t >> 8) & 0377;
	buff [1] = (t & 0377);
	if (vwrite (f, buff, 2) != 2) return (-1);
	return (0);
}

long
getlong (f)
	int		f;
{
	char		buff [4];

	if (vread (f, buff, 4) != 4) return (-1);
	return ((long) ((buff [0] & 0377) << 24) + ((buff [1] & 0377) << 16) +
		((buff [2] & 0377) << 8) + (buff [3] & 0377));
}

putlong (f, t)
	FILE		*f;
	long		t;
{
	char		buff [4];

	buff [0] = (t >> 24) & 0377;
	buff [1] = (t >> 16) & 0377;
	buff [2] = (t >> 8) & 0377;
	buff [3] = t & 0377;
	if (vwrite (f, buff, 4) != 4) return (-1);
	return (0);
}
