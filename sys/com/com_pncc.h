#ifndef __com_pncc__
#define	__com_pncc__
/*
 * Pathname component cache.
 *
 * The cache key is (parent directory, component name).  Cache value is
 * (child i-number, entry offset, capabilities).  Upon a cache hit, the user
 * may call iget(parent mount, child i-number).  If i-numbers have volatile
 * semantics for a filesystem, that filesystem may use the capabilities
 * to further qualify hits and misses.
 *
 * $Source: /d2/3.7/src/sys/com/RCS/com_pncc.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:26:45 $
 */
#include "../h/fstyp.h"		/* for struct dirlookupres */

typedef unsigned short	ncap_t;		/* capability type */

#if defined KERNEL || defined KMEM
/*
 * Public name cache value structure, an extension of the directory lookup
 * result object.
 */
struct ncvalue {
	struct dirlookupres ncv_dlres;	/* directory lookup result */
	ncap_t		ncv_pcap;	/* parent inumber capability */
	ncap_t		ncv_cap;	/* entry inumber capability */
};
#define	ncv_inum	ncv_dlres.dlr_inum
#define	ncv_offset	ncv_dlres.dlr_offset

/*
 * If tools which read /dev/kmem define KMEM before including this file,
 * they'll get up-to-date definitions of interesting structures and names.
 */
struct ncmeter {
	long	hits;		/* successful lookups */
	long	misses;		/* unsuccessful short-name lookups */
	long	steps;		/* hash chain search loop iterations */
	long	longlooks;	/* long name lookups (not counted as misses) */
	long	enters;		/* number of entry creates done */
	long	longents;	/* enters for long names (failed) */
	long	removes;	/* number of component names removed */
	long	devpurges;	/* number of purges by dev */
	long	purges;		/* number of total purges */
};

struct nclinks {			/* cache block base class */
	struct ncblock	*ncl_forw;	/* next and prev hash links */
	struct ncblock	*ncl_back;
	struct ncblock	*ncl_lruforw;	/* next and prev lru links */
	struct ncblock	*ncl_lruback;
};

/*
 * Each block represents one directory entry.  For the sake of both
 * simplicity and space efficiency, only names shorter than or as long as
 * NC_NAMLEN are cached.  An available block not on any hash chain is
 * indicated by a zero-length name.
 */
#define	NC_NAMLEN	14

struct ncblock {			/* cache linkage must come first */
	struct nclinks	nc_links;		/* hash and LRU links */
	ino_t		nc_pinum;		/* i-number of parent dir */
	dev_t		nc_pdev;		/* mount dev of parent dir */
	char		nc_name[NC_NAMLEN];	/* the entry name */
	struct ncvalue	nc_val;			/* cache value */
};
#define	nc_forw		nc_links.ncl_forw
#define	nc_back		nc_links.ncl_back
#define	nc_lruforw	nc_links.ncl_lruforw
#define	nc_lruback	nc_links.ncl_lruback
#define	nc_pcap		nc_val.ncv_pcap
#define	nc_cap		nc_val.ncv_cap
#define	nc_inum		nc_val.ncv_inum
#define	nc_offset	nc_val.ncv_offset

#ifdef KMEM
# define NM_NCMETER	"ncmeter"
# define NM_PNCC_BASE	"pncc_base"	/* base of cache block arena */
# define NM_PNCC_LIMIT	"pncc_limit"	/* limit of cache block arena */
# define NM_PNCC_SIZE	"pncc_size"	/* logical size thereof */
# define NM_PNCC_CAPGEN	"pncc_capgen"	/* capability generator seed */
# define NM_NC_HASH	"nc_hash"	/* cache hash table base */
# define NM_NC_LRU	"nc_lru"	/* cache lru list header */
#else
extern struct ncblock	*pncc_base;
extern struct ncblock	*pncc_limit;
extern short		pncc_size;
#endif	/* KMEM */

#endif	/* KERNEL || KMEM */

#ifdef KERNEL
/*
 * The result parameter filled in by a name cache lookup.
 */
struct ncentry {			/* public entry state */
	struct ncvalue	nce_val;	/* the cache lookup output */
	char		*nce_hash;	/* opaque hashing info */
};
#define	nce_pcap	nce_val.ncv_pcap
#define	nce_cap		nce_val.ncv_cap
#define	nce_dlres	nce_val.ncv_dlres
#define	nce_inum	nce_val.ncv_inum
#define	nce_offset	nce_val.ncv_offset

/*
 * Name cache operations.
 */
void	pncc_init();
ncap_t	pncc_newcap();

enum ncstat { NC_MISS, NC_HIT, NC_NOTCACHED };

/*
 * Look for (dp,name) in the cache, returning its value in entry if found,
 * NULL otherwise.  Unless NULL, the entry yields information to be used with
 * pncc_enter() or pncc_remove().
 *	struct inode	*dp;
 *	char		*name;
 *	int		len;
 *	struct ncentry	*entry;
 */
enum ncstat	pncc_lookup(/* dp, name, len, entry */);

/*
 * Make a name cache entry for (dp,name)->(entry->nce_val).  The entry
 * descriptor is got via pncc_lookup().
 */
enum ncstat	pncc_enter(/* dp, name, len, entry */);

/*
 * Remove the given entry from the name cache if and only if the capabilities
 * in entry match those of a cache entry.
 */
void	pncc_remove(/* entry */);

/*
 * Flush all entries refering to dev from the name cache.  If dev is NODEV,
 * flush the entire cache.  The device name used should be that of a mount
 * structure (not necessarily the same as the inode structure's device).
 *	dev_t	dev;
 */
void	pncc_purgedev(/* dev */);

#endif	/* KERNEL */

#endif	/* __com_pncc__ */
