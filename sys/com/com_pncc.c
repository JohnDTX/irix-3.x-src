/*
 * Common filesystem pathname component cache implementation.
 *
 * $Source: /d2/3.7/src/sys/com/RCS/com_pncc.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:26:45 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/conf.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/mount.h"
#include "../h/nami.h"
#include "../h/buf.h"
#include "../h/file.h"
#include "../h/user.h"
#include "../com/com_pncc.h"

struct ncblock	*pncc_base;	/* base of cache block arena */
struct ncblock	*pncc_limit;	/* limit of cache block arena */
extern short	pncc_size;	/* logical size thereof */

static ncap_t pncc_capgen = 0;	/* capability generator seed */

/*
 * Fabricate a new, unique capability.  Every capable cache entry has a
 * child inumber capability between 1 and the ncap_t upper bound.
 */
ncap_t
pncc_newcap()
{
	if (++pncc_capgen == 0) {
		pncc_purgedev(NODEV);
		pncc_capgen = 1;
	}
	return pncc_capgen;
}

#ifdef OS_METER
static struct ncmeter ncmeter;
#endif

/*
 * Test whether an name cache block is valid and invalidate an entry.
 */
#define	isvalid(nc) \
	((nc)->nc_name[0] != '\0')
#define	invalidate(nc) \
	((nc)->nc_name[0] = '\0')

/*
 * Cache hash lookup.  Hash chains are null-terminated, while the LRU
 * list is circularly linked.
 */
#define NC_NAMEBITS	7
#define NC_NBUCKETS	(1<<NC_NAMEBITS)
#define NC_NAMEMASK	(NC_NBUCKETS-1)

struct ncbucket {			/* cache bucket header */
	struct ncblock	*nc_chain;	/* this is dereferenced as nc_forw */
};
static struct ncbucket	nc_hash[NC_NBUCKETS];

static struct nclinks	nc_lru;		/* LRU list header */

/*
 * Hash function for bucket index.  We use the first, middle, and last
 * characters of the name, thus distinguishing permutations while avoiding
 * the missing end-of-string problem.
 */
#define NC_BUCKET(dp, name, len) \
	(&nc_hash[((int)(dp)->i_mntdev \
		   + (dp)->i_number \
		   + (((((name)[0]<<1) \
			 + (name)[(len)>>1])<<1) \
		      + (name)[(len)-1]) \
		   + (len)) \
		  & NC_NAMEMASK])

/*
 * Insert nc in the hash chain pointed to by hp.  Remove nc from its chain.
 */
#define	INSHASH(nc, hp) { \
	if (((nc)->nc_forw = (hp)->nc_chain) != NULL) \
		(nc)->nc_forw->nc_back = (nc); \
	(nc)->nc_back = (struct ncblock *)(hp); \
	(hp)->nc_chain = (nc); \
}
#define	REMHASH(nc) \
	if (((nc)->nc_back->nc_forw = (nc)->nc_forw) == NULL) \
		/* nothing to do */; \
	else \
		(nc)->nc_forw->nc_back = (nc)->nc_back

/*
 * Name cache block lru-list operations.
 */
#define	INSLRU(nc, lp) { \
	(nc)->nc_lruforw = ((struct ncblock *) (lp))->nc_lruforw; \
	(nc)->nc_lruback = (struct ncblock *) (lp); \
	(nc)->nc_lruforw->nc_lruback \
	    = (nc)->nc_lruback->nc_lruforw = (nc); \
}
#define	REMLRU(nc) { \
	(nc)->nc_lruforw->nc_lruback = (nc)->nc_lruback; \
	(nc)->nc_lruback->nc_lruforw = (nc)->nc_lruforw; \
}

/*
 * Initialize the pathname component cache, getting memory from the kernel's
 * dynamic memory allocator.
 */
void
pncc_init()
{
	register struct ncblock *nc, *lrup;
	register int n;

	if ((n = pncc_size) == 0) {
		return;		/* the cache is configured off */
	}
	lrup = (struct ncblock *) &nc_lru;
	lrup->nc_lruforw = lrup->nc_lruback = lrup;
	nc = pncc_base;
	while (--n >= 0) {
		INSLRU(nc, lrup);
		nc++;
	}
	ASSERT(nc == pncc_limit);
}

/*
 * Look for a name cache entry matching name, with parent directory dp.
 *	don't lookup overlong names
 *	find the hash bucket for (dp, name)
 *	search that bucket's chain
 *	if found move to tail of freelist
 * Return a pointer to the cache entry if a version was found.  Otherwise,
 * return NULL.  Return in hpp an opaque pointer to the appropriate hash
 * bucket for later use if this name can be entered in the cache.
 */
