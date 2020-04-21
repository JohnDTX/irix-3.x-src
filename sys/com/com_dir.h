#ifndef __com_dir__
#define	__com_dir__
/*
 * Common filesystem directory interface specification.
 * The directory entry structure and operations declared below constitute
 * an abstract layer imposed between namei() and filesystem-dependent
 * directory code.  This layer provides pathname component caching and a
 * directory lookup scan rotor.
 *
 * $Source: /d2/3.7/src/sys/com/RCS/com_dir.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:26:39 $
 */
#include "../com/com_pncc.h"

/*
 * An abstract directory entry suitable for passing parameters efficiently
 * between com_namei() and the common directory/name-cache functions.
 * From the bottom up, this structure is derived thus:
 *	entry				common directory entry
 *	    ncentry			public name cache entry
 *		ncvalue			name cache value
 *		    dirlookupres	directory lookup result
 */
struct entry {
	struct ncentry	e_ncent;	/* name cache value */
	enum ncstat	e_ncstat;	/* name cache lookup status */
	struct inode	*e_ip;		/* inode when not just looking up */
	char		*e_name;	/* entry filename */
	unsigned short	e_namlen;	/* name length */
};
#define	e_inum		e_ncent.nce_inum
#define	e_offset	e_ncent.nce_offset
#define	e_ncval		e_ncent.nce_val
#define	e_dlres		e_ncval.ncv_dlres

struct scanmeter {
	long	rotates;	/* # times scan was based on rotor */
	long	rotsaves;	/* # times offset saved as rotor */
	long	wraps;		/* # wraps around end of dir */
};

#ifdef KERNEL
/* dirlookup() how-to flags */
#define	DLF_IGET	0x1	/* get entry inode if name lookup succeeds */
#define	DLF_SCAN	0x2	/* use and update directory lookup rotor */

/*
 * Directory entry operations.  dirlookup() fills in ep, setting e_inum to
 * the found entry's i-number iff !(flags & DLF_IGET), otherwise getting an
 * inode for the entry's i-number or using dp in the case of ".", and in any
 * case returning 0 in e_inum and e_ip if the named entry is unlinked.
 *	struct inode	*dp, *ip, *pdp;
 *	char		*name;
 *	unsigned short	namlen;
 *	int		flags;
 *	struct entry	*ep;
 *	dflag_t		*dflagp;
 */
int	dirlookup(/* dp, name, namlen, flags, ep */);
int	direnter(/* dp, ip, ep */);
int	dirremove(/* dp, ep */);
int	dirinit(/* dp, pdp */);
int	dirisempty(/* dp, dflagp */);

/* remove any name cache reference for an existent rename target */
#define	dirpurgename(ep) \
	if ((ep)->e_ncstat == NC_HIT) \
		pncc_remove(&(ep)->e_ncent); \
	else

#endif	/* KERNEL */
#endif	/* __com_dir__ */
