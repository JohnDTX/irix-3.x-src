/* convert a name into an absolute pathname.  Don't touch absolute names or
 * remote file names.
 */

#include "inst.h"
#include "idb.h"

extern char	*getcwd ();

abspath (rel, abs)
	char		*rel;
	char		*abs;
{
	char		*argv [Maxargs], part1 [Strsize], part2 [Strsize];
	char		buff [Strsize];
	static char	cwd [Strsize];
	int		argc;
	Memset		*mset;

	if (*rel == '/') {
		strcpy (abs, rel);
		return;
	}
	split (rel, part1, ':', part2);
	if (strlen (part2)) {
		strcpy (abs, rel);
		return;
	}
	if (*cwd == '\0') {
		if (getcwd (abs, Strsize) == NULL) {
			strcpy (abs, rel);
			return;
		}
	}
	strcpy (abs, cwd);
	if (strlen (abs) > 1) strcat (abs, "/");
	strcat (abs, rel);
}
