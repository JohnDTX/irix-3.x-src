/* NFSSRC @(#)clnt.h	2.1 86/04/14 */
/*	@(#)clnt.h 1.1 86/02/03 SMI      */

/*
 * clnt.h - Client side remote procedure call interface.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

/*
 * Rpc calls return an enum clnt_stat.  This should be looked at more,
 * since each implementation is required to live with this (implementation
 * independent) list of errors.
 */
enum clnt_stat {
	RPC_SUCCESS=0,			/* call succeeded */
	/*
	 * local errors
	 */
	RPC_CANTENCODEARGS=1,		/* can't encode arguments */
	RPC_CANTDECODERES=2,		/* can't decode results */
	RPC_CANTSEND=3,			/* failure in sending call */
	RPC_CANTRECV=4,			/* failure in receiving result */
	RPC_TIMEDOUT=5,			/* call timed out */
	/*
	 * remote errors
	 */
	RPC_VERSMISMATCH=6,		/* rpc versions not compatible */
	RPC_AUTHERROR=7,		/* authentication error */
	RPC_PROGUNAVAIL=8,		/* program not available */
	RPC_PROGVERSMISMATCH=9,		/* program version mismatched */
	RPC_PROCUNAVAIL=10,		/* procedure unavailable */
	RPC_CANTDECODEARGS=11,		/* decode arguments error */
	RPC_SYSTEMERROR=12,		/* generic "other problem" */

	/*
	 * callrpc errors
	 */
	RPC_UNKNOWNHOST=13,		/* unknown host name */

	/*
	 * _ create errors
	 */
	RPC_PMAPFAILURE=14,		/* the pmapper failed in its call */
	RPC_PROGNOTREGISTERED=15,	/* remote program is not registered */
	/*
	 * unspecified error
	 */
	RPC_FAILED=16
};


/*
 * Error info.
 */
struct rpc_err {
	enum clnt_stat re_status;
	union {
		int RE_errno;		/* realated system error */
		enum auth_stat RE_why;	/* why the auth error occurred */
		struct {
			u_long low;	/* lowest verion supported */
			u_long high;	/* highest verion supported */
		} RE_vers;
		struct {		/* maybe meaningful if RPC_FAILED */
			long s1;
			long s2;
		} RE_lb;		/* life boot & debugging only */
	} ru;
#define	re_errno	ru.RE_errno
#define	re_why		ru.RE_why
#define	re_vers		ru.RE_vers
#define	re_lb		ru.RE_lb
};


/*
 * Client rpc handle.
 * Created by individual implementations, see e.g. rpc_udp.c.
 * Client is responsible for initializing auth, see e.g. auth_none.c.
 */
typedef struct {
	AUTH	*cl_auth;			/* authenticator */
	struct clnt_ops {
		enum clnt_stat	(*cl_call)();	/* call remote procedure */
		void		(*cl_abort)();	/* abort a call */
		void		(*cl_geterr)();	/* get specific error code */
		bool_t		(*cl_freeres)(); /* frees results */
		void		(*cl_destroy)();/* destroy this structure */
	} *cl_ops;
	caddr_t			cl_private;	/* private stuff */
} CLIENT;


/*
 * client side rpc interface ops
 *
 * Parameter types are:
 *
 */

/*
 * enum clnt_stat
 * CLNT_CALL(rh, proc, xargs, argsp, xres, resp, timeout)
 * 	CLIENT *rh;
 *	u_long proc;
 *	xdrproc_t xargs;
 *	caddr_t argsp;
 *	xdrproc_t xres;
 *	caddr_t resp;
 *	struct timeval timeout;
 */
#define	CLNT_CALL(rh, proc, xargs, argsp, xres, resp, secs)	\
	((*(rh)->cl_ops->cl_call)(rh, proc, xargs, argsp, xres, resp, secs))
#define	clnt_call(rh, proc, xargs, argsp, xres, resp, secs)	\
	((*(rh)->cl_ops->cl_call)(rh, proc, xargs, argsp, xres, resp, secs))

/*
 * void
 * CLNT_ABORT(rh);
 * 	CLIENT *rh;
 */
