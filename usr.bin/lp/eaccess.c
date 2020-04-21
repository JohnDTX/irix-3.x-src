/* eaccess(name, mode) -- determine accessibility of named file with respect
	to mode.  This routine performs the same function as access(2),
	but with respect to the effective user and group ids.
*/

#include	"lp.h"

SCCSID("@(#)eaccess.c	3.1")

int
eaccess(name, mode)
char *name;
int mode;
{
	struct stat buf;
	int perm, euid;

	if(stat(name, &buf) == -1)
		return(-1);

	if((buf.st_mode & S_IFMT) == S_IFDIR && !(mode & ACC_DIR))
		return(-1);

	if((euid = geteuid()) == 0)	/* ROOT */
		return(0);

	if(euid == buf.st_uid)
		perm = buf.st_mode;
	else if(getegid() == buf.st_gid)
		perm = (buf.st_mode & 070) << 3;
	else
		perm = (buf.st_mode & 07) << 6;

	if( ((mode & ACC_R) && !(perm & S_IREAD)) ||
	    ((mode & ACC_W) && !(perm & S_IWRITE)) ||
	    ((mode & ACC_X) && !(perm & S_IEXEC)) )
		return(-1);
	else
		return(0);
}
