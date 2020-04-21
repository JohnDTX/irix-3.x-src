/*	@(#)devtolin.c	1.2 of 3/31/82	*/
/*
 *	convert device to linename (as in /dev/linename)
 *	return ptr to LSZ-byte string, "?" if not found
 *	device must be character device
 *	maintains small list in tlist for speed
 */

#include <sys/types.h>
#include "acctdef.h"
#include <stdio.h>
#include <dirent.h>

#define TSIZE1	50	/* # distinct names, for speed only */
static	tsize1;
static struct tlist {
	char	tname[LSZ];	/* linename */
	dev_t	tdev;		/* device */
} tl[TSIZE1];

static struct dirent d;

dev_t	lintodev();

char *
devtolin(device)
dev_t device;
{
	register struct tlist *tp;
	register struct dirent *dp;
	DIR *fdev;

	for (tp = tl; tp < &tl[tsize1]; tp++)
		if (device == tp->tdev)
			return(tp->tname);

	if ((fdev = opendir("/dev")) == NULL)
		return("?");
	while ((dp = readdir(fdev)) != NULL) {
		if (dp->d_ino != 0 && lintodev(dp->d_name) == device) {
			if (tsize1 < TSIZE1) {
				tp->tdev = device;
				CPYN(tp->tname, dp->d_name);
				tsize1++;
			}
			CPYN(d.d_name, dp->d_name);
			closedir(fdev);
			return(d.d_name);
		}
	}
	closedir(fdev);
	return("?");
}
