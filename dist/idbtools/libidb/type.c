#include <sys/types.h>
#include <sys/stat.h>

idb_typec (i)
	int		i;
{
	int		c;

	if (i == S_IFBLK) c = 'b';
	else if (i == S_IFCHR) c = 'c';
	else if (i == S_IFDIR) c = 'd';
	else if (i == S_IFREG) c = 'f';
	else if (i == S_IFLNK) c = 'l';
#ifdef S_IFIFO
	else if (i == S_IFIFO) c = 'p';
#endif
	else c = 'f';
	return (c);
}

idb_typei (c)
	int		c;
{
	int		i;

	if (c == 'b') i = S_IFBLK;
	else if (c == 'c') i = S_IFCHR;
	else if (c == 'd') i = S_IFDIR;
	else if (c == 'f') i = S_IFREG;
	else if (c == 'l') i = S_IFLNK;
#ifdef S_IFIFO
	else if (c == 'p') i = S_IFIFO;
#endif
	else i = S_IFREG;
	return (i);
}
