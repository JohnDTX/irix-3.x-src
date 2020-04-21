/* NFSSRC @(#)clnt_kudp.c	2.1 86/04/14 */
/*      @(#)clnt_kudp.c 1.1 86/02/03 SMI      */

/*
 * clnt_kudp.c
 * Implements a kernel UPD/IP based, client side RPC.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#ifdef SVR3
# include "sys/debug.h"
# include "sys/sysmacros.h"
# include "sys/param.h"
# include "sys/systm.h"
# include "sys/signal.h"
# include "sys/errno.h"
# include "sys/psw.h"
# include "sys/pcb.h"
# include "sys/user.h"
# include "sys/time.h"
# include "sys/immu.h"
# include "sys/region.h"
# include "sys/sbd.h"
# include "sys/proc.h"
# include "sys/socket.h"
# include "sys/socketvar.h"
# include "sys/mbuf.h"
# include "net/if.h"
# include "net/route.h"
# include "netinet/in.h"
# include "netinet/in_pcb.h"
# include "sys/fs/nfs_stat.h"
# include "rpc/types.h"
# include "rpc/xdr.h"
# include "rpc/auth.h"
# include "rpc/clnt.h"
# include "rpc/rpc_msg.h"
# define hz	HZ
# define untimeout untimeout_func
#else
# include "../h/param.h"
# include "../h/systm.h"
# include "../h/user.h"
# ifdef sgi
#  include "../bsd/time.h"
# else
#  include "../h/kernel.h"
# endif
# include "../h/proc.h"
# include "../h/socket.h"
# include "../h/socketvar.h"
# include "../h/mbuf.h"
# include "../net/if.h"
# include "../net/route.h"
# include "../netinet/in.h"
# include "../netinet/in_pcb.h"
# ifdef sgi
#  include "../nfs/nfs_stat.h"
# endif
# include "../rpc/types.h"
# include "../rpc/xdr.h"
# include "../rpc/auth.h"
# include "../rpc/clnt.h"
# include "../rpc/rpc_msg.h"
#endif

struct mbuf	*ku_recvfrom();
#if !defined(SVR3) || defined(SIMPLEX)

#define	SETMUSTRUN(was_running)
#define	RESTOREMUSTRUN(was_running)

#else

#define	SETMUSTRUN(was_running)		{ was_running = setmustrun(0); }
#define	RESTOREMUSTRUN(was_running)	{ restoremustrun(was_running); }

#endif

int		ckuwakeup();

enum clnt_stat	clntkudp_callit();
void		clntkudp_abort();
void		clntkudp_error();
bool_t		clntkudp_freeres();
void		clntkudp_destroy();

void		xdrmbuf_init();

/*
 * Operations vector for UDP/IP based RPC
 */
static struct clnt_ops udp_ops = {
	clntkudp_callit,	/* do rpc call */
	clntkudp_abort,		/* abort call */
	clntkudp_error,		/* return error status */
	clntkudp_freeres,	/* free results */
	clntkudp_destroy	/* destroy rpc handle */
};

/*
 * Private data per rpc handle.  This structure is allocated by
 * clntkudp_create, and freed by cku_destroy.
 */
struct cku_private {
#ifdef sgi
	u_char			 cku_flags;	/* see below */
	signed char		 cku_count;	/* count of clustered mbufs */
						/* holding outbuf references */
#else
	u_int			 cku_flags;	/* see below */
#endif
	CLIENT			 cku_client;	/* client handle */
	int			 cku_retrys;	/* request retrys */
	struct socket		*cku_sock;	/* open udp socket */
	struct sockaddr_in	 cku_addr;	/* remote address */
	struct rpc_err		 cku_err;	/* error status */
	XDR			 cku_outxdr;	/* xdr routine for output */
	XDR			 cku_inxdr;	/* xdr routine for input */
	u_int			 cku_outpos;	/* position of in output mbuf */
	char			*cku_outbuf;	/* output buffer */
	char			*cku_inbuf;	/* input buffer */
	struct mbuf		*cku_inmbuf;	/* input mbuf */
	struct ucred		*cku_cred;	/* credentials */
};

#ifdef sgi
struct rcstat rcstat;
#else
struct {
	int	rccalls;
	int	rcbadcalls;
	int	rcretrans;
	int	rcbadxids;
	int	rctimeouts;
	int	rcwaits;
	int	rcnewcreds;
} rcstat;
#endif


#define	ptoh(p)		(&((p)->cku_client))
#define	htop(h)		((struct cku_private *)((h)->cl_private))

