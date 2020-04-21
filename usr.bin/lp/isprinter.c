/* isprinter -- predicate which returns TRUE if the specified name is
	     a legal lp printer, FALSE if not.		*/

#include	"lp.h"

SCCSID("@(#)isprinter.c	3.1")

isprinter(name)
char *name;
{
	char printer[FILEMAX];

	if(*name == '\0' || strlen(name) > DESTMAX)
		return(FALSE);

	/* Check member directory */

	sprintf(printer, "%s/%s/%s", SPOOL, MEMBER, name);
	return(eaccess(printer, ACC_R | ACC_W | ACC_DIR) != -1);
}
