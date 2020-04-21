/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*#ident	"@(#)kern-port:sys/statfs.h	10.2"*/
/*
 * $Source: /d2/3.7/src/sys/h/RCS/statfs.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:03 $
 */
/*
 * Structure returned by the statfs() system call.
 */

struct	statfs {
	short	f_fstyp;	/* File system type */
	long	f_bsize;	/* Block size */
	long	f_frsize;	/* Fragment size (if supported) */
	long	f_blocks;	/* Total number of blocks on file system */
	long	f_bfree;	/* Total number of free blocks */
	long	f_files;	/* Total number of file nodes (inodes) */
	long	f_ffree;	/* Total number of free file nodes */
	char	f_fname[6];	/* Volume name */
	char	f_fpack[6];	/* Pack name */
};
