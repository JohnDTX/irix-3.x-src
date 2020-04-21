#ifndef lint
static	char	rcsid[] = "$Header: /d2/3.7/src/sys/nfs/RCS/nfs_dir.c,v 1.1 89/03/27 17:33:09 root Exp $";
#endif
/*
 * NFS directory operations.
 *
 * $Source: /d2/3.7/src/sys/nfs/RCS/nfs_dir.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:09 $
 */
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
# include "sys/inode.h"
# include "sys/mount.h"
# include "sys/fs/nfs.h"
# include "sys/fs/nfs_clnt.h"
# include "sys/fstyp.h"
# include "sys/fs/com_pncc.h"
# include "sys/fs/rnode.h"
#else
# include "../h/param.h"
# include "../h/user.h"
# include "../h/buf.h"
# include "../h/inode.h"
# include "../h/mount.h"
# include "../nfs/nfs.h"
# include "../nfs/nfs_clnt.h"
# include "../nfs/rnode.h"
#endif

/*
 * Read directory entries.
 *
 * The NFS xdr primitive for deserializing directory results returns the
 * entries in dirent.h format.  Thus this routine merely makes a remote
 * read call of min(bufsize, transfer_size) bytes.
 */
int
nfs_getdents(dp, bufbase, bufsize)
	struct inode *dp;
	caddr_t bufbase;
	int bufsize;
{
	register int error = 0;
	register unsigned int count;
	struct nfsrddirargs rda;
	struct nfsrddirres  rd;
	register struct rnode *rp;
	struct ucred cred;
	register struct user *uiop = &u;
	struct buf *bp;

	rp = vtor(dp);
	if ((rp->r_flags & REOF)
	    && (dp->i_size == (u_long)uiop->u_offset)) {
		return (0);
	}
	count = bufsize;
#ifdef NFSDEBUG
	dprint(nfsdebug, 4,
	    "nfs_getdents %s %x %d count %d blksz %d, offset %d\n",
	    vtomi(dp)->mi_hostname, vtor(dp)->r_nfsattr.na_fsid,
	    vtor(dp)->r_nfsattr.na_nodeid, count, vtoblksz(dp),
	    uiop->u_offset);
#endif
	count = MIN(count, vtomi(dp)->mi_tsize);
	rda.rda_count = count;
	rda.rda_offset = uiop->u_offset;
	rda.rda_fh = *vtofh(dp);
	rd.rd_offset = uiop->u_offset;	/* value-result rfscall parameter */
	rd.rd_size = count;
	bp = geteblk(BTOBB(count));
	rd.rd_entries = (struct dirent *)bp->b_un.b_addr;
	crinit(uiop, &cred);

	error = rfscall(vtomi(dp), RFS_READDIR, xdr_rddirargs, (caddr_t)&rda,
	    xdr_getrddirres, (caddr_t)&rd, &cred);
	if (!error) {
		error = geterrno(rd.rd_status);
	}
	if (!error) {
		/*
		 * move dir entries to user land
		 */
		if (rd.rd_size) {
			if (copyout((caddr_t)rd.rd_entries, bufbase,
			    (int) rd.rd_size)) {
				error = EFAULT;
			}
#ifdef NOTDEF
			rda.rda_offset = rd.rd_offset;
#endif
			uiop->u_offset = rd.rd_offset;
		}
		if (rd.rd_eof) {
			rp->r_flags |= REOF;
			dp->i_size = uiop->u_offset;
		}
	}
	brelse(bp);
	if (error) {
		uiop->u_error = error;
		rd.rd_size = -1;
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_getdents: returning %d error %d offset %d\n",
	    rd.rd_size, error, uiop->u_offset);
#endif
	return (rd.rd_size);
}
