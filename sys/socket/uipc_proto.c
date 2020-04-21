/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)uipc_proto.c	7.1 (Berkeley) 6/5/86
 */

#ifdef mips
#include "../tcp-param.h"
#include "sys/param.h"
#include "sys/socket.h"
#include "sys/protosw.h"
#include "sys/domain.h"
#include "sys/mbuf.h"
#else
#include "../h/param.h"
#include "../h/socket.h"
#include "../h/protosw.h"
#include "../h/domain.h"
#include "../h/mbuf.h"
#include "unixdomain.h"
#endif

/*
 * Definitions of protocols supported in the UNIX domain.
 */

int	uipc_usrreq();
int	raw_init(),raw_usrreq(),raw_input(),raw_ctlinput();
extern	struct domain unixdomain;		/* or at least forward */

struct protosw unixsw[] = {
#if NUNIXDOMAIN > 0
{ SOCK_STREAM,	&unixdomain,	0,	PR_CONNREQUIRED|PR_WANTRCVD|PR_RIGHTS,
  0,		0,		0,		0,
  uipc_usrreq,
  0,		0,		0,		0,
},
{ SOCK_DGRAM,	&unixdomain,	0,		PR_ATOMIC|PR_ADDR|PR_RIGHTS,
  0,		0,		0,		0,
  uipc_usrreq,
  0,		0,		0,		0,
},
#endif
{ 0,		0,		0,		0,
  raw_input,	0,		raw_ctlinput,	0,
  raw_usrreq,
  raw_init,	0,		0,		0,
}
};

#if NUNIXDOMAIN > 0
int	unp_externalize(), unp_dispose();
#else
#define unp_externalize 0
#define unp_dispose 0
#endif

struct domain unixdomain =
    { AF_UNIX, "unix", 0, unp_externalize, unp_dispose,
      unixsw, &unixsw[sizeof(unixsw)/sizeof(unixsw[0])] };
