/* NFSSRC @(#)types.h	2.1 86/04/14 */
/*      @(#)types.h 1.1 86/02/03 SMI      */
#ifndef	_rpc_types_
#define	_rpc_types_

/*
 * Rpc additions to <sys/types.h>
 */

#define	bool_t	int
#define	enum_t	int
#define	FALSE	(0)
#define	TRUE	(1)
#define __dontcare__	-1

#ifndef KERNEL
# define mem_alloc(bsize)	malloc(bsize)
# define mem_free(ptr, bsize)	free(ptr)
char	*malloc();
void	free();
# ifndef major		/* ouch! */
#  include <sys/types.h>
# endif
#else
# ifdef SVR3
#  include "sys/types.h"
#  include "sys/fs/nfs_compat.h"
# else
#  include "../nfs/nfs_compat.h"
# endif
# define mem_alloc(bsize)	kmem_alloc((u_int)bsize)
# define mem_free(ptr, bsize)	kmem_free((caddr_t)(ptr), (u_int)(bsize))
#endif

#endif	/* _rpc_types_ */
