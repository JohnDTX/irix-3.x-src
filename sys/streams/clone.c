/* Copyright 1986, Silicon Graphics Inc., Mountain View, CA. */
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Clone Driver.
 */

#ifdef SVR3
#include "sys/sbd.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/var.h"
#include "sys/errno.h"
#include "sys/fstyp.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/pcb.h"
#include "sys/signal.h"
#include "sys/fs/s5dir.h"
#include "sys/user.h"
#include "sys/conf.h"
#include "sys/debug.h"
#include "sys/stropts.h"
#include "sys/stream.h"
#else

#include "../h/types.h"
#include "../h/param.h"
#include "../h/errno.h"
#include "../h/fstyp.h"
#include "../h/file.h"
#include "../h/inode.h"
#include "../h/user.h"
#include "../h/conf.h"
#include "../streams/stropts.h"
#include "../streams/stream.h"
#endif


int clnopen();
static struct module_info clnm_info = { 0, "CLONE", 0, 0, 0, 0 };
static struct qinit clnrinit = { NULL, NULL, clnopen, NULL, NULL, &clnm_info, NULL };
static struct qinit clnwinit = { NULL, NULL, NULL, NULL, NULL, &clnm_info, NULL };
struct streamtab clninfo = { &clnrinit, &clnwinit };

/*
 * Clone open.  Dev is the major device of the streams device to
 * open.  Look up the device in the cdevsw[].   Attach its qinit
 * structures to the read and write queues and call its open
 * with the q and the dev CLONEOPEN.  Swap in a new inode with
 * the real device number constructed from the major and the
 * minor returned by the device open.  
 */

int
clnopen(q, dev, flag, sflag)
queue_t *q;
int dev, flag, sflag;
{
	register short int extdev, mindev;
#ifdef SVR3
	register short int intdev;
#else
#define intdev extdev
#endif
	register int i;
	dev_t rdev;
	register struct streamtab *st;
	register struct file *fp;
	struct inode *ip, *nip;
	struct mount *mp;

	if (sflag) return(OPENFAIL);

	/*
	 * Get the device to open.
	 */
	extdev = minor(dev);
#ifdef SVR3
	intdev = major(makedev(extdev,0));
#endif
	if ((intdev >= cdevcnt) || !(st = cdevsw[intdev].d_str)) {
		u.u_error = ENXIO;
		return(OPENFAIL);
	}

	/*
	 * Substitute the real qinit values for the current ones
	 */
	setq(q, st->st_rdinit, st->st_wrinit);

	/*
	 * Call the device open with the stream flag  CLONEOPEN.  The device
	 * will either fail this or return a minor device number.
	 */
	rdev = makedev(extdev, 0);
	mindev = (*q->q_qinfo->qi_qopen)(q, rdev, flag, CLONEOPEN);
	if (mindev == OPENFAIL)
		return(OPENFAIL);

	/*
	 * Get the inode at top of this stream, allocate a new inode,
	 * and exchange the new inode for the old one.
	 */
	rdev = makedev(extdev, mindev);
	ip = ((struct stdata *)(q->q_next->q_ptr))->sd_inode;

	ASSERT(ip->i_sptr);

	/* set up dummy inode */

	/*
	 * Allocate an inode from the pipe file system.
	 */
	ASSERT(pipefstyp > 0 && pipefstyp < nfstyp);
	mp = fsinfo[pipefstyp].fs_pipe;
	nip = (*fstypsw[pipefstyp].fs_getinode)(mp, FSG_CLONE, rdev);
	if (!nip) {			/* bail out on failure */
		(*q->q_qinfo->qi_qclose)(q, flag);
		return(OPENFAIL);
	}

#ifdef SVR3
	plock(ip);
#else
	ilock(ip);
#endif

	nip->i_sptr = ip->i_sptr;	/* splice stream onto new inode */

#ifdef SVR3
	for (i=0; i<v.v_nofiles; i++)	/* find users handle to old inode */
		if ( (fp = u.u_ofile[i]) && (fp->f_inode == ip)) break;
	ASSERT(i < v.v_nofiles);
#else
	for (i=0; i<NOFILE; i++)	/* find users handle to old inode */
		if ( (fp = u.u_ofile[i]) && (fp->f_inode == ip)) break;
	ASSERT(i < NOFILE);
#endif

	/*
	 * link dummy inode to file table and to the stream
	 */
	ip->i_sptr = NULL;
	fp->f_inode = nip;
	nip->i_sptr->sd_inode = nip;
	nip->i_sptr->sd_strtab = st;
	iput(ip);
#ifdef SVR3
	prele(nip);
#else
	iunlock(nip);
#endif

	return(mindev);
}	
