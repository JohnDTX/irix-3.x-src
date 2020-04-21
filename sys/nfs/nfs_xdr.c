/*      @(#)nfs_xdr.c 1.1 85/05/30 SMI      */

#ifdef SVR3
# include "sys/debug.h"
# include "sys/types.h"
# include "sys/sysmacros.h"
# include "sys/param.h"
# include "sys/systm.h"
# include "sys/signal.h"
# include "sys/errno.h"
# include "sys/psw.h"
# include "sys/pcb.h"
# include "sys/user.h"
# include "sys/buf.h"
# include "sys/dirent.h"
# include "sys/file.h"
# include "sys/inode.h"
# include "sys/mbuf.h"
# include "netinet/in.h"
# include "sys/fs/nfs.h"
# include "rpc/types.h"
# include "rpc/xdr.h"
#else
# include "../h/param.h"
# include "../h/user.h"
# include "../h/buf.h"
# include "../h/dirent.h"
# include "../h/file.h"
# include "../h/inode.h"
# include "../h/mbuf.h"
# include "../netinet/in.h"
# include "../nfs/nfs.h"
# include "../rpc/types.h"
# include "../rpc/xdr.h"
#endif

#ifdef NFSDEBUG
extern int nfsdebug;

char *xdropnames[] = {"encode", "decode", "free"};
#endif

/*
 * These are the XDR routines used to serialize and deserialize
 * the various structures passed as parameters accross the network
 * between NFS clients and servers.
 */

/*
 * File access handle
 * The fhandle struct is treated a opaque data on the wire
 */