/* cku_flags */
#define	CKU_TIMEDOUT	0x001
#define	CKU_BUSY	0x002
#define	CKU_WANTED	0x004
#ifndef sgi
#define	CKU_BUFBUSY	0x008
#endif
#define	CKU_BUFWANTED	0x010

/* Times to retry */
#define	RECVTRIES	2
#define	SNDTRIES	4

int	clntkudpxid;		/* transaction id used by all clients */

static
noop()
{
}

#ifdef sgi
static
buffree(m)
	struct mbuf *m;
{
	register struct cku_private *p;

	p = (struct cku_private *) m->m_farg;
	ASSERT(p->cku_count > 0);
	if (--p->cku_count <= 0 && p->cku_flags & CKU_BUFWANTED) {
		p->cku_count = 0;
		p->cku_flags &= ~CKU_BUFWANTED;
		wakeup((caddr_t)p);
	}
}

static
bufdup(m)
	struct mbuf *m;
{
	register struct cku_private *p;

	p = (struct cku_private *) m->m_darg;
	ASSERT(p->cku_count > 0);
	p->cku_count++;
}
#else
static
buffree(p)
	struct cku_private *p;
{
	p->cku_flags &= ~CKU_BUFBUSY;
	if (p->cku_flags & CKU_BUFWANTED) {
		p->cku_flags &= ~CKU_BUFWANTED;
		wakeup((caddr_t)p);
	}
}
#endif

/*
 * Create an rpc handle for a udp rpc connection.
 * Allocates space for the handle structure and the private data, and
 * opens a socket.  Note sockets and handles are one to one.
 */
CLIENT *
clntkudp_create(addr, pgm, vers, retrys, cred)
	struct sockaddr_in *addr;
	u_long pgm;
	u_long vers;
	int retrys;
	struct ucred *cred;
{
	register CLIENT *h;
	register struct cku_private *p;
	register int error = 0;
	register int was_running;
	struct rpc_msg call_msg;
	struct mbuf *m, *mclgetx();
	extern int nfs_portmon;

	SETMUSTRUN(was_running);
#ifdef RPCDEBUG
	rpc_debug(4, "clntkudp_create(%X, %d, %d, %d\n",
	    addr->sin_addr.s_addr, pgm, vers, retrys);
#endif
	p = (struct cku_private *)kmem_alloc((u_int)sizeof *p);
	bzero((caddr_t)p, sizeof (*p));
	h = ptoh(p);

	if (!clntkudpxid) {
#ifdef sgi
		clntkudpxid = lbolt;	/* time in ticks */
#else
		clntkudpxid = time.tv_usec;
#endif
	}

	/* handle */
	h->cl_ops = &udp_ops;
	h->cl_private = (caddr_t) p;
	h->cl_auth = authkern_create();

	/* call message, just used to pre-serialize below */
	call_msg.rm_xid = 0;
	call_msg.rm_direction = CALL;
	call_msg.rm_call.cb_rpcvers = RPC_MSG_VERSION;
	call_msg.rm_call.cb_prog = pgm;
	call_msg.rm_call.cb_vers = vers;

	/* private */
	clntkudp_init(h, addr, retrys, cred);
#ifdef sgi
	p->cku_outbuf = (char *)kmem_allocmbuf((u_int)UDPMSGSIZE);
	m = mclgetx(noop, 0L, noop, 0L, p->cku_outbuf, UDPMSGSIZE, M_DONTWAIT);
#else
	p->cku_outbuf = (char *)kmem_alloc((u_int)UDPMSGSIZE);
	m = mclgetx(noop, 0L, p->cku_outbuf, UDPMSGSIZE, M_DONTWAIT);
#endif
	if (m == NULL)
		goto bad;
	xdrmbuf_init(&p->cku_outxdr, m, XDR_ENCODE);

	/* pre-serialize call message header */
	if (! xdr_callhdr(&(p->cku_outxdr), &call_msg)) {
		printf("clntkudp_create - Fatal header serialization error.");
		(void) m_freem(m);
		goto bad;
	}
	p->cku_outpos = XDR_GETPOS(&(p->cku_outxdr));
	(void) m_free(m);

	/* open udp socket */
	error = socreate(AF_INET, &p->cku_sock, SOCK_DGRAM, IPPROTO_UDP);
	if (error) {
		printf("clntkudp_create: socket creation problem, %d", error);
		goto bad;
	}
	if (error = bindresvport(p->cku_sock)) {
		printf("clntkudp_create: socket bind problem, %d", error);
		goto bad;
	}
	RESTOREMUSTRUN(was_running);
	return (h);

bad:
#ifdef sgi
	kmem_freembuf((caddr_t)p->cku_outbuf, (u_int)UDPMSGSIZE);
#else
	kmem_free((caddr_t)p->cku_outbuf, (u_int)UDPMSGSIZE);
#endif
	kmem_free((caddr_t)(caddr_t)p, (u_int)sizeof(struct cku_private));
#ifdef RPCDEBUG
	rpc_debug(4, "create failed\n");
#endif
	RESTOREMUSTRUN(was_running);
	return ((CLIENT *)NULL);
}

