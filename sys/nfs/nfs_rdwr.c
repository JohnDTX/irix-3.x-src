#ifndef lint
static	char	rcsid[] = "$Header: /d2/3.7/src/sys/nfs/RCS/nfs_rdwr.c,v 1.1 89/03/27 17:33:15 root Exp $";
#endif
/*
 * NFS read and write.
 *
 * $Source: /d2/3.7/src/sys/nfs/RCS/nfs_rdwr.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:15 $
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
# include "sys/file.h"
# include "sys/inode.h"
# include "sys/mount.h"
# include "sys/fs/nfs.h"
# include "sys/fs/nfs_clnt.h"
# include "sys/fstyp.h"
# include "sys/fs/com_pncc.h"
# include "sys/fs/rnode.h"
/*
# include "sys/conf.h"
*/
#else
# include "../h/param.h"
# include "../h/user.h"
# include "../h/buf.h"
# include "../h/file.h"
# include "../h/inode.h"
# include "../h/mount.h"
# include "../nfs/nfs.h"
# include "../nfs/nfs_clnt.h"
# include "../nfs/rnode.h"
#endif

#define	uiomove(base,length,rw,uiop) \
	(iomove(base, length, rw), uiop->u_error)

nfs_readi(ip)
	register struct inode *ip;
{
	if (ip->i_ftype == IFLNK) {
		u.u_error = nfs_readlink(ip, &u);
	} else {
		u.u_error = nfs_rdwr(ip, &u, B_READ);
	}
}

nfs_writei(ip)
	register struct inode *ip;
{
	u.u_error = nfs_rdwr(ip, &u, B_WRITE);
}

static int
nfs_rdwr(ip, uiop, rw)
	register struct inode *ip;
	register struct user *uiop;
	int rw;
{
	register int error = 0;
	register struct rnode *rp;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_rdwr: %s %x %d %s base %x count %d\n",
	    vtomi(ip)->mi_hostname,
	    vtor(ip)->r_nfsattr.na_fsid, vtor(ip)->r_nfsattr.na_nodeid,
	    rw == B_READ ? "READ" : "WRITE",
	    uiop->u_base, uiop->u_count);
#endif
	if (ip->i_ftype != IFREG) {
		return EACCES;
	}
	rp = vtor(ip);
	if (rw == B_WRITE
	    || (rw == B_READ && rp->r_cred.cr_ref == 0)) {
		crinit(uiop, &rp->r_cred);
	}

	if ((uiop->u_fmode & FMASK) == FAPPEND && rw == B_WRITE) {
		error = nfs_getattr(ip, &rp->r_cred);
		if (!error) {
			uiop->u_offset = ip->i_size;
		}
	}

	if (!error) {
		error = rwip(ip, uiop, rw);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_rdwr returning %d\n", error);
#endif
	return (error);
}

