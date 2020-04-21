/* NFSSRC @(#)authunix_prot.c	2.2 86/04/14 */
#ifndef lint
static char sccsid[] = "@(#)authunix_prot.c 1.1 86/02/03 Copyr 1984 Sun Micro";
#endif

/*
 * authunix_prot.c
 * XDR for UNIX style authentication parameters for RPC
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#ifdef KERNEL
# ifdef SVR3
#  include "sys/debug.h"
#  include "sys/types.h"
#  include "sys/sysmacros.h"
#  include "sys/param.h"
#  include "sys/systm.h"
#  include "sys/signal.h"
#  include "sys/errno.h"
#  include "sys/psw.h"
#  include "sys/pcb.h"
#  include "sys/user.h"
#  include "sys/immu.h"
#  include "sys/region.h"
#  include "sys/sbd.h"
#  include "sys/proc.h"
#  include "sys/utsname.h"
#  include "rpc/types.h"
#  include "rpc/xdr.h"
#  include "rpc/auth.h"
#  include "rpc/auth_unix.h"
# else
#  include "../h/param.h"
#  include "../h/systm.h"
#  include "../h/user.h"
#  ifndef sgi
#   include "../h/kernel.h"
#  endif
#  include "../h/proc.h"
#  include "../rpc/types.h"
#  include "../rpc/xdr.h"
#  include "../rpc/auth.h"
#  include "../rpc/auth_unix.h"
# endif
#else
#include "types.h"	/* <> */
#include "xdr.h"	/* <> */
#include "auth.h"	/* <> */
#include "auth_unix.h"	/* <> */
#endif

/*
 * XDR for unix authentication parameters.
 */
bool_t
xdr_authunix_parms(xdrs, p)
	register XDR *xdrs;
	register struct authunix_parms *p;
{

	if (xdr_u_long(xdrs, &(p->aup_time))
	    && xdr_string(xdrs, &(p->aup_machname), MAX_MACHINE_NAME)
	    && xdr_int(xdrs, &(p->aup_uid))
	    && xdr_int(xdrs, &(p->aup_gid))
	    && xdr_array(xdrs, (caddr_t *)&(p->aup_gids),
		    &(p->aup_len), NGRPS, sizeof(int), xdr_int) ) {
		return (TRUE);
	}
	return (FALSE);
}

#ifdef KERNEL
/*
 * XDR kernel unix auth parameters.
 * Goes out of the u struct directly.
 * NOTE: this is an XDR_ENCODE only routine.
 */
xdr_authkern(xdrs)
	register XDR *xdrs;
{
#ifdef sgi
	register int	*gp;
	auto int	uid = u.u_uid;
	auto int	gid = u.u_gid;
	auto u_int	len;
	auto caddr_t	groups;
	auto char	*name = hostname;
	static	int	u_groups[NGRPS];
#else
 	int	*gp;
 	int	 uid = u.u_uid;
 	int	 gid = u.u_gid;
 	int	 len;
 	caddr_t	groups;
 	char	*name = hostname;
#endif

	if (xdrs->x_op != XDR_ENCODE) {
		return (FALSE);
	}

#ifdef sgi
	for (gp = &u_groups[NGRPS]; gp > u_groups; gp--) {
#else
 	for (gp = &u.u_groups[NGRPS]; gp > u.u_groups; gp--) {
#endif
		if (gp[-1] >= 0) {
			break;
		}
	}
#ifdef sgi
	len = gp - u_groups;
	groups = (caddr_t)u_groups;
        if (xdr_u_long(xdrs, (u_long *) &time)
#else
 	len = gp - u.u_groups;
 	groups = (caddr_t)u.u_groups;
        if (xdr_u_long(xdrs, (u_long *)&time.tv_sec)
#endif
            && xdr_string(xdrs, &name, MAX_MACHINE_NAME)
            && xdr_int(xdrs, &uid)
            && xdr_int(xdrs, &gid)
	    && xdr_array(xdrs, &groups, &len, NGRPS, sizeof(int), xdr_int) ) {
                return (TRUE);
	}
	return (FALSE);
}
#endif