clntkudp_init(h, addr, retrys, cred)
	CLIENT *h;
	struct sockaddr_in *addr;
	int retrys;
	struct ucred *cred;
{
	struct cku_private *p = htop(h);

	p->cku_retrys = retrys;
	p->cku_addr = *addr;
	p->cku_cred = cred;
}

/*
 * Time out back off function. tim is in hz
 */
#define MAXTIMO	(60 * hz)
#define backoff(tim)	((((tim) << 1) > MAXTIMO) ? MAXTIMO : ((tim) << 1))

/*
 * Call remote procedure.
 * Most of the work of rpc is done here.  We serialize what is left
 * of the header (some was pre-serialized in the handle), serialize
 * the arguments, and send it off.  We wait for a reply or a time out.
 * Timeout causes an immediate return, other packet problems may cause
 * a retry on the receive.  When a good packet is received we deserialize
 * it, and check verification.  A bad reply code will cause one retry
 * with full (longhand) credentials.
 */
enum clnt_stat 
clntkudp_callit(h, procnum, xdr_args, argsp, xdr_results, resultsp, wait)
	register CLIENT	*h;
	u_long		procnum;
	xdrproc_t	xdr_args;
	caddr_t		argsp;
	xdrproc_t	xdr_results;
	caddr_t		resultsp;
	struct timeval	wait;
{
	register struct cku_private *p = htop(h);
	register XDR	   	   *xdrs;
	register struct socket	   *so = p->cku_sock;
	int			   rtries;
	int			   stries = p->cku_retrys;
	struct sockaddr_in	   from;
	struct rpc_msg		   reply_msg;
	int			   s;
	register int		   was_running;
#ifdef sgi
	struct ucred		   tmpcred;
#else
	struct ucred		   *tmpcred;
#endif
	struct mbuf		   *m;
	int timohz;
	u_long xid;

	SETMUSTRUN(was_running);

#ifdef RPCDEBUG
	rpc_debug(4, "cku_callit\n");
#endif
	rcstat.rccalls++;

	while (p->cku_flags & CKU_BUSY) {
		rcstat.rcwaits++;
		p->cku_flags |= CKU_WANTED;
		sleep((caddr_t)h, PZERO-2);
	}
	p->cku_flags |= CKU_BUSY;

	/*
	 * Set credentials into the u structure
	 */
#ifdef sgi
	crinit(&u, &tmpcred);
	cruse(p->cku_cred, &u);
#else
	tmpcred = u.u_cred;
	u.u_cred = p->cku_cred;
#endif

	xid = clntkudpxid++;

	/*
	 * This is dumb but easy: keep the time out in units of hz
	 * so it is easy to call timeout and modify the value.
	 */
	timohz = wait.tv_sec * hz + (wait.tv_usec * hz) / 1000000;

call_again:

