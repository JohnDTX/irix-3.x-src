#ifndef _nfs_stat_
#define	_nfs_stat_
/*
 * NFS statistics structures.
 *
 * $Source: /d2/3.7/src/sys/nfs/RCS/nfs_stat.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:18 $
 */

/*
 * client side statistics
 */
struct clstat {
	int	nclsleeps;		/* client handle waits */
	int	nclgets;		/* client handle gets */
	int	ncalls;			/* client requests */
	int	nbadcalls;		/* rpc failures */
	int	reqs[32];		/* count of each request */
};

/*
 * server side statistics
 */
struct svstat {
	int	ncalls;		/* number of calls received */
	int	nbadcalls;	/* calls that failed */
	int	reqs[32];	/* count for each request */
};

/*
 * rpc client statistics.
 */
struct rcstat {
	int	rccalls;
	int	rcbadcalls;
	int	rcretrans;
	int	rcbadxids;
	int	rctimeouts;
	int	rcwaits;
	int	rcnewcreds;
};

/*
 * rpc server statistics
 */
struct rsstat {
	int	rscalls;
	int	rsbadcalls;
	int	rsnullrecv;
	int	rsbadlen;
	int	rsxdrcall;
};

#endif
