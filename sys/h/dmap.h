/*
 * Definitions for the mapping of vitual swap
 * space to the physical swap area - the disk map.
 *
 * NDMAP is now defined in "machine/vmparam.h"
 *
 * $Source: /d2/3.7/src/sys/h/RCS/dmap.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:22 $
 */

struct	dmap {
	swblk_t	dm_size;	/* current size used by process (in blocks) */
	swblk_t	dm_alloc;	/* amount of swap space allocated (in blocks) */
	swblk_t	dm_map[NDMAP];	/* first disk block number in each chunk */
};

/*
 * The following structure is that ``returned''
 * from a call to vstodb().
 */
struct	dblock {
	swblk_t	db_base;	/* base of physical contig drum block */
	swblk_t	db_size;	/* size of block (in blocks) */
};
