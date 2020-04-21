/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* #ident	"@(#)head:dirent.h	1.1" */


#define MAXNAMLEN	512		/* maximum filename length */
#define DIRBUF		4096		/* buffer size for fs-indep. dirs */


typedef struct
	{
	int	dd_fd;			/* file descriptor */
	int	dd_loc;			/* offset in block */
	int	dd_size;		/* amount of valid data */
	char	*dd_buf;		/* directory block */
	}	DIR;			/* stream data from opendir() */

/*
 * Redefine names to avoid accidental name collisions when linking
 * programs that use the BSD4.2 style opendir/readdir interface.
 */

#ifndef SVR3

#define	opendir		Vopendir
#define	closedir	Vclosedir
#define	readdir		Vreaddir
#define	seekdir		Vseekdir
#define	telldir		Vtelldir
#define	scandir		Vscandir

#endif /* SVR3 */

extern DIR		*opendir();
extern struct dirent	*readdir();
extern long		telldir();
extern void		seekdir();
extern int		closedir();

#define rewinddir( dirp )	seekdir( dirp, 0L )
#include <sys/dirent.h>