#define	CLNT_ABORT(rh)	((*(rh)->cl_ops->cl_abort)(rh))
#define	clnt_abort(rh)	((*(rh)->cl_ops->cl_abort)(rh))

/*
 * struct rpc_err
 * CLNT_GETERR(rh);
 * 	CLIENT *rh;
 */
#define	CLNT_GETERR(rh,errp)	((*(rh)->cl_ops->cl_geterr)(rh, errp))
#define	clnt_geterr(rh,errp)	((*(rh)->cl_ops->cl_geterr)(rh, errp))


/*
 * bool_t
 * CLNT_FREERES(rh, xres, resp);
 * 	CLIENT *rh;
 *	xdrproc_t xres;
 *	caddr_t resp;
 */
#define	CLNT_FREERES(rh,xres,resp) ((*(rh)->cl_ops->cl_freeres)(rh,xres,resp))
#define	clnt_freeres(rh,xres,resp) ((*(rh)->cl_ops->cl_freeres)(rh,xres,resp))

/*
 * void
 * CLNT_DESTROY(rh);
 * 	CLIENT *rh;
 */
#define	CLNT_DESTROY(rh)	((*(rh)->cl_ops->cl_destroy)(rh))
#define	clnt_destroy(rh)	((*(rh)->cl_ops->cl_destroy)(rh))


/*
 * RPCTEST is a test program which is accessable on every rpc
 * transport/port.  It is used for testing, performance evaluation,
 * and network administration.
 */

#define RPCTEST_PROGRAM		((u_long)1)
#define RPCTEST_VERSION		((u_long)1)
#define RPCTEST_NULL_PROC	((u_long)2)
#define RPCTEST_NULL_BATCH_PROC	((u_long)3)

/*
 * By convention, procedure 0 takes null arguments and returns them
 */

#define NULLPROC ((u_long)0)

/*
 * Below are the client handle creation routines for the various
 * implementations of client side rpc.  They can return NULL if a 
 * creation failure occurs.
 */

#ifndef KERNEL
/*
 * Memory based rpc (for speed check and testing)
 * CLIENT *
 * clntraw_create(prog, vers)
 *	u_long prog;
 *	u_long vers;
 */
extern CLIENT *clntraw_create();

/*
 * TCP based rpc
 * CLIENT *
 * clnttcp_create(raddr, prog, vers, sockp, sendsz, recvsz)
 *	struct sockaddr_in *raddr;
 *	u_long prog;
 *	u_long version;
 *	register int *sockp;
 *	u_int sendsz;
 *	u_int recvsz;
 */
extern CLIENT *clnttcp_create();

/*
 * UDP based rpc.
 * CLIENT *
 * clntudp_create(raddr, program, version, wait, sockp)
 *	struct sockaddr_in *raddr;
 *	u_long program;
 *	u_long version;
 *	struct timeval wait;
 *	int *sockp;
 *
 * Same as above, but you specify max packet sizes.
 * CLIENT *
 * clntudp_bufcreate(raddr, program, version, wait, sockp, sendsz, recvsz)
 *	struct sockaddr_in *raddr;
 *	u_long program;
 *	u_long version;
 *	struct timeval wait;
 *	int *sockp;
 *	u_int sendsz;
 *	u_int recvsz;
 */
extern CLIENT *clntudp_create();
extern CLIENT *clntudp_bufcreate();

/* 
 * If a creation fails, the following allows the user to figure out why.
 */
struct rpc_createerr {
	enum clnt_stat cf_stat;
	struct rpc_err cf_error; /* useful when cf_stat == RPC_PMAPFAILURE */
};

extern struct rpc_createerr rpc_createerr;
#endif !KERNEL

#ifdef KERNEL
/*
 * Kernel udp based rpc
 * CLIENT *
 * clntkudp_create(addr, pgm, vers)
 *	struct sockaddr_in *addr;
 *	u_long pgm;
 *	u_long vers;
 */
extern CLIENT *clntkudp_create();
#endif

#define UDPMSGSIZE	8800	/* rpc imposed limit on udp msg size */
#define RPCSMALLMSGSIZE	400	/* a more reasonable packet size */