static int
rwip(ip, uio, rw)
	register struct inode *ip;
	register struct user *uio;
	int rw;
{
	register struct buf *bp;
	register struct rnode *rp;
	register daddr_t bn;
	register int len;
	register int n, boff;
	bool_t eof = 0;
	int size;
	int error = 0;

	if (uio->u_count == 0) {
		return (0);
	}
	if (NFS_WILDOFF(uio->u_offset)
	    || NFS_WILDOFF(uio->u_offset + uio->u_count)) {
		return (EINVAL);
	}
	if (rw == B_WRITE && ip->i_ftype == IFREG
	    && uio->u_offset >= (u_long) BBTOB(uio->u_limit)) {
		return (EFBIG);
	}
	size = vtoblksz(ip);
	if (size <= 0) {
		panic("rwip: zero size");
	}
	rp = vtor(ip);
	do {
		/*
		 * Scale the uio's offset into a basic block number for
		 * use with the extent-oriented bio.
		 */
		boff = uio->u_offset % size;
		bn = BTOBBT(uio->u_offset - boff);
		len = BTOBBT(size);
		n = MIN((unsigned)(size - boff), uio->u_count);
		if (rw == B_READ) {
			register off_t bytes_to_eof;

			if (bn < 0) {	/* hole in file - can't happen */
				bp = geteblk(len);
				clrbuf(bp);
			} else {
				if (incore(ip->i_dev, bn, len)) {
					/*
					 * get attributes to check whether in
					 * core data is stale
					 */
					(void) nfs_getattr(ip, &rp->r_cred);
				}
				if (rp->r_lastr + len == bn) {
					bp = breada(ip->i_dev, bn, len,
						bn + len, len);
				} else {
					bp = bread(ip->i_dev, bn, len);
				}
			}
			rp->r_lastr = bn;
			bytes_to_eof = ip->i_size - uio->u_offset;
			if (bytes_to_eof <= 0) {
				brelse(bp);
				return (0);
			}
			if (bytes_to_eof < n) {
				n = bytes_to_eof;
				eof = 1;
			}
		} else {	/* B_WRITE */
			if (rp->r_error) {
				error = rp->r_error;
				goto bad;
			}
			if (n == size) {
				bp = getblk(ip->i_dev, bn, len);
			} else {
				bp = bread(ip->i_dev, bn, len);
			}
		}
		if (bp->b_flags & B_ERROR) {
			error = bp->b_error;
			if (!error) {
				error = EIO;
			}
			brelse(bp);
			goto bad;
		}
#ifdef SVR0
		if (bp->b_flags & B_DMA) {	/* XXX sgi-ism */
			bio_dma_free(bp);
		}
#endif
		error = uiomove(bp->b_un.b_addr+boff, n, rw, uio);
#ifdef SVR3
		ASSERT(valusema(&ip->i_lock) <= 0);	/* still locked? */
#endif
		if (rw == B_READ) {
			ip->i_flag |= IACC;
			brelse(bp);
		} else {
			/*
			 * i_size is the maximum number of bytes known
			 * to be in the file.
			 * Make sure it is at least as high as the last
			 * byte we just wrote into the buffer.
			 */
			if (ip->i_size < uio->u_offset) {
				ip->i_size = uio->u_offset;
			}
			ip->i_flag |= IUPD | ICHG;
			rp->r_flags |= RDIRTY;
			if (n + boff == size) {
				bp->b_flags |= B_AGE;
				bawrite(bp);
			} else {
				bdwrite(bp);
			}
		}
	} while (uio->u_error == 0 && uio->u_count > 0 && !eof);
	if (rw == B_WRITE && uio->u_count && uio->u_error == 0) {
		printf("rwip: short write. resid %d ip %x bn %d\n",
		    uio->u_count, ip, bn);
	}
	if (error == 0) {
		error = uio->u_error;
	}
bad:
	return (error);
}

static int
nfs_readlink(ip, uiop)
	struct inode *ip;
	struct user *uiop;
{
	register int error;
	register struct ucred *credp;
	struct nfsrdlnres rl;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_readlink %s %x %d\n",
	    vtomi(ip)->mi_hostname,
	    vtor(ip)->r_nfsattr.na_fsid, vtor(ip)->r_nfsattr.na_nodeid);
#endif
	credp = &vtor(ip)->r_cred;
	if (credp->cr_ref == 0) {
		crinit(uiop, credp);
	}
	rl.rl_data = (char *)kmem_alloc((u_int)NFS_MAXPATHLEN);
	error =
	    rfscall(vtomi(ip), RFS_READLINK, xdr_fhandle, (caddr_t)vtofh(ip),
		xdr_rdlnres, (caddr_t)&rl, credp);
	if (!error) {
		error = geterrno(rl.rl_status);
		if (!error) {
			error = uiomove(rl.rl_data, (int)rl.rl_count,
			    B_READ, uiop);
		}
	}
	kmem_free((caddr_t)rl.rl_data, (u_int)NFS_MAXPATHLEN);
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_readlink: returning %d\n", error);
#endif
	return (error);
}

/*
 * NFS bio implementation
 */
#ifdef NOTDEF
#ifdef SVR3
#else
# include "../vm/vm.h"
# include "../h/map.h"
# include "../machine/pte.h"
#endif
#endif

struct buf *async_bufhead;
int async_daemon_count = 0;	/* total number of biods */
int async_curlength = 0;	/* current number of bufs on the async queue */
int async_maxlength;		/* maximum number of bufs on queue */

/*
 * The maximum queue length should be somewhat greater than the
 * total count of async daemons running to improve throughput.
 * The macro below uses a 1.2 magnification factor over the count.
 */
#define	compute_maxlen(count)	((5*(count))/4)

#ifdef SVR3
sema_t	async_wait;
lock_t	async_lock;
#else
int	async_waiters = 0;
#endif SVR3

#if !defined(SVR3) || defined(SIMPLEX)
# define ASYNC_LOCK()
# define ASYNC_UNLOCK()
#else
# define ASYNC_LOCK()		spsemahi(async_lock);
# define ASYNC_UNLOCK()		svsemax(async_lock);
#endif