	/*
	 * Wait till buffer gets freed then make a type 2 mbuf point at it
	 * The buffree routine clears CKU_BUFBUSY and does a wakeup when
	 * the mbuf gets freed.
	 */
	s = splimp();
#ifdef sgi
	while (p->cku_count > 0) {
#else
	while (p->cku_flags & CKU_BUFBUSY) {
#endif
		p->cku_flags |= CKU_BUFWANTED;
		/*
		 * This is a kludge to avoid deadlock in the case of a
		 * loop-back call.  The client can block waiting for
		 * the server to free the mbuf while the server is blocked
		 * waiting for the client to free the reply mbuf.  Avoid this
		 * by flushing the input queue every once in a while while
		 * we are waiting.
		 */
		timeout(wakeup, (caddr_t)p, hz);
		sleep((caddr_t)p, PZERO-3);
		sbflush(&so->so_rcv);
	}
#ifdef sgi
	ASSERT(p->cku_count == 0);
	p->cku_count = 1;
#else
	p->cku_flags |= CKU_BUFBUSY;
#endif
	(void) splx(s);
#ifdef sgi
	m = mclgetx(buffree, (long)p, bufdup, (long)p, p->cku_outbuf,
		UDPMSGSIZE, M_WAIT);
#else
	m = mclgetx(buffree, (long)p, p->cku_outbuf, UDPMSGSIZE, M_WAIT);
#endif
	if (m == NULL) {
		p->cku_err.re_status = RPC_SYSTEMERROR;
		p->cku_err.re_errno = ENOBUFS;
#ifdef sgi
		{ struct mbuf m; /* XXX NIH syndrome: mbuf/free interface */
		  m.m_farg = (long) p;
		  buffree(&m);
		}
#else
		buffree(p);
#endif
		goto done;
	}

	/*
	 * The transaction id is the first thing in the
	 * preserialized output buffer.
	 */
	(*(u_long *)(p->cku_outbuf)) = xid;

	xdrmbuf_init(&p->cku_outxdr, m, XDR_ENCODE);
	xdrs = &p->cku_outxdr;
	XDR_SETPOS(xdrs, p->cku_outpos);

	/*
	 * Serialize dynamic stuff into the output buffer.
	 */
	if ((! XDR_PUTLONG(xdrs, (long *)&procnum)) ||
	    (! AUTH_MARSHALL(h->cl_auth, xdrs)) ||
	    (! (*xdr_args)(xdrs, argsp))) {
		p->cku_err.re_status = RPC_CANTENCODEARGS;
		p->cku_err.re_errno = EIO;
		(void) m_freem(m);
		goto done;
	}

	if (m->m_next == 0) {		/* XXX */
		m->m_len = XDR_GETPOS(&(p->cku_outxdr));
	}
	if (p->cku_err.re_errno =
	    ku_sendto_mbuf(so, m, &p->cku_addr)) {
		p->cku_err.re_status = RPC_CANTSEND;
		goto done;
	}

	reply_msg.acpted_rply.ar_verf = _null_auth;
	reply_msg.acpted_rply.ar_results.where = resultsp;
	reply_msg.acpted_rply.ar_results.proc = xdr_results;

	for (rtries = RECVTRIES; rtries; rtries--) {
		s = splnet();
		while (so->so_rcv.sb_cc == 0) {
			/*
			 * Set timeout then wait for input or timeout
			 */
#ifdef RPCDEBUG
			rpc_debug(3, "callit: waiting %d\n", timohz);
#endif
			timeout(ckuwakeup, (caddr_t)p, timohz);
			so->so_rcv.sb_flags |= SB_WAIT;
			sleep((caddr_t)&so->so_rcv.sb_cc, PRIBIO);
			untimeout(ckuwakeup, (caddr_t)p);

			if (p->cku_flags & CKU_TIMEDOUT) {
				p->cku_flags &= ~CKU_TIMEDOUT;
				(void) splx(s);
				p->cku_err.re_status = RPC_TIMEDOUT;
				p->cku_err.re_errno = ETIMEDOUT;
				rcstat.rctimeouts++;
				goto done;
			}
		}

		if (so->so_error) {
			so->so_error = 0;
			(void) splx(s);
			continue;
		}

		p->cku_inmbuf = ku_recvfrom(so, &from);
		(void) splx(s);
		if (p->cku_inmbuf == NULL) {
			continue;
		}
		p->cku_inbuf = mtod(p->cku_inmbuf, char *);

		if (p->cku_inmbuf->m_len < sizeof(u_long)) {
			m_freem(p->cku_inmbuf);
			continue;
		}
		/*
		 * If reply transaction id matches id sent
		 * we have a good packet.
		 */
		if (*((u_long *)(p->cku_inbuf))
		    != *((u_long *)(p->cku_outbuf))) {
			rcstat.rcbadxids++;
			m_freem(p->cku_inmbuf);
			continue;
		}
		/*
		 * Flush the rest of the stuff on the input queue
		 * for the socket.
		 */
		s = splnet();
		sbflush(&so->so_rcv);
		(void) splx(s);
		break;
	}

	if (rtries == 0) {
		p->cku_err.re_status = RPC_CANTRECV;
		goto done;
	}

	/*
	 * Process reply
	 */

	xdrs = &(p->cku_inxdr);
	xdrmbuf_init(xdrs, p->cku_inmbuf, XDR_DECODE);

	/*
	 * Decode and validate the response.
	 */
	if (xdr_replymsg(xdrs, &reply_msg)) {
		_seterr_reply(&reply_msg, &(p->cku_err));

		if (p->cku_err.re_status == RPC_SUCCESS) {
			/*
			 * Reply is good, check auth.
			 */
			if (! AUTH_VALIDATE(h->cl_auth,
			    &reply_msg.acpted_rply.ar_verf)) {
				p->cku_err.re_status = RPC_AUTHERROR;
				p->cku_err.re_why = AUTH_INVALIDRESP;
			}
			if (reply_msg.acpted_rply.ar_verf.oa_base != NULL) {
				/* free auth handle */
				xdrs->x_op = XDR_FREE;
				(void)xdr_opaque_auth(xdrs,
				    &(reply_msg.acpted_rply.ar_verf));
			} 
		}
	} else {
		p->cku_err.re_status = RPC_CANTDECODERES;
	}
	m_freem(p->cku_inmbuf);

#ifdef RPCDEBUG
	rpc_debug(4, "cku_callit done\n");
#endif
done:
	if ( (p->cku_err.re_status != RPC_SUCCESS) &&
	     (p->cku_err.re_status != RPC_CANTENCODEARGS) &&
	     (--stries > 0) ) {
		rcstat.rcretrans++;
		timohz = backoff(timohz);
		if (p->cku_err.re_status == RPC_SYSTEMERROR ||
		    p->cku_err.re_status == RPC_CANTSEND) {
			/*
			 * Errors due to lack of resources, wait a bit
			 * and try again.
			 */
#ifdef sgi
			timeout(wakeup, (caddr_t)&lbolt, hz);
#endif
			sleep((caddr_t)&lbolt, PZERO-4);
		}
		goto call_again;
	}

#ifdef sgi
interrupted:
	cruse(&tmpcred, &u);
#else
	u.u_cred = tmpcred;
#endif
	p->cku_flags &= ~CKU_BUSY;
	if (p->cku_flags & CKU_WANTED) {
		p->cku_flags &= ~CKU_WANTED;
		wakeup((caddr_t)h);
	}
	if (p->cku_err.re_status != RPC_SUCCESS) {
		rcstat.rcbadcalls++;
	}
	RESTOREMUSTRUN(was_running);
	return (p->cku_err.re_status);
}

/*
 * Wake up client waiting for a reply.
 */
ckuwakeup(p)
	register struct cku_private *p;
{

#ifdef RPCDEBUG
	rpc_debug(4, "cku_timeout\n");
#endif
	p->cku_flags |= CKU_TIMEDOUT;
	sbwakeup(&p->cku_sock->so_rcv);
}

/*
 * Return error info on this handle.
 */
void
clntkudp_error(h, err)
	CLIENT *h;
	struct rpc_err *err;
{
	register struct cku_private *p = htop(h);

