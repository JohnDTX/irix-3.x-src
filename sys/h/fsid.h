#ifndef	__FSID__
#define	__FSID__
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*#ident	"@(#)kern-port:sys/fsid.h	10.3"*/
/*
 * $Source: /d2/3.7/src/sys/h/RCS/fsid.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:32 $
 */

/* Fstyp names for use in fsinfo structure. These names */
/* must be constant across releases and will be used by a */
/* user level routine to map fstyp to fstyp index as used */
/* ip->i_fstyp. This is necessary for programs like mount. */

#define FSID_PIPE	"pipe"		/* anonymous-pipe filesystem */
#define FSID_BELL	"bell"		/* Bell (SGI SVR0) filesystem */
#define	FSID_EFS	"efs"		/* Extent filesystem */
#define	FSID_NFS	"nfs"		/* Sun NFS */
#define FSID_PROC	"proc"		/* process filesystem */
#define FSID_SOCKET	"socket"	/* socket filesystem */

#endif	__FSID__
