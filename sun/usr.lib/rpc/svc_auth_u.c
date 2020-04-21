/* NFSSRC @(#)svc_auth_unix.c	2.1 86/04/14 */
#ifndef lint
static char sccsid[] = "@(#)svc_auth_unix.c 1.1 86/02/03 Copyr 1984 Sun Micro";
#endif

/*
 * svc_auth_unix.c
 * Handles UNIX flavor authentication parameters on the service side of rpc.
 * There are two svc auth implementations here: AUTH_UNIX and AUTH_SHORT.
 * _svcauth_unix does full blown unix style uid,gid+gids auth,
 * _svcauth_short uses a shorthand auth to index into a cache of longhand auths.
 * Note: the shorthand has been gutted for efficiency.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#ifdef KERNEL
# ifdef SVR3
#  include "sys/param.h"
#  include "sys/sysmacros.h"
#  include "sys/systm.h"
#  include "sys/time.h"
#  include "netinet/in.h"
#  include "rpc/types.h"
#  include "rpc/xdr.h"
#  include "rpc/auth.h"
#  include "rpc/clnt.h"
#  include "rpc/rpc_msg.h"
#  include "rpc/svc.h"
#  include "rpc/auth_unix.h"
#  include "rpc/svc_auth.h"
# else
#  include "../h/param.h"
#  ifdef sgi
#   include "../h/systm.h"
#   include "../bsd/time.h"
#  else
#   include "../h/time.h"
#   include "../h/kernel.h"
#  endif
#  include "../netinet/in.h"
#  include "../rpc/types.h"
#  include "../rpc/xdr.h"
#  include "../rpc/auth.h"
#  include "../rpc/clnt.h"
#  include "../rpc/rpc_msg.h"
#  include "../rpc/svc.h"
#  include "../rpc/auth_unix.h"
#  include "../rpc/svc_auth.h"
# endif
#else
#include <stdio.h>
#include "types.h"	/* <> */
#include <sys/time.h>
#include <netinet/in.h>
#include "xdr.h"	/* <> */
#include "auth.h"	/* <> */
#include "clnt.h"	/* <> */
#include "rpc_msg.h"	/* <> */
#include "svc.h"	/* <> */
#include "auth_unix.h"	/* <> */
#include "svc_auth.h"	/* <> */
char *mem_alloc();
#endif

#define	RNDUP(x)  ((((x) + BYTES_PER_XDR_UNIT - 1) / BYTES_PER_XDR_UNIT) \
		   * BYTES_PER_XDR_UNIT)

/*
 * Unix longhand authenticator
 */
enum auth_stat
_svcauth_unix(rqst, msg)
	register struct svc_req *rqst;
	register struct rpc_msg *msg;
{
	register enum auth_stat stat;
	XDR xdrs;
	register struct authunix_parms *aup;
	register long *buf;
	struct area {
		struct authunix_parms area_aup;
		char area_machname[MAX_MACHINE_NAME];
		int area_gids[NGRPS];
	} *area;
	u_int auth_len;
	int str_len, gid_len;
	register int i;

	area = (struct area *) rqst->rq_clntcred;
	aup = &area->area_aup;
	aup->aup_machname = area->area_machname;
	aup->aup_gids = area->area_gids;
	auth_len = (u_int)msg->rm_call.cb_cred.oa_length;
	xdrmem_create(&xdrs, msg->rm_call.cb_cred.oa_base, auth_len,XDR_DECODE);
	buf = XDR_INLINE(&xdrs, auth_len);
	if (buf != NULL) {
		aup->aup_time = IXDR_GET_LONG(buf);
		str_len = IXDR_GET_U_LONG(buf);
		bcopy(buf, aup->aup_machname, str_len);
		aup->aup_machname[str_len] = 0;
		str_len = RNDUP(str_len);
		buf += str_len / sizeof (long);
		aup->aup_uid = IXDR_GET_LONG(buf);
		aup->aup_gid = IXDR_GET_LONG(buf);
		gid_len = IXDR_GET_U_LONG(buf);
		if (gid_len > NGRPS) {
			stat = AUTH_BADCRED;
			goto done;
		}
		aup->aup_len = gid_len;
		for (i = 0; i < gid_len; i++) {
			aup->aup_gids[i] = IXDR_GET_LONG(buf);
		}
		/*
		 * five is the smallest unix credentials structure -
		 * timestamp, hostname len (0), uid, gid, and gids len (0).
		 */
		if ((5 + gid_len) * BYTES_PER_XDR_UNIT + str_len > auth_len) {
			printf("bad auth_len gid %d str %d auth %d\n",
			       gid_len, auth_len, auth_len);
			stat = AUTH_BADCRED;
			goto done;
		}
	} else if (! xdr_authunix_parms(&xdrs, aup)) {
		xdrs.x_op = XDR_FREE;
		(void)xdr_authunix_parms(&xdrs, aup);
		stat = AUTH_BADCRED;
		goto done;
	}
	rqst->rq_xprt->xp_verf.oa_flavor = AUTH_NULL;
	rqst->rq_xprt->xp_verf.oa_length = 0;
	stat = AUTH_OK;
done:
	XDR_DESTROY(&xdrs);
	return (stat);
}


/*
 * Shorthand unix authenticator
 * Looks up longhand in a cache.
 */
/*ARGSUSED*/
enum auth_stat 
_svcauth_short(rqst, msg)
	struct svc_req *rqst;
	struct rpc_msg *msg;
{
	return (AUTH_REJECTEDCRED);
}