	*err = p->cku_err;
}

static bool_t
clntkudp_freeres(cl, xdr_res, res_ptr)
	CLIENT *cl;
	xdrproc_t xdr_res;
	caddr_t res_ptr;
{
	register struct cku_private *p = (struct cku_private *)cl->cl_private;
	register XDR *xdrs = &(p->cku_outxdr);

	xdrs->x_op = XDR_FREE;
	return ((*xdr_res)(xdrs, res_ptr));
}

void 
clntkudp_abort()
{
}

/*
 * Destroy rpc handle.
 * Frees the space used for output buffer, private data, and handle
 * structure, and closes the socket for this handle.
 */
void
clntkudp_destroy(h)
	CLIENT *h;
{
	register struct cku_private *p = htop(h);
	register int was_running;

	SETMUSTRUN(was_running);

#ifdef RPCDEBUG
	rpc_debug(4, "cku_destroy %x\n", h);
#endif
	(void) soclose(p->cku_sock);
#ifdef sgi
	kmem_freembuf((caddr_t)p->cku_outbuf, (u_int)UDPMSGSIZE);
#else
	kmem_free((caddr_t)p->cku_outbuf, (u_int)UDPMSGSIZE);
#endif
	kmem_free((caddr_t)p, (u_int)sizeof(*p));

	RESTOREMUSTRUN(was_running);
}

/*
 * try to bind to a reserved port
 */
static
bindresvport(so)
	struct socket *so;
{
	struct sockaddr_in *sin;
	struct mbuf *m;
	u_short i;
	int error;
	int saveuid;

#	define MAX_PRIV	(IPPORT_RESERVED-1)
#	define MIN_PRIV	(IPPORT_RESERVED/2)

	m = m_get(M_WAIT, MT_SONAME);
	if (m == NULL) {
		printf("bindresvport: couldn't alloc mbuf");
		return(ENOBUFS);
	}

	sin = mtod(m, struct sockaddr_in *);
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = INADDR_ANY;
	m->m_len = sizeof(struct sockaddr_in);

	saveuid = u.u_uid;
	u.u_uid = 0;
	error = EADDRINUSE;
	for (i = MAX_PRIV; error == EADDRINUSE && i >= MIN_PRIV; i--) {
		sin->sin_port = htons(i);
		error = sobind(so, m);
	}
	u.u_uid = saveuid;

	(void) m_freem(m);
	return (error);
}
