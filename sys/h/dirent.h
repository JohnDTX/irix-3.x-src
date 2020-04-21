/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*#ident	"@(#)kern-port:sys/dirent.h	10.5"*/
/*
 * $Source: /d2/3.7/src/sys/h/RCS/dirent.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:17 $
 */

/*
 * The following structure defines the file
 * system independent directory entry.
 *
 */

struct dirent				/* data from readdir() */
	{
	long		d_ino;		/* inode number of entry */
	off_t		d_off;		/* offset of disk direntory entry */
	unsigned short	d_reclen;	/* length of this record */
	char		d_name[1];	/* name of file */
	};

/*
 * Compute a dirent's size given its name length.
 * Introduces a dependency on param.h.
 */
#define	DIRENTBASESIZE \
	(((struct dirent *) 0)->d_name - (char *) 0)
#define	DIRENTSIZE(namlen) \
	((DIRENTBASESIZE + (namlen) + NBPW) & ~(NBPW - 1))
