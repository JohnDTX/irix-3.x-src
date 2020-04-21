/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*#ident	"@(#)kern-port:os/fstyp.c	10.10"*/
/*
 * $Source: /d2/3.7/src/sys/sys/RCS/fstyp.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:19 $
 */
#include "../h/types.h"
#include "../h/param.h"
#include "../h/fstyp.h"
#include "../h/errno.h"
#include "../h/user.h"
#include "../h/conf.h"

/*
 * NOTE: Developers making use of the File System Switch mechanism to
 * build file system types in SVR3 should be aware that the architecture
 * is not frozen and may change in future releases of System V.  File
 * system types developed for SVR3 may require changes in order to work
 * with future releases of the system.
 */

fsinit()
{
	register int i;

#ifdef FSSTRACE
	printf("fsinit: nfstyp %d\n", nfstyp);
#endif
	/* fstyp 0 is invalid, so start with 1 */
	for (i = 1; i < nfstyp; i++)
		FS_INIT(i);
}

fsnull()
{
	return(1);
}

fszero()
{
	return(0);
}

fsstray()
{
	printf("stray fs call\n");
	u.u_error = EINVAL;
	return(NULL);
}

long
fslstray()
{
	printf("stray fs (long) call\n");
	u.u_error = EINVAL;
	return(NULL);
}

int *
fsistray()
{
	printf("stray fs (int *) call\n");
	u.u_error = EINVAL;
	return(NULL);
}

struct inode *
fsinstray()
{
	printf("stray fs (inode *) call\n");
	u.u_error = EINVAL;
	return(NULL);
}

sysfs()
{

	register struct uap {
		int	opcode;
	} *uap;

	uap = (struct uap *) u.u_ap;
	switch (uap->opcode) {
	case GETFSIND:   
	{
		register int amount;

	/*
	 *	Translate fs identifier to an index
	 *	into the fsinfo structure.
	 */
		register struct	a {
			int	opcode;
			char	*fsname;
		} *uap;
		register int	index;
		register struct	fsinfo	*ap;
		char	fsbuf[FSTYPSZ];


		uap = (struct a *) u.u_ap;
		amount = fustring(uap->fsname, fsbuf, FSTYPSZ);
		if (amount >= 0) {	/* fsbuf overflow */
			u.u_error = EINVAL;
			return;
		} else if (u.u_error != 0) {
			/* ASSERT(u.u_error == EFAULT); */
			return;
		}

	/*
	 *	Search fsinfo structure for the fs identifier
	 *	and return the index.
	 */

		for (ap=fsinfo, index=0; ap < &fsinfo[nfstyp]; ap++, index++)
			if (!strcmp(ap->fs_name,fsbuf)) {
				u.u_rval1 = index;
				return;
			}

		u.u_error = EINVAL;
		break;
	}

	case GETFSTYP:
	{

	/*
	 *	Translate fstype index into an fs identifier
	 */
		register struct a {
			int	opcode;
			int	fs_index;
			char	*cbuf;
		} *uap;
		register char	*src;
		register int	index;
		register struct	fsinfo	*ap;
		char	*osrc;


		uap = (struct a *) u.u_ap;
		index = uap->fs_index;
		if (index <= 0 || index >= nfstyp) {
			u.u_error = EINVAL;
			return;
		}
		ap = &fsinfo[index];
		src = ap->fs_name ? ap->fs_name : "";

		for (osrc = src; *src++;)
			;
	
		if (copyout(osrc, uap->cbuf, src - osrc))
			u.u_error = EFAULT;

		break;
	}

	case GETNFSTYP:

	/*
	 *	Return number of fstypes configured in the
	 *	system.
	 */

		u.u_rval1 = nfstyp - 1;
		break;

	default:
		u.u_error = EINVAL;
	}
}
