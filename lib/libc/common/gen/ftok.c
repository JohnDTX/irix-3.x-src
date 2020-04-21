/*	@(#)ftok.c	1.2	*/
#include <sys/param.h>
#include <sys/stat.h>

key_t
ftok(path, id)
char *path;
char id;
{
	struct stat st;

	return(stat(path, &st) < 0 ? (key_t)-1 :
	    (key_t)((key_t)id << 24 | ((long)(unsigned)minor(st.st_dev)) << 16 |
		(unsigned)st.st_ino));
}