enum ncstat
pncc_lookup(dp, name, len, nce)
	register struct inode *dp;
	register char *name;
	register int len;
	register struct ncentry *nce;
{
	register struct ncblock *nc;
	register struct ncbucket *hp;
	enum ncstat stat;

	if (len > NC_NAMLEN) {
		METER(ncmeter.longlooks++);
		return NC_NOTCACHED;
	}
	if (pncc_size == 0) {
		return NC_NOTCACHED;	/* the cache has been turned off */
	}
	hp = NC_BUCKET(dp, name, len);
	nc = hp->nc_chain;
	for (;;) {
		if (nc == NULL) {
			METER(ncmeter.misses++);
			stat = NC_MISS;
			break;
		}
		if (nc->nc_pinum == dp->i_number
		    && nc->nc_pdev == dp->i_mntdev->m_dev
		    && !strncmp(nc->nc_name, name, NC_NAMLEN)) {
			METER(ncmeter.hits++);
			stat = NC_HIT;
			REMLRU(nc);
			INSLRU(nc, nc_lru.ncl_lruback);
			break;
		}
		METER(ncmeter.steps++);
		nc = nc->nc_forw;
	}
	if (nce != NULL) {
		if (nc == NULL) {
			/* miss, so flag nce as incapable */
			nce->nce_cap = 0;
		} else {
			nce->nce_val = nc->nc_val;
		}
		nce->nce_hash = (char *) hp;
	}
	return stat;
}

/*
 * Create a new name cache entry for name and inum with parent dp.
 *	grab an available cache entry from the head of the freelist
 *	if it was on a hash chain, unlink it from its old chain
 *	fill in its new data
 *	put it on its new hash chain
 *	put it on the tail of the freelist
 */
enum ncstat
pncc_enter(dp, name, len, nce)
	register struct inode *dp;
	register char *name;
	register int len;
	register struct ncentry *nce;
{
	register struct ncblock *nc;

	/* no one should call us if the cache has been turned off */
	ASSERT(pncc_size != 0);
	if (nce->nce_hash == NULL || len > NC_NAMLEN) {
		METER(ncmeter.longents++);
		return NC_NOTCACHED;
	}
	METER(ncmeter.enters++);
	nc = nc_lru.ncl_lruforw;
	ASSERT(nc != NULL);		/* cache must be on */
	REMLRU(nc);
	if (isvalid(nc)) {
		ASSERT(nc->nc_back);
		REMHASH(nc);
	}
	nc->nc_pdev = dp->i_mntdev->m_dev;
	nc->nc_pinum = dp->i_number;
	strncpy(nc->nc_name, name, NC_NAMLEN);
	if (nce->nce_cap == 0) {
		/*
		 * If the user failed to assign at least the entry inumber
		 * capability, then assign a new capability.  Pncc_newcap()
		 * may call pncc_purge(), so it is imperative that nc not
		 * be on the lru list.
		 */
		nce->nce_cap = pncc_newcap();
	}
	nc->nc_val = nce->nce_val;
	INSHASH(nc, (struct ncbucket *) nce->nce_hash);
	INSLRU(nc, nc_lru.ncl_lruback);
	return NC_HIT;
}

/*
 * Remove the block pointed at by nc from the cache.
 *	remove entry from hash chain
 *	move it to head of freelist
 *	invalidate
 */
#define	REMOVE(nc) { \
	REMHASH(nc); \
	REMLRU(nc); \
	INSLRU(nc, &nc_lru); \
	invalidate(nc); \
}

/*
 * To find the target entry on the hash chain nce->nce_hash, it is sufficient
 * to compare capabilities, *provided* we require that all pncc users set at
 * least one of the capabilities to a cache-unique value.  See the code in
 * pncc_enter() which sets a default child inumber capability.
 */
void
pncc_remove(nce)
	register struct ncentry *nce;
{
	register struct ncblock *nc;

	/* no one should call us if the cache has been turned off */
	ASSERT(pncc_size != 0);
	ASSERT(nce != NULL);
	ASSERT(nce->nce_cap != 0);
	for (nc = ((struct ncbucket *) nce->nce_hash)->nc_chain;
	     nc != NULL; nc = nc->nc_forw) {
		if (nc->nc_cap == nce->nce_cap
		    && nc->nc_pcap == nce->nce_pcap) {
			METER(ncmeter.removes++);
			REMOVE(nc);
		}
	}
}

/*
 * Invalidate name cache entries for this device.
 */
void
pncc_purgedev(dev)
	register dev_t dev;
{
	register struct ncblock *nc, *nc2;

	if (pncc_size == 0) {
		return;		/* the cache has been turned off */
	}
	METER(dev == NODEV ? ncmeter.purges++ : ncmeter.devpurges++);
	for (nc = nc_lru.ncl_lruforw; nc != (struct ncblock *) &nc_lru;
	     nc = nc2) {
		nc2 = nc->nc_lruforw;
		if ((nc->nc_pdev == dev || NODEV == dev)
		    && isvalid(nc)) {
			REMOVE(nc);
		}
	}
}