int
nfs_strategy(bp)
	register struct buf *bp;
{
	register struct rnode *rp;
	register struct buf *bp1;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_strategy bp %x\n", bp);
#endif
#ifdef SVR3
	if (bp->b_flags & B_SWAP)
#else
	if (bp->b_flags & (B_DIRTY|B_UAREA|B_PAGET))
#endif
		panic("nfs_strategy: swapping to nfs");
	/*
	 * If there was an asynchronous write error on this rnode
	 * then we just return the old error code. This continues
	 * until the rnode goes away (zero ref count). We do this because
	 * there can be many procs writing this rnode.
	 */
	rp = bptor(bp);
	if (rp->r_error) {
		bp->b_error = rp->r_error;
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}
#ifdef R2300
	ASSERT((bp->b_flags & B_PHYS) == 0);	/* XXX not yet implemented */
#endif
#ifdef IP2
	ASSERT((bp->b_flags & B_PHYS) == 0);	/* XXX not yet implemented */
#endif
#ifdef vax
	if (bp->b_flags & B_PHYS) {
		register int npte;
		register int n;
		register long a;
		register struct pte *pte, *kpte;
		caddr_t va;
		int o;
		caddr_t saddr;

		/*
		 * Buffer's data is in userland, or in some other
		 * currently inaccessable place. We get a hunk of
		 * kernel address space and map it in.
		 */
		o = (int)bp->b_un.b_addr & PGOFSET;
		npte = btoc(bp->b_bcount + o);
		while ((a = rmalloc(kernelmap, (long)clrnd(npte))) == NULL) {
			kmapwnt++;
			sleep((caddr_t)kernelmap, PSWP+4);
		}
		kpte = &Usrptmap[a];
		pte = vtopte(bp->b_proc, btop(bp->b_un.b_addr));
		for (n = npte; n--; kpte++, pte++)
			*(int *)kpte = PG_NOACC | (*(int *)pte & PG_PFNUM);
		va = (caddr_t)kmxtob(a);
		vmaccess(&Usrptmap[a], va, npte);
		saddr = bp->b_un.b_addr;
		bp->b_un.b_addr = va + o;
		/*
		 * do the io
		 */
		do_bio(bp);
		/*
		 * Release kernel maps
		 */
		bp->b_un.b_addr = saddr;
		kpte = &Usrptmap[a];
		for (n = npte; n-- ; kpte++)
			*(int *)kpte = PG_NOACC;
		rmfree(kernelmap, (long)clrnd(npte), a);
	} else
#endif
	ASSERT(rp->r_iocount >= 0);
	rp->r_iocount++;

	if (bp->b_flags & B_ASYNC) {
		ASYNC_LOCK();
		if ((async_curlength < async_maxlength) ||
		    (bp->b_flags & B_BDFLUSH)) {
			if (async_bufhead) {
				bp1 = async_bufhead;
				while (bp1->av_forw) {
					bp1 = bp1->av_forw;
				}
				bp1->av_forw = bp;
			} else {
				async_bufhead = bp;
			}
			bp->av_forw = NULL;
			async_curlength++;
#ifdef SVR3
			cvsema(&async_wait);
#else
			if (async_waiters)
				wakeup((caddr_t) &async_bufhead);
#endif
			ASYNC_UNLOCK();
		} else {
			ASYNC_UNLOCK();
			do_bio(bp);
		}
	} else {
		do_bio(bp);
	}
}

/*
 * Drop an rnode's i/o count and issue a wakeup to anyone interested when it
 * decrements to zero.
 */
void
riodone(rp)
	register struct rnode *rp;
{
	ASSERT(rp->r_iocount > 0);
	if (--rp->r_iocount == 0 && (rp->r_flags & RIOWAIT)) {
		rp->r_flags &= ~RIOWAIT;
		wakeup((caddr_t) &rp->r_flags);
	}
}

/*
 * Macros for processing asynchronous buffers.  Since read-aheads are async
 * and may be staled by binval(), we must drop them with brelse().  Of course
 * the per-rnode i/o completion must be noticed also (riodone()).
 */
#ifdef SVR3
# define B_INVAL B_STALE
# define DO_BIO_WITH_SETUP(bp)	do_bio(bp)
#endif

#ifdef SVR0
# define DO_BIO_WITH_SETUP(bp) \
	if (!(bp->b_flags & B_KMAP)) {	/* XXX a la B_DMA */ \
		bio_allockmap(bp); \
	} \
	do_bio(bp)
