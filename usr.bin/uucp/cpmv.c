/* @(#)cpmv.c	1.2 */
/* $Source: /d2/3.7/src/usr.bin/uucp/RCS/cpmv.c,v $ */
static	char	*Sccsid = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 18:30:17 $ */
#include "uucp.h"
#include <sys/types.h>
#include <sys/stat.h>


/*
 * copy f1 to f2 locally
 *	f1	-> source file name
 *	f2	-> destination file name
 * return:
 *	0	-> ok
 *	FAIL	-> failed
 */
xcp(f1, f2)
char *f1, *f2;
{
	return xxcp(f1, f2, -1);
}

xxcp(f1, f2, mode)
char *f1, *f2;
int mode;
{
	register FILE *fp1, *fp2;
	register int len;
	char buf[BUFSIZ];
	char *strrchr(), *lxp, *strcpy(), *strcat();
	char full[MAXFULLNAME];
	struct stat s;

	if ((fp1 = fopen(f1, "r")) == NULL)
		return(FAIL);
	strcpy(full, f2);
	if (stat(f2, &s) == 0) {

		/*
		 * check for directory
		 */
		if ((s.st_mode & S_IFMT) == S_IFDIR) {
			strcat(full, "/");
			strcat(full, (lxp=strrchr(f1, '/'))?lxp+1:f1);
		}
	}
	DEBUG(4, "full %s\n", full);
	if ((fp2 = fopen(full, "w")) == NULL) {
		fclose(fp1);
		return(FAIL);
	}

	/*
	 * note read and write should be
	 * checked  thoroughly
	 */
	while((len = fread(buf, sizeof (char), BUFSIZ, fp1)) > 0)
		fwrite(buf, sizeof (char), len, fp2);
	fclose(fp1);
	fclose(fp2);
	if (mode != -1)
		chmod(full,mode);
	return(0);
}

/*
 * move f1 to f2 locally
 * returns:
 *	0	-> ok
 *	FAIL	-> failed
 */
xmv(f1, f2)
register char *f1, *f2;
{
	register int ret;

	if (link(f1, f2) < 0) {

		/*
		 * copy file
		 */
		ret = xcp(f1, f2);
		if (ret)
			return(ret);
	}
	unlink(f1);
	return(0);
}
