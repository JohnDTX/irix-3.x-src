#include <stdio.h>

static char	*xrshtable [] = {
			"/usr/bsd/rsh",
			"/usr/bin/rsh",
			"rsh",
		};

char *
xrsh ()
{
	static int	i = -1;

	if (i != -1) return (xrshtable [i]);
	for (i = 0; i < sizeof (xrshtable) / sizeof (*xrshtable); ++i) {
		if (access (xrshtable [i], 1) == 0) {
			return (xrshtable [i]);
		}
	}
	fprintf (stderr, "warning: can't find rsh in standard places\n");
	return (xrshtable [--i]);
}