#endif

#define	DO_BIO(bp, rp) \
	if ((bp->b_flags & B_INVAL) == 0) { \
		DO_BIO_WITH_SETUP(bp); \
	} else { \
		riodone(rp); \
		brelse(bp); \
	}

async_daemon()
{
	register struct buf *bp;

	ASYNC_LOCK();
	async_daemon_count++;
	async_maxlength = compute_maxlen(async_daemon_count);
	ASYNC_UNLOCK();

	for (;;) {
	
		ASYNC_LOCK();
		while (async_bufhead == NULL) {
#ifdef SVR3
			if (svpsema(async_lock, &async_wait, (PZERO+1)|PCATCH)) {
#else
			async_waiters++;
			if (sleep((caddr_t)&async_bufhead, (PZERO+1)|PCATCH)) {
				async_waiters--;
#endif
				ASYNC_LOCK();
				async_daemon_count--;
				async_maxlength =
					compute_maxlen(async_daemon_count);
				ASYNC_UNLOCK();
				u.u_error = EINTR;
				exit(0);
			}
#ifndef SVR3
			async_waiters--;
#endif
			ASYNC_LOCK();
		}
		bp = async_bufhead;
		async_bufhead = bp->av_forw;
		async_curlength--;
		ASSERT(async_curlength >= 0);
		ASYNC_UNLOCK();

 		ASSERT(bp->b_flags & B_BUSY);
 		ASSERT(bp->b_flags & B_ASYNC);
		DO_BIO(bp, bptor(bp));
	}
}

/*
 * Complete any asynchronous I/O for the given dev:  seek through
 * the async daemon buffer list for matching buffers.  If any,
 * pull them off the list and complete the I/O synchronously.
 * If we've gone all through the list and there is still some
 * I/O pending on the rnode, wait for it to complete.
 *
 * We can't call bdwait for fear of being hung by biod death.
 * Also, bdwait waits for *all* asynchronous writes  for every
 * file in the system to finish.  Because asynchronous I/O may
 * already be in progress, we must make use of the rnode's
 * r_iocount/RIOWAIT mechanism to sleep till they complete.
 */
nfs_bflushw(rp, dev)
	register struct rnode *rp;
	register dev_t dev;
{
	register struct buf *bp, **bpp;

top:
	bflush(dev);	/* turn delayed writes into async writes */
	for (;;) {
		bpp = &async_bufhead;
		ASYNC_LOCK();
		for (;;) {
			bp = *bpp;
			if (bp == NULL) {
				ASYNC_UNLOCK();
				ASSERT(rp->r_iocount >= 0);
				if (rp->r_iocount > 0) {
					rp->r_flags |= RIOWAIT;
					sleep((caddr_t) &rp->r_flags, PRIBIO);
					goto top;
				}
				return;
			}
			if (bp->b_dev == dev) {
				break;
			}
			bpp = &bp->av_forw;
		}
		*bpp = bp->av_forw;
		async_curlength--;
		ASYNC_UNLOCK();

 		ASSERT(bp->b_flags & B_BUSY);
 		ASSERT(bp->b_flags & B_ASYNC);
		DO_BIO(bp, rp);
	}
	/* NOTREACHED */
}

static int
do_bio(bp)
	register struct buf *bp;
{
	register struct rnode *rp;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4,
	    "do_bio: addr %x, blk %d, offset %d, size %d, B_READ %d\n",
	    bp->b_un.b_addr, bp->b_blkno, BBTOB(bp->b_blkno),
	    bp->b_bcount, bp->b_flags & B_READ);
#endif
	rp = bptor(bp);
	ASSERT(rtov(rp) && bptorn(bp) == rtov(rp)->i_number);
	if ((bp->b_flags & B_READ) == B_READ) {
		auto int resid;

		bp->b_error = nfsread(rtov(rp), bp->b_un.b_addr,
			BBTOB(bp->b_blkno), (int)bp->b_bcount,
			(int *)&resid, &rp->r_cred);
		if (bp->b_error == 0) {
			if (resid) {
				bzero(bp->b_un.b_addr + bp->b_bcount - resid,
				      resid);
			}
			ASSERT(rtov(rp)->i_size == rp->r_nfsattr.na_size);
		}
	} else if (!rp->r_error) {
		register int count;

		count =
		    MIN(bp->b_bcount, rtov(rp)->i_size - BBTOB(bp->b_blkno));
		if (count < 0) {	/* XXX */
			panic("do_bio: write count < 0");
		}
		bp->b_error = nfswrite(rtov(rp), bp->b_un.b_addr,
			BBTOB(bp->b_blkno), count, &rp->r_cred);
		/*
		 * If the write fails and it was asynchronous
		 * all future writes will get an error.
		 */
		if (bp->b_error && bp->b_flags & B_ASYNC) {
			rp->r_error = bp->b_error;
		}
	} else {
		bp->b_error = rp->r_error;
	}
	if (bp->b_error) {
		bp->b_flags |= B_ERROR;
	}
	riodone(rp);
	iodone(bp);
}