bool_t
xdr_fhandle(xdrs, fh)
	XDR *xdrs;
	fhandle_t *fh;
{

	if (xdr_opaque(xdrs, (caddr_t)fh, NFS_FHSIZE)) {
#ifdef NFSDEBUG
		dprint(nfsdebug, 6, "xdr_fhandle: %s %x %d\n",
		    xdropnames[(int)xdrs->x_op], fh->fh_fsid, fh->fh_fno);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_fhandle %s FAILED\n",
	    xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}


/*
 * Arguments to remote write and writecache
 */
bool_t
xdr_writeargs(xdrs, wa)
	register XDR *xdrs;
	register struct nfswriteargs *wa;
{
	register bool_t result = FALSE;

	if (xdr_fhandle(xdrs, &wa->wa_fhandle) &&
	    xdr_long(xdrs, (long *)&wa->wa_begoff) &&
	    xdr_long(xdrs, (long *)&wa->wa_offset) &&
	    xdr_long(xdrs, (long *)&wa->wa_totcount)) {
		switch (xdrs->x_op) {
		case XDR_DECODE:	/* server */
			ASSERT(wa->wa_data == (char *) 0);
			if (! xdr_u_int(xdrs, &wa->wa_count)
			    || (wa->wa_count > NFS_MAXDATA)) {
				break;
			}
			if (wa->wa_count < BBSIZE) {
				wa->wa_bp = (struct buf *) 0;
				wa->wa_data = kmem_alloc((u_int) wa->wa_count);
			} else {
				wa->wa_bp = geteblk(BTOBB(wa->wa_count));
				wa->wa_data = wa->wa_bp->b_un.b_addr;
			}
			ASSERT(wa->wa_data);
			result = xdr_opaque(xdrs, wa->wa_data, wa->wa_count);
			break;
	
		case XDR_FREE:		/* server */
			ASSERT(wa->wa_data);
			if (wa->wa_bp) {
				brelse(wa->wa_bp);
				wa->wa_data = (char *) 0;
			}
			/* FALL THROUGH */

		case XDR_ENCODE:	/* client */
			result = xdr_bytes(xdrs, &wa->wa_data,
				(u_int *)&wa->wa_count, NFS_MAXDATA);
			break;
		}
		if (result) {
#ifdef NFSDEBUG
			dprint(nfsdebug, 6, "xdr_writeargs: %s off %d ct %d\n",
			    xdropnames[(int)xdrs->x_op],
			    wa->wa_offset, wa->wa_totcount);
#endif
			return (TRUE);
		}
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_writeargs: %s FAILED\n",
	    xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}


/*
 * File attributes
 */
bool_t
xdr_fattr(xdrs, na)
	XDR *xdrs;
	register struct nfsfattr *na;
{
	register long *ptr;

#ifdef NFSDEBUG
	dprint(nfsdebug, 6, "xdr_fattr: %s\n",
	    xdropnames[(int)xdrs->x_op]);
#endif
	if (xdrs->x_op == XDR_ENCODE) {
		ptr = XDR_INLINE(xdrs, 17 * BYTES_PER_XDR_UNIT);
		if (ptr != NULL) {
			IXDR_PUT_ENUM(ptr, na->na_type);
			IXDR_PUT_LONG(ptr, na->na_mode);
			IXDR_PUT_LONG(ptr, na->na_nlink);
			IXDR_PUT_LONG(ptr, na->na_uid);
			IXDR_PUT_LONG(ptr, na->na_gid);
			IXDR_PUT_LONG(ptr, na->na_size);
			IXDR_PUT_LONG(ptr, na->na_blocksize);
			IXDR_PUT_LONG(ptr, na->na_rdev);
			IXDR_PUT_LONG(ptr, na->na_blocks);
			IXDR_PUT_LONG(ptr, na->na_fsid);
			IXDR_PUT_LONG(ptr, na->na_nodeid);
			IXDR_PUT_LONG(ptr, na->na_atime.tv_sec);
			IXDR_PUT_LONG(ptr, na->na_atime.tv_usec);
			IXDR_PUT_LONG(ptr, na->na_mtime.tv_sec);
			IXDR_PUT_LONG(ptr, na->na_mtime.tv_usec);
			IXDR_PUT_LONG(ptr, na->na_ctime.tv_sec);
			IXDR_PUT_LONG(ptr, na->na_ctime.tv_usec);
			return (TRUE);
		}
	} else {
		ptr = XDR_INLINE(xdrs, 17 * BYTES_PER_XDR_UNIT);
		if (ptr != NULL) {
			na->na_type = IXDR_GET_ENUM(ptr, enum nfsftype);
			na->na_mode = IXDR_GET_LONG(ptr);
			na->na_nlink = IXDR_GET_LONG(ptr);
			na->na_uid = IXDR_GET_LONG(ptr);
			na->na_gid = IXDR_GET_LONG(ptr);
			na->na_size = IXDR_GET_LONG(ptr);
			na->na_blocksize = IXDR_GET_LONG(ptr);
			na->na_rdev = IXDR_GET_LONG(ptr);
			na->na_blocks = IXDR_GET_LONG(ptr);
			na->na_fsid = IXDR_GET_LONG(ptr);
			na->na_nodeid = IXDR_GET_LONG(ptr);
			na->na_atime.tv_sec = IXDR_GET_LONG(ptr);
			na->na_atime.tv_usec = IXDR_GET_LONG(ptr);
			na->na_mtime.tv_sec = IXDR_GET_LONG(ptr);
			na->na_mtime.tv_usec = IXDR_GET_LONG(ptr);
			na->na_ctime.tv_sec = IXDR_GET_LONG(ptr);
			na->na_ctime.tv_usec = IXDR_GET_LONG(ptr);
			return (TRUE);
		}
	}
	if (xdr_enum(xdrs, (enum_t *)&na->na_type) &&
	    xdr_u_long(xdrs, &na->na_mode) &&
	    xdr_u_long(xdrs, &na->na_nlink) &&
	    xdr_u_long(xdrs, &na->na_uid) &&
	    xdr_u_long(xdrs, &na->na_gid) &&
	    xdr_u_long(xdrs, &na->na_size) &&
	    xdr_u_long(xdrs, &na->na_blocksize) &&
	    xdr_u_long(xdrs, &na->na_rdev) &&
	    xdr_u_long(xdrs, &na->na_blocks) &&
	    xdr_u_long(xdrs, &na->na_fsid) &&
	    xdr_u_long(xdrs, &na->na_nodeid) &&
	    xdr_timeval(xdrs, &na->na_atime) &&
	    xdr_timeval(xdrs, &na->na_mtime) &&
	    xdr_timeval(xdrs, &na->na_ctime) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_fattr: %s FAILED\n",
	    xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

/*
 * Arguments to remote read
 */
bool_t
xdr_readargs(xdrs, ra)
	XDR *xdrs;
	struct nfsreadargs *ra;
{

	if (xdr_fhandle(xdrs, &ra->ra_fhandle) &&
	    xdr_long(xdrs, (long *)&ra->ra_offset) &&
	    xdr_long(xdrs, (long *)&ra->ra_count) &&
	    xdr_long(xdrs, (long *)&ra->ra_totcount) ) {
#ifdef NFSDEBUG
		dprint(nfsdebug, 6, "xdr_readargs: %s off %d ct %d\n",
		    xdropnames[(int)xdrs->x_op],
		    ra->ra_offset, ra->ra_totcount);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_readargs: %s FAILED\n",
	    xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

/*
 * Info necessary to free the bp which is also an mbuf
 */
struct rrokinfo {
	int	(*func)();	/* MUST BE FIRST */
	int	count;
	struct buf *bp;
};
	
static
rrokfree(rip)
	register struct rrokinfo *rip;
{
	register int s;

	s = splimp();
	while (rip->count > 0) {
		sleep((caddr_t)rip, PZERO-1);
	}
	(void) splx(s);
	if (rip->bp != NULL) {
		brelse(rip->bp);
	}
	kmem_free(rip, sizeof *rip);
}

/*
 * Wake up user process to free bp (nfssvc sleeping in rrokfree).
 * Called when an mbuf containing buffer cache read data is freed by
 * a network interface on transmit-done interrupt.
 */
static
rrokwakeup(mp)
	struct mbuf *mp;
{
	register struct rrokinfo *rip;

	rip = (struct rrokinfo *) mp->m_farg;
	ASSERT(rip->count > 0);
	if (--rip->count <= 0) {
		wakeup((caddr_t)rip);
	}
}

static
rrokdup(mp)
	struct mbuf *mp;
{
	register struct rrokinfo *rip;

	rip = (struct rrokinfo *) mp->m_darg;
	ASSERT(rip->count > 0);
	rip->count++;
}

/*
 * Status OK portion of remote read reply
 */
bool_t
xdr_rrok(xdrs, rrok)
	register XDR *xdrs;
	register struct nfsrrok *rrok;
{

	if (xdr_fattr(xdrs, &rrok->rrok_attr)) {
		if (xdrs->x_op == XDR_ENCODE && rrok->rrok_bp) {
			/* server side */
			register struct rrokinfo *rip;

			rip = (struct rrokinfo *)kmem_alloc(sizeof *rip);
			rip->func = rrokfree;
			rip->count = 0;
			rip->bp = rrok->rrok_bp;
			xdrs->x_public = (caddr_t)rip;
			if (xdrmbuf_putbuf(xdrs, rrok->rrok_data,
			    (u_int)rrok->rrok_count, rrokwakeup, (long)rip,
			    rrokdup, (long)rip)) {
#ifdef NFSDEBUG
				dprint(nfsdebug, 6, "xdr_rrok: %s %d addr %x\n",
				    xdropnames[(int)xdrs->x_op],
				    rrok->rrok_count, rrok->rrok_data);
#endif
				rip->count++;
				return (TRUE);
			} else {
				rip->count = 0;
			}
		} else {			/* client side */
			if (xdr_bytes(xdrs, &rrok->rrok_data,
			    (u_int *)&rrok->rrok_count, NFS_MAXDATA) ) {
#ifdef NFSDEBUG
				dprint(nfsdebug, 6, "xdr_rrok: %s %d addr %x\n",
				    xdropnames[(int)xdrs->x_op],
				    rrok->rrok_count, rrok->rrok_data);
#endif
				return (TRUE);
			}
		}
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_rrok: %s FAILED\n",
	    xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

struct xdr_discrim rdres_discrim[2] = {
	{ (int)NFS_OK, xdr_rrok },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Reply from remote read
 */
bool_t
xdr_rdresult(xdrs, rr)
	XDR *xdrs;
	struct nfsrdresult *rr;
{

#ifdef NFSDEBUG
	dprint(nfsdebug, 6, "xdr_rdresult: %s\n", xdropnames[(int)xdrs->x_op]);
#endif
	if (xdr_union(xdrs, (enum_t *)&(rr->rr_status),
	      (caddr_t)&(rr->rr_ok), rdres_discrim, xdr_void) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_rdresult: %s FAILED\n",
	    xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

/*
 * File attributes which can be set
 */
bool_t
xdr_sattr(xdrs, sa)
	XDR *xdrs;
	struct nfssattr *sa;
{

	if (xdr_u_long(xdrs, &sa->sa_mode) &&
	    xdr_u_long(xdrs, &sa->sa_uid) &&
	    xdr_u_long(xdrs, &sa->sa_gid) &&
	    xdr_u_long(xdrs, &sa->sa_size) &&
	    xdr_timeval(xdrs, &sa->sa_atime) &&
	    xdr_timeval(xdrs, &sa->sa_mtime) ) {
#ifdef NFSDEBUG
		dprint(nfsdebug, 6,
		    "xdr_sattr: %s mode %o uid %d gid %d size %d\n",
		    xdropnames[(int)xdrs->x_op], sa->sa_mode, sa->sa_uid,
		    sa->sa_gid, sa->sa_size);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_sattr: %s FAILED\n",
	    xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

struct xdr_discrim attrstat_discrim[2] = {
	{ (int)NFS_OK, xdr_fattr },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Reply status with file attributes
 */
bool_t
xdr_attrstat(xdrs, ns)
	XDR *xdrs;
	struct nfsattrstat *ns;
{

	if (xdr_union(xdrs, (enum_t *)&(ns->ns_status),
	      (caddr_t)&(ns->ns_attr), attrstat_discrim, xdr_void) ) {
#ifdef NFSDEBUG
		dprint(nfsdebug, 6, "xdr_attrstat: %s stat %d\n",
		    xdropnames[(int)xdrs->x_op], ns->ns_status);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_attrstat: %s FAILED\n",
	    xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

/*
 * NFS_OK part of read sym link reply union
 */
bool_t
xdr_srok(xdrs, srok)
	XDR *xdrs;
	struct nfssrok *srok;
{

	if (xdr_bytes(xdrs, &srok->srok_data, (u_int *)&srok->srok_count,
	    NFS_MAXPATHLEN) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_srok: %s FAILED\n",
	    xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

struct xdr_discrim rdlnres_discrim[2] = {
	{ (int)NFS_OK, xdr_srok },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Result of reading symbolic link
 */
bool_t
xdr_rdlnres(xdrs, rl)
	XDR *xdrs;
	struct nfsrdlnres *rl;
{

#ifdef NFSDEBUG
	dprint(nfsdebug, 6, "xdr_rdlnres: %s\n", xdropnames[(int)xdrs->x_op]);
#endif
	if (xdr_union(xdrs, (enum_t *)&(rl->rl_status),
	      (caddr_t)&(rl->rl_srok), rdlnres_discrim, xdr_void) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_rdlnres: %s FAILED\n",
	    xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

/*
 * Arguments to readdir
 */
bool_t
xdr_rddirargs(xdrs, rda)
	XDR *xdrs;
	struct nfsrddirargs *rda;
{
	if (xdr_fhandle(xdrs, &rda->rda_fh) &&
	    xdr_u_long(xdrs, &rda->rda_offset) &&
	    xdr_u_long(xdrs, &rda->rda_count) ) {
#ifdef NFSDEBUG
		dprint(nfsdebug, 6,
		    "xdr_rddirargs: %s fh %x %d, off %d, cnt %d\n",
		    xdropnames[(int)xdrs->x_op],
		    rda->rda_fh.fh_fsid, rda->rda_fh.fh_fno, rda->rda_offset,
		    rda->rda_count);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_rddirargs: %s FAILED\n",
	    xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

/*
 * Directory read reply:
 * union (enum status) {
 *	NFS_OK: union entlist;
 *		boolean eof;
 *	default:
 * }
 *
 * Directory entries
 *	struct  dirent {
 *		long	d_ino;		* inode number of entry *
 *		off_t	d_off;		* offset of this record *
 *		u_short d_reclen;	* length of this record *
 *		char    d_name[1];	* name, consistent with reclen *
 *	};
 * are on the wire as:
 * union entlist (boolean valid) {
 * 	TRUE: struct dirent;
 *	      u_long nxtoffset;
 * 	      union entlist;
 *	FALSE:
 * }
 * where dirent is:
 * 	struct dirent {
 *		u_long	de_fid;
 *		string	de_name<NFS_MAXNAMELEN>;
 *	}
 */

#define	nextdp(dp)	((struct dirent *)((char *)(dp) + (dp)->d_reclen))

/*
 * ENCODE ONLY
 */
bool_t
xdr_putrddirres(xdrs, rd)
	register XDR *xdrs;
	register struct nfsrddirres *rd;
{
	register struct dirent *dp;
	register int size;
	register int xdrpos;
	auto bool_t true = TRUE;
	auto bool_t false = FALSE;
#ifdef DIROFFBUG
	/* XXX fix dirent offset bug and remove this in next SVR0 release */
	auto u_long offset = 0;
#endif

#ifdef NFSDEBUG
	dprint(nfsdebug, 6, "xdr_putrddirres: %s size %d offset %d\n",
	    xdropnames[(int)xdrs->x_op], rd->rd_size, rd->rd_offset);
#endif
	if (xdrs->x_op != XDR_ENCODE) {
		return (FALSE);
	}
	if (!xdr_enum(xdrs, (enum_t *)&rd->rd_status)) {
		return (FALSE);
	}
	if (rd->rd_status != NFS_OK) {
		return (TRUE);
	}

	xdrpos = XDR_GETPOS(xdrs);
	for (size = rd->rd_size, dp = rd->rd_entries;
	     size > 0;
	     size -= dp->d_reclen, dp = nextdp(dp) ) {
		auto char *name;
		auto u_int namlen;
		register char *cp;

		if (dp->d_reclen == 0) {
#ifdef NFSDEBUG
			dprint(nfsdebug, 2, "xdr_putrddirres: bad directory\n");
#endif
			return (FALSE);
		}
#ifdef NFSDEBUG
		dprint(nfsdebug, 10,
		    "xdr_putrddirres: entry %d %s %d %d %d %d\n",
		    dp->d_ino, dp->d_name, dp->d_off,
		    dp->d_reclen, XDR_GETPOS(xdrs), size);
#endif
		if (dp->d_ino == 0) {
			continue;
		}
		name = dp->d_name;
		for (cp = name; *cp; cp++)
			continue;
		namlen = cp - name;
#ifdef DIROFFBUG
		if (size - dp->d_reclen > 0) {
			offset = nextdp(dp)->d_off;
		} else {
			ASSERT(rd->rd_offset > offset);
			offset = rd->rd_offset;
		}
#endif
		if (!xdr_bool(xdrs, &true) ||
		    !xdr_u_long(xdrs, (u_long *) &dp->d_ino) ||
		    !xdr_bytes(xdrs, &name, &namlen, NFS_MAXNAMLEN) ||
#ifdef DIROFFBUG
		    !xdr_u_long(xdrs, &offset))
#else
		    !xdr_u_long(xdrs, (u_long *) &dp->d_off))
#endif
		{
			return (FALSE);
		}
		if (XDR_GETPOS(xdrs) - xdrpos >= rd->rd_bufsize) {
			rd->rd_eof = FALSE;
			break;
		}
	}
	if (!xdr_bool(xdrs, &false)) {
		return (FALSE);
	}
	if (!xdr_bool(xdrs, &rd->rd_eof)) {
		return (FALSE);
	}
	return (TRUE);
}

/*
 * DECODE ONLY
 */
bool_t
xdr_getrddirres(xdrs, rd)
	register XDR *xdrs;
	register struct nfsrddirres *rd;
{
	struct dirent *dp;
	register int size;
	register int reclen;
	auto bool_t valid;
	auto u_long offset;

	if (!xdr_enum(xdrs, (enum_t *)&rd->rd_status)) {
		return (FALSE);
	}
	if (rd->rd_status != NFS_OK) {
		return (TRUE);
	}

#ifdef NFSDEBUG
	dprint(nfsdebug, 6, "xdr_getrddirres: %s size %d\n",
	    xdropnames[(int)xdrs->x_op], rd->rd_size);
#endif
	size = rd->rd_size;
	dp = rd->rd_entries;
#ifdef DIROFFBUG
	offset = rd->rd_offset;
#endif
	for (;;) {
		auto u_short namlen;

		if (! xdr_bool(xdrs, &valid)) {
			return (FALSE);
		}
		if (! valid) {
			break;	/* end of entlist */
		}
#ifdef DIROFFBUG
		dp->d_off = offset;
#endif
		if (!xdr_u_long(xdrs, (u_long *) &dp->d_ino) ||
		    !xdr_u_short(xdrs, &namlen) ||	/* u_short?! */
		    ((reclen = DIRENTSIZE(namlen)) > size) ||
		    !xdr_opaque(xdrs, (caddr_t)dp->d_name, (u_int)namlen) ||
		    !xdr_u_long(xdrs, &offset)) {
#ifdef NFSDEBUG
			dprint(nfsdebug, 2,
			    "xdr_getrddirres: entry error\n");
#endif
			return (FALSE);
		}
#ifndef DIROFFBUG
		dp->d_off = offset;
#endif
		dp->d_reclen = reclen;
		dp->d_name[namlen] = '\0';
#ifdef NFSDEBUG
		dprint(nfsdebug, 10,
		    "xdr_getrddirres: entry %d %s(%d) %d %d\n",
		    dp->d_ino, dp->d_name, namlen,
		    dp->d_reclen, dp->d_off);
#endif
		size -= reclen;
		if (size <= 0) {
			return (FALSE);
		}
		dp = nextdp(dp);
	}
	if (!xdr_bool(xdrs, &rd->rd_eof)) {
		return (FALSE);
	}
	rd->rd_size = (char *)dp - (char *)rd->rd_entries;
	rd->rd_offset = offset;
#ifdef NFSDEBUG
	dprint(nfsdebug, 6,
	    "xdr_getrddirres: returning size %d offset %d eof %d\n",
	    rd->rd_size, rd->rd_offset, rd->rd_eof);
#endif
	return (TRUE);
}

/*
 * Arguments for directory operations
 */
bool_t
xdr_diropargs(xdrs, da)
	XDR *xdrs;
	struct nfsdiropargs *da;
{
#ifdef NFSDEBUG
	register char *name = da->da_name;
#endif

	if (xdr_fhandle(xdrs, &da->da_fhandle) &&
	    xdr_string(xdrs, &da->da_name, NFS_MAXNAMLEN) ) {
#ifdef NFSDEBUG
		if (name == NULL && da->da_name != NULL) {
			name = da->da_name;
		}
		dprint(nfsdebug, 6, "xdr_diropargs: %s %x %d '%s'\n",
		    xdropnames[(int)xdrs->x_op], da->da_fhandle.fh_fsid,
		    da->da_fhandle.fh_fno, name);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_diropargs: FAILED\n");
#endif
	return (FALSE);
}

/*
 * NFS_OK part of directory operation result
 */
bool_t
xdr_drok(xdrs, drok)
	XDR *xdrs;
	struct nfsdrok *drok;
{

	if (xdr_fhandle(xdrs, &drok->drok_fhandle) &&
	    xdr_fattr(xdrs, &drok->drok_attr) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_drok: FAILED\n");
#endif
	return (FALSE);
}

struct xdr_discrim diropres_discrim[2] = {
	{ (int)NFS_OK, xdr_drok },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Results from directory operation 
 */
bool_t
xdr_diropres(xdrs, dr)
	XDR *xdrs;
	struct nfsdiropres *dr;
{

	if (xdr_union(xdrs, (enum_t *)&(dr->dr_status),
	      (caddr_t)&(dr->dr_drok), diropres_discrim, xdr_void) ) {
#ifdef NFSDEBUG
		dprint(nfsdebug, 6, "xdr_diropres: %s stat %d\n",
		    xdropnames[(int)xdrs->x_op], dr->dr_status);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_diropres: FAILED\n");
#endif
	return (FALSE);
}

/*
 * Time structure
 */
bool_t
xdr_timeval(xdrs, tv)
	XDR *xdrs;
	struct timeval *tv;
{

	if (xdr_long(xdrs, &tv->tv_sec) &&
	    xdr_long(xdrs, &tv->tv_usec) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_timeval: FAILED\n");
#endif
	return (FALSE);
}

/*
 * arguments to setattr
 */
bool_t
xdr_saargs(xdrs, argp)
	XDR *xdrs;
	struct nfssaargs *argp;
{

	if (xdr_fhandle(xdrs, &argp->saa_fh) &&
	    xdr_sattr(xdrs, &argp->saa_sa) ) {
#ifdef NFSDEBUG
		dprint(nfsdebug, 6, "xdr_saargs: %s %x %d\n",
		    xdropnames[(int)xdrs->x_op], argp->saa_fh.fh_fsid,
		    argp->saa_fh.fh_fno);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_saargs: FAILED\n");
#endif
	return (FALSE);
}

/*
 * arguments to create and mkdir
 */
bool_t
xdr_creatargs(xdrs, argp)
	XDR *xdrs;
	struct nfscreatargs *argp;
{

	if (xdr_diropargs(xdrs, &argp->ca_da) &&
	    xdr_sattr(xdrs, &argp->ca_sa) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_creatargs: FAILED\n");
#endif
	return (FALSE);
}

/*
 * arguments to link
 */
bool_t
xdr_linkargs(xdrs, argp)
	XDR *xdrs;
	struct nfslinkargs *argp;
{

	if (xdr_fhandle(xdrs, &argp->la_from) &&
	    xdr_diropargs(xdrs, &argp->la_to) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_linkargs: FAILED\n");
#endif
	return (FALSE);
}

/*
 * arguments to rename
 */
bool_t
xdr_rnmargs(xdrs, argp)
	XDR *xdrs;
	struct nfsrnmargs *argp;
{

	if (xdr_diropargs(xdrs, &argp->rna_from) &&
	    xdr_diropargs(xdrs, &argp->rna_to) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_rnmargs: FAILED\n");
#endif
	return (FALSE);
}

/*
 * arguments to symlink
 */
bool_t
xdr_slargs(xdrs, argp)
	XDR *xdrs;
	struct nfsslargs *argp;
{

	if (xdr_diropargs(xdrs, &argp->sla_from) &&
	    xdr_string(xdrs, &argp->sla_tnm, (u_int)NFS_MAXPATHLEN) &&
	    xdr_sattr(xdrs, &argp->sla_sa) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_slargs: FAILED\n");
#endif
	return (FALSE);
}

/*
 * NFS_OK part of statfs operation
 */
xdr_fsok(xdrs, fsok)
	XDR *xdrs;
	struct nfsstatfsok *fsok;
{

	if (xdr_long(xdrs, (long *)&fsok->fsok_tsize) &&
	    xdr_long(xdrs, (long *)&fsok->fsok_bsize) &&
	    xdr_long(xdrs, (long *)&fsok->fsok_blocks) &&
	    xdr_long(xdrs, (long *)&fsok->fsok_bfree) &&
	    xdr_long(xdrs, (long *)&fsok->fsok_bavail) ) {
#ifdef NFSDEBUG
		dprint(nfsdebug, 6,
		    "xdr_fsok: %s tsz %d bsz %d blks %d bfree %d bavail %d\n",
		    xdropnames[(int)xdrs->x_op], fsok->fsok_tsize,
		    fsok->fsok_bsize, fsok->fsok_blocks, fsok->fsok_bfree,
		    fsok->fsok_bavail);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_fsok: FAILED\n");
#endif
	return (FALSE);
}

struct xdr_discrim statfs_discrim[2] = {
	{ (int)NFS_OK, xdr_fsok },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Results of statfs operation
 */
xdr_statfs(xdrs, fs)
	XDR *xdrs;
	struct nfsstatfs *fs;
{

	if (xdr_union(xdrs, (enum_t *)&(fs->fs_status),
	      (caddr_t)&(fs->fs_fsok), statfs_discrim, xdr_void) ) {
#ifdef NFSDEBUG
		dprint(nfsdebug, 6, "xdr_statfs: %s stat %d\n",
		    xdropnames[(int)xdrs->x_op], fs->fs_status);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_statfs: FAILED\n");
#endif
	return (FALSE);
}