/*
 * Read from a file.
 * Reads data in largest chunks possible and passes full size also so
 * server can do read ahead if it wants to.
 */
static int
nfsread(ip, base, offset, count, residp, cred)
	struct inode *ip;
	register caddr_t base;
	register int offset;
	register int count;
	int *residp;
	struct ucred *cred;
{
	register int error;
	register int tsize;
	struct nfsreadargs ra;
	struct nfsrdresult rr;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfsread %s %x %d offset %d, totcount %d\n",
	    vtomi(ip)->mi_hostname,
	    vtor(ip)->r_nfsattr.na_fsid, vtor(ip)->r_nfsattr.na_nodeid,
	    offset, count);
#endif
	do {
		rr.rr_data = base;
		ra.ra_fhandle = *vtofh(ip);
		ra.ra_offset = offset;
		tsize = MIN(vtomi(ip)->mi_tsize, count);
		ra.ra_totcount = tsize;
		ra.ra_count = tsize;
		error = rfscall(vtomi(ip), RFS_READ, xdr_readargs, (caddr_t)&ra,
			xdr_rdresult, (caddr_t)&rr, cred);
		if (!error) {
			error = geterrno(rr.rr_status);
		}
		base += rr.rr_count;
		offset += rr.rr_count;
		count -= rr.rr_count;
	} while (!error && count > 0 && rr.rr_count == tsize);
	*residp = count;
	if (!error) {
		nfs_attrcache(ip, &rr.rr_attr, SFLUSH);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfsread: returning %d, resid %d\n",
		error, *residp);
#endif
	return (error);
}

/*
 * Write to file.
 * Writes to remote server in largest size chunks that the server can
 * handle.  Write is synchronous.
 */
static int
nfswrite(ip, base, offset, count, cred)
	struct inode *ip;
	register caddr_t base;
	register int offset;
	register int count;
	struct ucred *cred;
{
	register int error;
	register int tsize;
	struct nfswriteargs wa;
	struct nfsattrstat *ns;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfswrite %s %x %d offset %d, count %d\n",
	    vtomi(ip)->mi_hostname,
	    vtor(ip)->r_nfsattr.na_fsid, vtor(ip)->r_nfsattr.na_nodeid,
	    offset, count);
#endif
	ns = (struct nfsattrstat *)kmem_alloc((u_int)sizeof(*ns));
	do {
		wa.wa_data = base;
		wa.wa_fhandle = *vtofh(ip);
		wa.wa_begoff = offset;
		tsize = MIN(vtomi(ip)->mi_stsize, count);
		wa.wa_totcount = tsize;
		wa.wa_count = tsize;
		wa.wa_offset = offset;
		error = rfscall(vtomi(ip), RFS_WRITE, xdr_writeargs,
			(caddr_t)&wa, xdr_attrstat, (caddr_t)ns, cred);
		if (!error) {
			error = geterrno(ns->ns_status);
		}
#ifdef NFSDEBUG
		dprint(nfsdebug, 3, "nfswrite: sent %d of %d, error %d\n",
		    tsize, count, error);
#endif
		base += tsize;
		offset += tsize;
		count -= tsize;
	} while (!error && count > 0);
	if (!error) {
		nfs_attrcache(ip, &ns->ns_attr, NOFLUSH);
	}
	kmem_free((caddr_t)ns, (u_int)sizeof(*ns));
	switch (error) {
	case 0:
	case EDQUOT:
	case EINTR:
		break;

	case ENOSPC:
		printf("NFS write error: on host %s remote file system full\n",
		   vtomi(ip)->mi_hostname );
		break;

	default:
		printf("NFS write error %d on host %s fs %x file %d\n",
		    error, vtomi(ip)->mi_hostname, vtor(ip)->r_nfsattr.na_fsid,
		    vtor(ip)->r_nfsattr.na_nodeid);
		break;
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfswrite: returning %d\n", error);
#endif
	return (error);
}
