/* $Header: /d2/3.7/src/sys/multibusif/RCS/if_cy.c,v 1.1 89/03/27 17:32:00 root Exp $ */
/*
 * if_cy.c - Cypress Network interface routines
 *
 * Author:	Thomas Narten
 *		Dept. of Computer Sciences
 *		Purdue University
 * Date:	Sun Sep 22 1985
 * Copyright (c) 1985 Thomas Narten
 */


#if defined(sgi) && defined(SVR3)
#define NCYPRESS 1
#else SVR3
#include "cypress.h"
#endif SVR3
#if NCYPRESS > 0
static char rcs_ident[] = "$Header: /d2/3.7/src/sys/multibusif/RCS/if_cy.c,v 1.1 89/03/27 17:32:00 root Exp $";

#ifdef sgi
#define BSD43
#define CYDEBUG
#ifdef SVR3
#include "../tcp-param.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/mbuf.h"
#include "sys/protosw.h"
#include "sys/socket.h"
#include "sys/cmn_err.h"
#include "sys/errno.h"
#include "sys/debug.h"
#include "sys/kopt.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/pcb.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/fs/s5dir.h"
#include "sys/user.h"
#include "sys/file.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/strids.h"
#else SVR3
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mbuf.h"
#include "../h/protosw.h"
#include "../h/socket.h"
#include "../h/errno.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/file.h"
#include "../streams/stream.h"
#include "../streams/stropts.h"
#include "../streams/strids.h"
#endif SVR3
#include "../net/if.h"
#include "../net/netisr.h"
#include "../net/route.h"
#include "../net/soioctl.h"
#include "../netinet/in.h"
#include "../netinet/in_systm.h"
#include "../netinet/ip.h"
#include "../netinet/ip_var.h"
#include "../netinet/in_var.h"
#include "if_cy.h"

#else sgi
#include "../machine/pte.h"
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mbuf.h"
#include "../h/buf.h"
#include "../h/protosw.h"
#include "../h/socket.h"
#include "../h/vmmac.h"
#include "../h/ioctl.h"
#include "../h/errno.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/kernel.h"
#include "../h/clist.h"
#include "../h/dk.h"
#include "../h/file.h"
#include "../h/tty.h"
#include "../h/ttydev.h"
#include "../h/dk.h"

#include "../net/if.h"
#include "../net/netisr.h"
#include "../net/route.h"
#include "../netinet/in.h"
#include "../netinet/in_systm.h"
#include "../netinet/ip.h"
#include "../netinet/ip_var.h"

#if defined(BSD43)
#include "../netinet/in_var.h"
#endif

#ifdef	vax
#include "../vax/mtpr.h"
#endif
#if defined(BSD43)
#include "uba.h"
#endif
#include "../netinet/if_cy.h"


 struct mbuf *cyFrameToMbuf();
#endif sgi

int cydebug = 0;
int cyhopcount = 4;
#define cy_setr(h,ln)	(cy_dev.MPhln[h]= ln)
#define mphln(h)	(cy_dev.MPhln[h])



struct cyln rgcyln[CLINEMAX];
struct cy_dev cy_dev;
#ifndef sgi
struct ifqueue cyintrq;	/* queue for packets from lower devices  */
#endif sgi

int cycnn = 0;
int cycrpf = 0;
int cycfl  = 0;
int cycdirect = 0;
int cycnns = 0;
int cycrpfs = 0;
int cycfls  = 0;
int cycdirects = 0;
int cy_cqhiwat = 50,          /* accept no more packets past here */
    cy_cqlowat = 30,          /* only accept short packets while queue */
			      /* length above low water mark           */
     cy_small = 75;	      /* size of a "small" packet */
#ifdef sgi
extern u_char ipcksum;			/* force IP checksummin on */

int cyattach(), cyinit(), cyoutput(), cyioctl();

static struct module_info stm_info = {
    STRID_CYPRESS,			/* module ID */
    "CYPRESS",				/* module name */
    0,					/* minimum packet size */
    INFPSZ,				/* infinite maximum packet size	*/
    1,					/* hi-water mark */
    0,					/* lo-water mark */
};

static cy_rsrv(), cy_open(), cy_close();
static struct qinit st_rinit = {
	putq, cy_rsrv, cy_open, cy_close, NULL, &stm_info, NULL
};

static cy_wput(), cy_wsrv();
static struct qinit st_winit = {
	cy_wput, cy_wsrv, NULL, NULL, NULL, &stm_info, NULL
};

struct streamtab if_cyinfo = {&st_rinit, &st_winit, NULL, NULL};

extern mblk_t *str_allocb(), *getq();
#else sgi
int cy_topen(), cy_tclose(), cy_trint(),
     cyattach(), cyinit(), cyoutput(), cyioctl();
#endif sgi
int cyMonitor(), cy_toutput();

 /*
  * ============================================================
  * cyattach - called at boot time to intialize interface.
  * ============================================================
  */
#ifdef sgi
if_cyinit()
#else sgi
cyattach()
#endif sgi
{
    register struct ifnet *pif = &cy_dev.cyd_if;
#ifdef sgi
    register int i;
#else sgi
    register int i, j;
#endif sgi
    for (i=0; i < CHOSTSMAX; i++)
	cy_setr(i,CYR_NOROUTE);
#ifdef sgi
    for (i=0; i < CLINEMAX; i++)
	rgcyln[i].cy_send.ifq_maxlen = cy_cqhiwat;
#else sgi
    for (i=0, j=0; i < CLINEMAX; i++) {
	rgcyln[i].cy_send.ifq_maxlen = cy_cqhiwat;
	rgcyln[i].cyl_pdmabuf1= &cy_dev.cyd_rgdmabuf[j++];
	rgcyln[i].cyl_pdmabuf2= &cy_dev.cyd_rgdmabuf[j++];
    }
#endif sgi
    cy_dev.cyd_logrechd = cy_dev.cyd_logrectl = 0; /* intialize queue */
    pif -> if_name = "cy";
    pif -> if_unit = 0;
    pif -> if_mtu   = CYMTU - sizeof(struct cy_hdr);
    pif -> if_flags = IFF_BROADCAST;
    pif -> if_init  = cyinit;
    pif -> if_output = cyoutput;
    pif -> if_ioctl = cyioctl;
    pif -> if_watchdog = NULL;
#if defined(sgi) && defined(SVR3)
    if (showconfig)
#endif
    printf("%s\n", STARTUP);
    if_attach(pif);
#ifdef sgi
    ipcksum = 1;			/* force IP-checksumming on */
#endif
}


 /*
  * ============================================================
  * cyinit - Set the interface flags to running if we know our local
  * address and at least one output line has been configured.
  * ============================================================
  */
cyinit(unit, uban)
int unit, uban;			/* currently ignored! */
{
    register struct ifnet *pif = &cy_dev.cyd_if;
#ifndef sgi
    register struct sockaddr_in *sin;
#endif sgi
    register int ln;		/* line number */
#if defined(BSD43)
    if (pif->if_addrlist == NULL)
	return;
#else
    sin = (struct sockaddr_in *)&pif->if_addr;
    if (sin->sin_addr.s_addr == 0)
	return;
#endif
    for (ln = 0; ln < CLINEMAX; ln++) {
	if(rgcyln[ln].cyl_dest.s_addr != 0)	/* if address is known */
	    break;
    }
    if (ln >= CLINEMAX) return;	/* no point in turning on interface */
#ifdef CYDEBUG
    printf("cyinit: Turning on interface.\n");
#endif
#if (defined(ULTRIX11) || defined(BSD42) || defined(NFS30))
    if_rtinit(pif, RTF_UP);
#endif
    pif->if_flags |= IFF_UP | IFF_RUNNING;
}



#ifndef sgi
/*
 * ========================================================================
 * cy_lattach - Attach a network interface as a Cypress line
 * ========================================================================
 */

cy_lattach(pf)
int (*pf)();
{
    printf("cy_lattach: called\n");
}
#endif sgi



/*
 * ============================================================
 * cyioctl - Process an ioctl request
 * ============================================================
 */
cyioctl(pif, cmd, data)
register struct ifnet *pif;
int cmd;
caddr_t data;
{
#ifdef NFS30
    register struct ifreq *pifr;
    register struct cy_ioctlarg *pcy_ioctlarg;
    register struct sockaddr_in *sin, sintemp;
    register int s = splimp(), error = 0;
    register int ln;		/* line number */

    if (u.u_uid)
	return(EPERM);

    switch (cmd) {
      case SIOCSIFADDR:
	if (pif->if_flags & IFF_RUNNING)
	    if_rtinit(pif, -1);     /* delete previous route */
	sin = (struct sockaddr_in *) data;
	pif->if_net = in_netof(sin->sin_addr);
	pif->if_host[0] = in_lnaof(sin->sin_addr);
	cy_dev.cyd_hid = in_lnaof(sin->sin_addr);
	printf("cyioctl: host id = %d.\n", cy_dev.cyd_hid);
	cy_dev.cyd_baddr = if_makeaddr(in_netof(sin->sin_addr),
		INADDR_ANY);
	printf("cyioctl: broadcast addr = 0x%x.\n", cy_dev.cyd_baddr);
	cy_dev.cyd_haddr = sin->sin_addr;
	printf("cyioctl: host address = 0x%x.\n", cy_dev.cyd_haddr);
	cy_setr(cy_dev.cyd_hid, CYR_THISHOST);

	sin = (struct sockaddr_in *)&pif->if_addr;
	sin->sin_family = AF_INET;
	sin->sin_addr = if_makeaddr(pif->if_net, cy_dev.cyd_hid);
	sin = (struct sockaddr_in *) &pif->if_broadaddr;
	sin->sin_family = AF_INET;
	sin->sin_addr = if_makeaddr(pif->if_net, INADDR_ANY);
	pif->if_flags |= IFF_BROADCAST | IFF_UP;

	/* set up routing table entry */
	if (pif->if_flags & IFF_RUNNING) {
	    printf("cyioctl: if_route returns %d.\n",
		   if_rtinit(pif, RTF_UP));
	} else
	    cyinit(pif->if_unit, 0);

	cyTurnOnMonitor();
	break;
#endif NFS30
#ifdef BSD42
    register struct ifreq *pifr;
    register struct cy_ioctlarg *pcy_ioctlarg;
    register struct sockaddr_in *sin, sintemp;
    register int s = splimp(), error = 0;
    register int ln;		/* line number */

    if (u.u_uid)
	return(EPERM);
    switch (cmd) {
	case SIOCSIFADDR:
		if (pif->if_flags & IFF_RUNNING)
		if_rtinit(pif, -1);     /* delete previous route */
	   /*
	    * 4.2 BSD version
	    */
	    pifr = (struct ifreq *) data;
	    sin = (struct sockaddr_in *) &pifr->ifr_addr;
	    pif->if_net = in_netof(sin->sin_addr);
	    pif->if_host[0] = in_lnaof(sin->sin_addr);
	    cy_dev.cyd_hid = in_lnaof(sin->sin_addr);
	    printf("cyioctl: host id = %d.\n", cy_dev.cyd_hid);
	    cy_dev.cyd_baddr = if_makeaddr(in_netof(sin->sin_addr),
		INADDR_ANY);
	    printf("cyioctl: broadcast addr = 0x%x.\n", cy_dev.cyd_baddr);
	    cy_dev.cyd_haddr = sin->sin_addr;
	    printf("cyioctl: host address = 0x%x.\n", cy_dev.cyd_haddr);
	    cy_setr(cy_dev.cyd_hid, CYR_THISHOST);

	    sin = (struct sockaddr_in *)&pif->if_addr;
	    sin->sin_family = AF_INET;
	    sin->sin_addr = if_makeaddr(pif->if_net, cy_dev.cyd_hid);
	    sin = (struct sockaddr_in *) &pif->if_broadaddr;
	    sin->sin_family = AF_INET;
	    sin->sin_addr = if_makeaddr(pif->if_net, INADDR_ANY);
	    pif->if_flags |= IFF_BROADCAST | IFF_UP;
		/* set up routing table entry */
	    if (pif->if_flags & IFF_RUNNING) {
		    printf("cyioctl: if_route returns %d.\n",
		    if_rtinit(pif, RTF_UP));
	    } else
		cyinit(pif->if_unit, 0);
	    cyTurnOnMonitor();
	    break;
#endif BSD42
#ifdef BSD43
    register struct cy_ioctlarg *pcy_ioctlarg;
#ifdef sgi
    register struct sockaddr_in *sin;
#else sgi
    register struct sockaddr_in *sin, sintemp;
#endif sgi
    register int s = splimp(), error = 0;
    register int ln;		/* line number */

    if (u.u_uid)
	return(EPERM);
    switch (cmd) {
	case SIOCSIFADDR:
	    sin = (struct sockaddr_in *) data;
	    cy_dev.cyd_hid = in_lnaof(sin->sin_addr);
	    printf("cyioctl: host id = %d.\n", cy_dev.cyd_hid);
	    cy_dev.cyd_baddr = in_makeaddr(in_netof(sin->sin_addr),
		INADDR_ANY);
	    printf("cyioctl: broadcast addr = 0x%x.\n", cy_dev.cyd_baddr);
	    cy_dev.cyd_haddr = sin->sin_addr;
	    printf("cyioctl: host address = 0x%x.\n", cy_dev.cyd_haddr);
	    cy_setr(cy_dev.cyd_hid, CYR_THISHOST);
	    pif->if_flags |= IFF_BROADCAST | IFF_UP;
		/* set up routing table entry */
	    if (pif->if_flags & IFF_RUNNING) {
	    } else
		cyinit(pif->if_unit, 0);
	    cyTurnOnMonitor();
	    break;
#endif BSD43
#ifdef ULTRIX11
    register struct cy_ioctlarg *pcy_ioctlarg;
    register struct sockaddr_in *sin, sintemp;
    register int s = splimp(), error = 0;
    register int ln;		/* line number */

    if (u.u_uid)
	return(EPERM);
    switch (cmd) {
	case SIOCSIFADDR:
		if (pif->if_flags & IFF_RUNNING)
		if_rtinit(pif, -1);     /* delete previous route */
	    sin = (struct sockaddr_in *) data;
	    pif->if_net = in_netof(sin->sin_addr);
	    pif->if_host[0] = in_lnaof(sin->sin_addr);
	    cy_dev.cyd_hid = in_lnaof(sin->sin_addr);
	    printf("cyioctl: host id = %d.\n", cy_dev.cyd_hid);
	    cy_dev.cyd_baddr = if_makeaddr(in_netof(sin->sin_addr),
		INADDR_ANY);
	    printf("cyioctl: broadcast addr = 0x%x.\n", cy_dev.cyd_baddr);
	    cy_dev.cyd_haddr = sin->sin_addr;
	    printf("cyioctl: host address = 0x%x.\n", cy_dev.cyd_haddr);
	    cy_setr(cy_dev.cyd_hid, CYR_THISHOST);

	    sin = (struct sockaddr_in *)&pif->if_addr;
	    sin->sin_family = AF_INET;
	    sin->sin_addr = if_makeaddr(pif->if_net, cy_dev.cyd_hid);
	    sin = (struct sockaddr_in *) &pif->if_broadaddr;
	    sin->sin_family = AF_INET;
	    sin->sin_addr = if_makeaddr(pif->if_net, INADDR_ANY);
	    pif->if_flags |= IFF_BROADCAST | IFF_UP;
		/* set up routing table entry */
	    if (pif->if_flags & IFF_RUNNING) {
		    printf("cyioctl: if_route returns %d.\n",
		    if_rtinit(pif, RTF_UP));
	    } else
		cyinit(pif->if_unit, 0);
	    cyTurnOnMonitor();
	    break;

#endif ULTRIX11
#ifdef ULTRIX12
    register struct cy_ioctlarg *pcy_ioctlarg;
    register struct sockaddr_in *sin, sintemp;
    register int s = splimp(), error = 0;
    register int ln;		/* line number */

    if (u.u_uid)
	return(EPERM);
    switch (cmd) {
	case SIOCSIFADDR:
	    sin = (struct sockaddr_in *) data;
	    pif->if_net = in_netof(sin->sin_addr);
	    cy_dev.cyd_hid = in_lnaof(sin->sin_addr);
	    printf("cyioctl: host id = %d.\n", cy_dev.cyd_hid);
	    cy_dev.cyd_baddr = in_makeaddr(in_netof(sin->sin_addr),
		INADDR_ANY);
	    printf("cyioctl: broadcast addr = 0x%x.\n", cy_dev.cyd_baddr);
	    cy_dev.cyd_haddr = sin->sin_addr;
	    printf("cyioctl: host address = 0x%x.\n", cy_dev.cyd_haddr);
	    cy_setr(cy_dev.cyd_hid, CYR_THISHOST);
	    sin = (struct sockaddr_in *)&pif->if_addr;
	    sin->sin_family = AF_INET;
	    sin->sin_addr = in_makeaddr(pif->if_net, cy_dev.cyd_hid);
	    pif->if_flags |= IFF_BROADCAST | IFF_UP;
		/* set up routing table entry */
	    if (pif->if_flags & IFF_RUNNING) {
	    } else
		cyinit(pif->if_unit, 0);
	    cyTurnOnMonitor();
	    break;
#endif ULTRIX12
	case CYIOC_SROUTE:
	    pcy_ioctlarg = (struct cy_ioctlarg *) data;
	    ln = pcy_ioctlarg->cyi_ln;
	    sin = & (pcy_ioctlarg -> cyi_sin);
	    if (sin->sin_family != AF_INET) {
		error = EAFNOSUPPORT;
		break;
	    }
	    if (!cyValidNetNumber(sin)) {
		error = ECYBADADDR;
		break;
	    }
	    /*
	     * don't set route through any lines to ourselves. That is,
	     * Ignore the request if the destination is for us. This is
	     * needed, since setting the route would prevent us from
	     * recognizing the packet as being addressed to us.
	     */

	if (sin->sin_addr.s_addr != cy_dev.cyd_haddr.s_addr)
		cy_setr(in_lnaof(sin->sin_addr), ln);
	break;
	case CYIOC_GETLOGREC :
	    cyGetLogRec(data);
	    break;
	case CYIOC_FLUSHROUTES:
	    for (ln=0; ln<CHOSTSMAX; ln++)
		cy_setr(ln, CYR_NOROUTE);
	    break;
	case CYIOC_SROUTEIP:
	    pcy_ioctlarg = (struct cy_ioctlarg *) data;
	    sin = & (pcy_ioctlarg -> cyi_sin);
	    if (sin->sin_family != AF_INET) {
		error = EAFNOSUPPORT;
		break;
	    }
	    if (!cyValidNetNumber(sin)) {
		error = ECYBADADDR;
		break;
	    }
	    if (sin->sin_addr.s_addr != cy_dev.cyd_haddr.s_addr)
		cy_setr(in_lnaof(sin->sin_addr), CYR_THISHOST);
	    break;
	case CYIOC_GNETNUM:
	    pcy_ioctlarg = (struct cy_ioctlarg *) data;
	    sin = & (pcy_ioctlarg -> cyi_sin);
#ifdef sgi
	    bzero((char*)sin, sizeof(*sin));
#else sgi
	    bzero(sin, sizeof(*sin));
#endif sgi
	    if (cy_dev.cyd_naddr.s_addr) {
		sin->sin_family = AF_INET;
		sin->sin_addr.s_addr = cy_dev.cyd_naddr.s_addr;
	    } else
		error = ECYBADNET;
	    break;
#ifndef sgi
#ifdef BSD43
	  case CYIOC_ATTACHIF:
/*
 * this is a kudgy way to attach a "real" network interface as a Cypress line.
 * THe problem is, you can't issue ioctl's to the interface until after it has
 * been configured via SIOCSIFADDR. By then, Unix routing has already added
 * a route to its internal tables.
 */
	    {
		register int error;
		register struct sockaddr_in *sin;
		register struct ifnet *ifp;
		register int ln;
		struct ifaddr *ia = 0;
		struct mbuf *m;
		printf("ioctl(CYIOC_ATTACHIF): called\n");
		pcy_ioctlarg = (struct cy_ioctlarg *) data;
		ifp = (struct ifnet *) ifunit(pcy_ioctlarg->cyi_ifname);
		if (ifp == 0) {
		    error = ENXIO;
		    break;
		}
		if ((ln = pcy_ioctlarg->cyi_ln) < 0 || (ln >= CLINEMAX)) {
		    error = EBADLINE;
		    printf("cy_ioctl: bad line %d\n", ln);
		    break;
		}

		if (rgcyln[ln].cyl_output != NULL) {
		    printf("cy_ioctl: line already in use\n");
		    error = ELINEBUSY;
		    break;
		}
		sin = &(pcy_ioctlarg -> cyi_sin);
		if (sin->sin_family != AF_INET) {
		    printf("cy_ioctl: bogus dest addr\n");
		    error = EAFNOSUPPORT;
		    break;
		}
		if (!cyValidNetNumber(sin)) {
		    printf("cy_lattach: invalid net number\n");
		    error = ECYBADADDR;
		    break;
		}

		ia = ifp->if_addrlist;
		 /*
		  * Give interface an address. This is needed, cause the device
		  * won't configure itself properly if it does not have one.
		  */
		if (ia == (struct ifaddr *)0) {
		    m = m_getclr(M_WAIT, MT_IFADDR);
		    if (m == (struct mbuf *)NULL) {
			error = ENOBUFS;
			break;
		    }
		    ia = mtod(m, struct ifaddr *);
		    ia->ifa_ifp = ifp;
		    ia->ifa_next = NULL;
		    ia->ifa_addr.sa_family = AF_CYPRESS;
		    *(int *) ia->ifa_addr.sa_data = 0xffffffff;
		    ia->ifa_dstaddr.sa_family = AF_CYPRESS;
		    *(int *) ia->ifa_dstaddr.sa_data = 0xffffffff;
		    ifp->if_addrlist = ia;
		}
		rgcyln[ln].cyl_output = ifp->if_output;
		rgcyln[ln].cy_flags = CYF_LINEUP;
		rgcyln[ln].cyl_if = ifp;
		rgcyln[ln].cyl_dest.s_addr = sin->sin_addr.s_addr;
		if (ifp->if_ioctl)
		    error = (*ifp->if_ioctl)(ifp, CYIOC_ATTACHIF,
					     (caddr_t) &rgcyln[ln]);
		else
		    panic("CYIOC_ATTACHIF");
		if (error) {
		    printf("cyioctl: line not attached, if_ioctl failed\n");
		    rgcyln[ln].cy_flags &= ~CYF_LINEUP;
		}
		break;
	    }
#endif BSD43
#endif sgi
	default:

	    printf("cyioctl: Unknown control function 0x%x.\n", cmd);

	    error = EINVAL;
	    break;
    }
    splx(s);
    return (error);
}

#ifdef sgi
/*
 * open Cypress stream module
 */
/* ARGSUSED */
static
cy_open(rq, dev, flag, sflag)
register queue_t *rq;			/* our read queue */
dev_t dev;
int flag;
int sflag;
{
    register struct st_c *tp;
    register queue_t *wq = WR(rq);

    if (MODOPEN != sflag)		/* no cloning allowed */
	return OPENFAIL;

    if (0 != u.u_uid) {
	u.u_error = EPERM;
	return OPENFAIL;
    }

    if (0 != rq->q_ptr) {		/* can open only once */
	u.u_error = EBUSY;
	return OPENFAIL;
    }

    if (!(tp = (struct st_c*)malloc(sizeof(*tp))))
	return OPENFAIL;

    bzero((char*)tp, sizeof(*tp));
    tp->cy_rq = rq;
    tp->cy_wq = rq;
    rq->q_ptr = (caddr_t)tp;
    wq->q_ptr = (caddr_t)tp;

    return 0;
}


/* flush input or output */
static
cy_flush(op, rq, wq)
u_char op;
register queue_t *rq;
register queue_t *wq;
{
    register int s;
    register struct st_c *tp = (struct st_c*)rq->q_ptr;
    register struct cyln *pcyln = &rgcyln[tp->cy_unit];
    register struct mbuf *pm;

    ASSERT(tp->cy_rq == rq);

    if (op & FLUSHW) {			/* free partially sent packet */
	s = splstr();
	flushq(wq,FLUSHALL);
	pm = tp->cy_wmbuf;
	tp->cy_wmbuf = 0;
	(void)splimp();
	if (pm) m_freem(pm);
	for (;;) {			/* empty the mbuf queue */
	    register struct ifqueue *pifq = &pcyln->cy_send;
	    IF_DEQUEUE(pifq, pm);
	    if (!pm) break;
	    IF_DROP(pifq);
	    cy_dev.cyd_if.if_oerrors++;
	    m_freem(pm);
	}
	splx(s);
	str_unbcall(rq);		/* stop waiting for buffers */
    }

    if (op & FLUSHR) {			/* free partially received packet */
	s = splstr();
	flushq(rq,FLUSHALL);
	pm = tp->cy_rmbase;
	tp->cy_rmbase = 0;
	tp->cy_rmbtail = 0;
	(void)splimp();
	if (pm) m_freem(pm);
	splx(s);
    }
}


/* close cypress stream module */
static
cy_close(rq)
register queue_t *rq;
{
    register struct st_c *tp = (struct st_c*)rq->q_ptr;
    register struct cyln *pcyln = &rgcyln[tp->cy_unit];
    register int s;

    s = splstr();
    ASSERT(pcyln->cy_tp == tp);
    pcyln->cy_tp = NULL;
    pcyln->cy_flags &= ~CYF_LINEUP;

    cy_flush(FLUSHRW, rq, tp->cy_wq);	/* flush everything */

    free((char*)tp);
    splx(s);
}


/*
 * handle streams packets from the stream head.  They should be only IOCTLs.
 */
static
cy_wput(wq, bp)
register queue_t *wq;			/* our write queue */
register mblk_t *bp;
{
    register struct st_c *tp = (struct st_c*)wq->q_ptr;
    register struct iocblk *iocp;
    register struct cy_ioctlarg *arg;
    register int ln;			/* line number */
    register int s;

    ASSERT(tp->cy_wq == wq);

    switch (bp->b_datap->db_type) {
    case M_IOCTL:
	iocp = (struct iocblk*)bp->b_rptr;
	arg = (struct cy_ioctlarg*)bp->b_cont->b_rptr;
	switch (iocp->ioc_cmd) {
	case CYIOC_ADDLINE:
	    ASSERT(iocp->ioc_count == sizeof(*arg));
	    s = splimp();
	    if ((ln = arg->cyi_ln) < 0 || (ln >= CLINEMAX)) {
		iocp->ioc_error = EBADLINE;
	    } else if (rgcyln[ln].cy_tp != NULL) {
		iocp->ioc_error = ELINEBUSY;
	    } else {
		tp->cy_unit = ln;
		rgcyln[ln].cy_flags |= CYF_ISTTY;
		rgcyln[ln].cy_tp = tp;
	    }
	    splx(s);
	    break;

	case CYIOC_SDESTADDR:
	    ASSERT(iocp->ioc_count == sizeof(*arg));
#ifdef CYDEBUG
	    printf("cy_tioctl: Setting destination address.\n");
#endif
	    s = splimp();
	    if (((ln = tp->cy_unit) < 0) || (ln >= CLINEMAX)) {
		iocp->ioc_error = ETTYBUSY;
	    } else if (arg->cyi_sin.sin_family != AF_INET) {
		iocp->ioc_error = EAFNOSUPPORT;
	    } else if (!cyValidNetNumber(&arg->cyi_sin)) {
		iocp->ioc_error = ECYBADADDR;
	    } else {
		rgcyln[ln].cyl_dest.s_addr = arg->cyi_sin.sin_addr.s_addr;
		rgcyln[ln].cy_flags |= (CYF_LINEUP|CYF_CARRIER);
	    }
	    splx(s);
	    break;

	default:
#ifdef CYDEBUG
	    printf("cy_tioctl: unknown control function.\n");
#endif
	    iocp->ioc_error = EINVAL;
	    break;
	}
	bp->b_datap->db_type = M_IOCNAK;
	qreply(wq,bp);
	break;

    case M_DATA:			/* data does not come this way */
    case M_DELAY:			/* do not allow random controls */
	sdrv_error(wq,bp);
	break;

    case M_FLUSH:
	cy_flush(*bp->b_rptr, tp->cy_rq, wq);
	putnext(wq,bp);
	break;

    default:				/* just pass other messages on */
	putnext(wq, bp);
	break;
    }
}

#else sgi
/*
 * ============================================================
 * cy_topen - Open a tty line for use as a Cypress line
 * ============================================================
 */
cy_topen(dev, tp)
dev_t dev;
register struct tty *tp;
{
    register int s;
    register struct cy_buf *pcy_buf;
    struct cy_buf *cy_getbuf();

	/*
	 * Verify we can do this..
	 */

#ifdef CYDEBUG
printf("cy_topen: called.\n");
#endif
    if (u.u_uid)
	return(EPERM);
    if (tp->t_line == CYDISC)
	return (EBUSY);
    s = splimp();
/*
 * Set up the stuff in the tty structure.
 */
/*    bp = geteblk(CCHTTYBUF);*/
    if((pcy_buf = cy_getbuf()) == NULL)
	panic("cy_open");

    tp->cy_bufp = pcy_buf;
    tp->cy_cp =  pcy_buf->cyb_rgch;
    tp->cy_inbuf = 0;
    tp->cy_unit = 0;
    ttyflush(tp, FREAD|FWRITE);
    splx(s);
    return(0);
}

/*
 * ============================================================
 * cy_tclose - Close a tty line, the line discipline is being reset.
 * ============================================================
 */
cy_tclose(tp)
register struct tty *tp;
{
    register int s;
    register struct cyln *pcyln = mptpcyln(tp);
    s = splimp();
#ifdef CYDEBUG

printf("cy_tclose: called.\n");
#endif
    pcyln->cy_tp = NULL;
    pcyln->cy_flags &= ~CYF_LINEUP;
    if (tp->cy_bufp) {
	cy_freebuf(tp->cy_bufp);
	tp->cy_bufp = NULL;
    } else
	printf("cyclose: no buf!\n");
    tp->cy_cp = 0;
    tp->cy_inbuf = 0;
    tp->cy_unit = 0;
    tp->t_line = 0;		/* paranoid: avoid races */
    splx(s);
    return(0);
}
/*
 * ============================================================
 * cy_tioctl - Ioctl for /dev/tty being used as a cypress line.
 * ============================================================
 */
cy_tioctl(tp, cmd, data)
register struct tty *tp;
caddr_t data;
int cmd;
{
    register struct cy_ioctlarg *pcy_ioctlarg = (struct cy_ioctlarg *) data;
    register struct sockaddr_in *sin;
    register int s = splimp(), error = 0;
    register int ln;		/* line number */

    if (u.u_uid)
	return(EPERM);
    switch (cmd) {
	case CYIOC_ADDLINE:
#ifdef CYDEBUG
printf("cy_tioctl: Adding new line.\n");
#endif
	    if ((ln = pcy_ioctlarg->cyi_ln) < 0 || (ln >= CLINEMAX)) {
		error = EBADLINE;
		break;
	    }

	    if (rgcyln[ln].cy_tp != NULL) {
		error = ELINEBUSY;
		break;
	    }
	    rgcyln[ln].cy_flags |= CYF_ISTTY;
	    rgcyln[ln].cy_tp = tp;
	    rgcyln[ln].cyl_output = cy_toutput;
	    tp -> cy_unit = ln;
	    break;
	case CYIOC_SDESTADDR:
#ifdef CYDEBUG
printf("cy_tioctl: Setting destination address.\n");
#endif
	    if (((ln = tp->cy_unit) < 0) || (ln >= CLINEMAX)) {
		error = ETTYBUSY;
		break;
	    }
	    sin = &(pcy_ioctlarg -> cyi_sin);
	    if (sin->sin_family != AF_INET) {
		error = EAFNOSUPPORT;
		break;
	    }
	    if (!cyValidNetNumber(sin)) {
		error = ECYBADADDR;
		break;
	    }
	    rgcyln[ln].cyl_dest.s_addr = sin->sin_addr.s_addr;
	    rgcyln[ln].cy_flags |= (CYF_LINEUP|CYF_CARRIER);
	    break;
	default:
#ifdef CYDEBUG
printf("cy_tioctl: unknown control function.\n");
#endif
	    error = EINVAL;
	    break;
    }
    splx(s);
    return(error);
}
#endif sgi

/*
 * ============================================================
 * cyoutput - got a packet from above. Add Cypress header and
 * send to cypress layer.
 * ============================================================
 */
cyoutput(pif, pm, dst)
register struct ifnet *pif;
register struct mbuf *pm;
struct sockaddr *dst;
{
    int s, error;
    register struct cy_hdr *pcy_hdr;
    register struct sockaddr_in *dest = (struct sockaddr_in *) dst;
    register struct mbuf *pmNew;
    register int cydst;			/* destination Cypress host */
#ifndef sgi
    struct sockaddr_in *psin;
#endif

    cy_dev.cyd_cipup++;

#if !defined(BSD43)
    if (dst->sa_family != pif->if_addr.sa_family) {
	printf("cy%d: can't handle af%d\n", pif->if_unit,
	    dst->sa_family);
	m_freem(pm);
	return(EAFNOSUPPORT);
    }
#endif

    s = splimp();
    if (dest->sin_addr.s_addr == cy_dev.cyd_baddr.s_addr) {
	register struct mbuf *pmnew = m_copy(pm, 0, M_COPYALL);
	if (pmnew == NULL) {
	    m_freem(pm);
	    splx(s);
	    return(ENOBUFS);
	}
	cyIPenqueue(pmnew);
    }

/*
 * add the cypress 2 byte header.  It seems a waste to allocate an
 * mbuf for this, but it makes everything cleaner.  We'll have to
 * verify that performance is acceptable.
 */
    if (pm->m_off > MMAXOFF ||
	MMINOFF + sizeof (struct cy_hdr) > pm->m_off) {
	pmNew = m_get(M_DONTWAIT, MT_HEADER);
	if (pmNew == 0) {
	    m_freem(pm);
	    splx(s);
	    return(ENOBUFS);
	}
	pmNew->m_next = pm;
	pmNew->m_off = MMINOFF;
	pmNew->m_len = sizeof (struct cy_hdr);
	pm = pmNew;
    } else {
	pm->m_off -= sizeof (struct cy_hdr);
	pm->m_len += sizeof (struct cy_hdr);
    }
    pcy_hdr = mtod(pm, struct cy_hdr *);
#ifdef sgi
    cydst =  in_lnaof(dest->sin_addr);
#else sgi
    cydst =  in_lnaof(dest->sin_addr.s_addr);
#endif sgi
    if (dest->sin_addr.s_addr == cy_dev.cyd_baddr.s_addr) {
	pcy_hdr->cyh_dest = (char) cy_dev.cyd_hid;
	pcy_hdr->cyh_handling = CYH_RPF;
    } else {
	pcy_hdr->cyh_dest = cydst;
	pcy_hdr->cyh_handling = CYH_DIRECT;
    }
    pcy_hdr -> cyh_type = CYT_IP;
    pcy_hdr -> cyh_hopcnt = cyhopcount;	/* maximum value */
    if (cydebug) {
	printf("cyoutput: type 0x%x, handling 0x%x, hopcount 0x%x  0x%x\n",
	pcy_hdr->cyh_type & 0xff, pcy_hdr->cyh_handling, pcy_hdr->cyh_hopcnt,
	*((u_char *)pcy_hdr));
    }
    error = cy_sendpkt(pm, cydst,(struct cyln *) NULL, pcy_hdr->cyh_handling);
    splx(s);
    return (error);
}


#ifdef sgi
 /* accumulate msg blocks, copying them into an mbuf chain */
static
cy_rsrv(rq)
register queue_t *rq;			/* our read queue */
{
    register int mblen, strlim;
    register u_char *mbptr, *strptr;
    register struct mbuf *pm;
    register mblk_t *bp;
    register struct st_c *tp = (struct st_c*)rq->q_ptr;
    register struct cyln *pcyln;
    register struct mbuf *mbase;
    register int s;

    ASSERT(rq == tp->cy_rq);
    pcyln = &rgcyln[tp->cy_unit];
    ASSERT(pcyln->cy_tp == tp);

    s = splstr();
    mbase = tp->cy_rmbase;
    pm = tp->cy_rmbtail;
    tp->cy_rmbase = 0;
    tp->cy_rmbtail = 0;
    splx(s);
    bp = 0;
    for (;;) {
	if (!bp) {
	    bp = getq(rq);
	    if (!bp) break;
	}

	if (!pm) {
	    pm = m_get(M_DONTWAIT, MT_DATA);
	    if (!pm) {
		cy_dev.cyd_if.if_ierrors++;
		cy_flush(FLUSHR, rq, tp->cy_wq);
		freemsg(bp);
		if (0 != mbase) m_freem(mbase);
		mbase = 0;
		continue;		/* (avoid race) */
	    }
	    if (!mbase) {		/* start a new mbuf chain */
		tp->cy_rdle = 0;
		tp->cy_rlen = 0;
		mbase = pm;
	    } else {			/* add to existing chain */
		tp->cy_rmbtail->m_next = pm;
		tp->cy_rmbtail = pm;
	    }
	    tp->cy_rmbtail = pm;
	    pm->m_off = MMINOFF;
	    pm->m_len = 0;
	}
	mbptr = mtod(pm, u_char*) + pm->m_len;

/* Since we have to see if each byte is an STX or DLE, we may as well copy.
 *	This should be changed to check 4 bytes at a time, by masking
 *	against 0x80808080.
 */
	for (;;) {
	    register int lim = (MMAXOFF - MMINOFF) - pm->m_len;
	    strptr = bp->b_rptr;
	    strlim = bp->b_wptr - strptr;
	    if (lim > strlim) lim = strlim;
	    mblen = 0;
	    do {
		register u_char c = *strptr++;
		if (c & 0x80) {
		    if (tp->cy_rdle) {
			tp->cy_rdle = 0;
/* corrupt or missing data could make us guess wrong here, but hopefully,
 *	the IP checksum will save the world.
 */
			c = (c == DLE_STX ? STX : DLE);
		    } else if (c == DLE) {
			tp->cy_rdle = 1;
			strlim = bp->b_wptr - strptr;
			if (lim > strlim) lim = strlim;
			continue;
		    } else if (c == STX) {
			pm->m_len += mblen;
			mblen += tp->cy_rlen;
			if (mblen < sizeof(struct cy_hdr)) {
			    cyMakeLogRec(CYL_BADPKTLEN, tp->cy_unit,  mbase);
			    m_freem(mbase);
			} else if (mblen > CCHTTYBUF) {
			    cyMakeLogRec(CYL_PKTTOOBIG, tp->cy_unit, mbase);
			    m_freem(mbase);
			} else {
			    cy_input(pm, pcyln);
			}
			mbase = 0;
			pm = 0;
			break;
		    }
		}
		*mbptr++ = c;
	    } while (++mblen < lim);

	    bp->b_rptr = strptr;
	    if (strptr >= bp->b_wptr) {
		register mblk_t *bp2 = bp->b_cont;
		freeb(bp);
		bp = bp2;
	    }

	    if (0 != pm) {
		pm->m_len += mblen;
		tp->cy_rlen += mblen;
		if ((pm->m_len += mblen) >= (MMAXOFF - MMINOFF)) {
		    if (tp->cy_rlen
			> CCHTTYBUF + (MMAXOFF - MMINOFF)) {
			pm->m_len = 0;	/* detect babblers */
		    } else {
			pm = 0;
		    }
		}
	    }
	}
    }

    s = splstr();
    tp->cy_rmbase = mbase;
    tp->cy_rmbtail = pm;
    splx(s);
}
#else sgi
/*
 * ============================================================
 * cy_trint - reciever interrupt....not called, macros expand inline!
 * ============================================================
 */
cy_trint(ch, tp)
register u_char ch;
register struct tty *tp;
{
    *tp->cy_cp++ = ch;
    if ((ch&0xff) == STX  || ++tp->cy_inbuf == CCHTTYBUF)
	cy_tpktint(tp);
}


/*
 * ============================================================
 * cy_tpktint - We have seen a packet delimiter.
 * ============================================================
 */
cy_tpktint(tp)
register struct tty *tp;
{
    register struct cy_buf *pcy_bufnew,	/* new buffer */
	*pcy_buf;	/* buffer that just got filled */
    register struct cyln *pcyln = &rgcyln[tp->cy_unit];
    struct mbuf *pm;
    pcyln->cyl_cchr += tp->cy_inbuf;
    if (tp->cy_inbuf <= (sizeof (struct cy_hdr))) {
	pm = cyFrameToMbuf((tp->cy_bufp)->cyb_rgch, tp->cy_inbuf);
	cyMakeLogRec(CYL_BADPKTLEN, tp->cy_unit,  pm);
	if (pm != NULL)
	    m_freem(pm);
	tp->cy_cp = (tp->cy_bufp)->cyb_rgch;
	tp->cy_inbuf = 0;
	return;
    } else if (tp->cy_inbuf == CCHTTYBUF) { /* overrun buffer */
	pm = cyFrameToMbuf((tp->cy_bufp)->cyb_rgch, tp->cy_inbuf);
	cyMakeLogRec(CYL_PKTTOOBIG, tp->cy_unit, pm);
	if (pm != NULL)
	    m_freem(pm);
	tp->cy_cp = (tp->cy_bufp)->cyb_rgch;
	tp->cy_inbuf = 0;
	return;
    }
    /*
     * Check first that we were able to allocate a new buffer. If not,
    * we are getting packets faster than we can process them. This
     * may be caused by noise on the line.
     */
    pcy_bufnew = cy_getbuf();
    pcy_buf = tp->cy_bufp;
    if (pcy_bufnew == NULL) {
	pm = cyFrameToMbuf((tp->cy_bufp)->cyb_rgch, tp->cy_inbuf);
	cyMakeLogRec(CYL_NOBUFFERAVAIL, tp->cy_unit, pm);
	if (pm != NULL)
	    m_freem(pm);
	cy_dev.cyd_if.if_ierrors++;
	tp->cy_cp = (tp->cy_bufp)->cyb_rgch;
	tp->cy_inbuf = 0;
    } else {
	pcy_buf->cyb_pcyln = pcyln;
	pcy_buf->cyb_cch = tp->cy_inbuf;
	cy_enqueue(pcy_buf);
	tp->cy_bufp = pcy_bufnew;
	tp->cy_cp = tp->cy_bufp->cyb_rgch;
	tp->cy_inbuf = 0;
	schednetisr(NETISR_CY);
    }
}
/*
 * ============================================================
 * cy_dequeue - Remove an entry from the tty/Cypress queue
 * ============================================================
 */
struct cy_buf *cy_dequeue()
{
    register struct cy_buf *pcy_buf;
    if ((pcy_buf=cy_dev.cyd_pbufqhd) == NULL)
	return(NULL);
    cy_dev.cyd_pbufqhd = cy_dev.cyd_pbufqhd->cyb_pbuf;
    return(pcy_buf);
}

/*
 * ============================================================
 * cyintr - Software interrupt handler. Removes packets that are
 * in cy_buf structures, puts them in mbufs, passes them to IP or
 * routes them to other Cypress hosts.
 * ============================================================
 */
cyintr()
{
    register struct mbuf *pm;
    register int    len, n;
    register struct cy_buf *pcy_buf;
    register struct cyln *pcyln;
    int s = splimp();

    for (n=0; n < CLINEMAX; n++)
	cy_filldmabuf(&rgcyln[n]);

    for (pcy_buf = cy_dequeue();pcy_buf != NULL;cy_freebuf(pcy_buf),
	pcy_buf = cy_dequeue()) {
	len = pcy_buf->cyb_cch;
	if (len == 0 || len == CCHTTYBUF) {
	    pm = cyFrameToMbuf(pcy_buf->cyb_rgch, len);
	    cyMakeLogRec(CYL_BADPKTLEN, mpcylnln(pcy_buf->cyb_pcyln), pm);
	    if (pm != NULL)
		m_freem(pm);
	    continue;
	}
	if ((pm = cyFrameToMbuf(pcy_buf->cyb_rgch, len)) == NULL)
	    continue;
	cy_input(pm, &cy_dev.cyd_if, pcy_buf->cyb_pcyln);
    }
/*
 * Now get packets from real network devices
 */
#ifdef BSD43
    IF_DEQUEUE(&cyintrq, pm);
    while (pm) {
	pcyln = *mtod(pm, struct cyln **);
	IF_ADJ(pm);
	cy_input(pm, &cy_dev.cyd_if, pcyln);
	IF_DEQUEUE(&cyintrq, pm);
    }
#endif
    splx(s);
    return;
}
#endif sgi
/*
 * ============================================================
 * cy_sendpkt - send out a Cypress packet.
 * ============================================================
 */
cy_sendpkt(pm, dst, pcyln, handling)
register struct mbuf *pm;	/* packet (with Cypress header) */
register struct cyln *pcyln;	/* NULL implies packet from local host */
register  int dst;		/* which Cypress host to send the packet to */
				/* this is only the host number! */
int handling;			/* how do we route the packet */
{
    register int ln;
    struct mbuf *pmNew;
#ifdef sgi
    int clnMax;			/* how many lines have been configured? */
#else sgi
    register struct ifqueue *pifq;
    int clnMax;			/* how many lines have been configured? */
    int w, error;
#endif sgi
    struct cyln *pcyln2;
    dst &= 0xff;
    cy_dev.cyd_copln++;
    if (m_length(pm) > CYMTU) {	/* somehow a huge packet got to us */
	cyMakeLogRec(CYL_MTUEXCEEDED, mpcylnln(pcyln), pm);
	m_freem(pm);
	return(0);
    }
    switch (handling & 0x03) {
	case CYH_DIRECT:
	/*
	 * let's try to route the packet out on a line
	 */
	    if (((ln =mphln(dst)) == CYR_NOROUTE) || (ln > CLINEMAX) ) {
		if (pcyln != NULL) { /* lets try to send back an ICMP error */
		    cyForMe(pm, pcyln);
		    return(EHOSTUNREACH);
		} else {
		    m_freem(pm);
		    return(EHOSTUNREACH);
		}
	    }
	    if (ln == CYR_THISHOST) {	/* send back up (loopback) */
		cyForMe(pm, pcyln);
		return(0);
	    }
	    if (((pcyln2 = &rgcyln[ln])->cy_tp == NULL) ||
		((pcyln2->cy_flags&CYF_CARRIER) == 0)) { /* serial line not */
							/* configured yet */
		if (pcyln != NULL) { /* lets try to send back an ICMP error */
		    cyForMe(pm, pcyln);
		    return(EHOSTUNREACH);
		} else {
		    m_freem(pm);
		    return(EHOSTUNREACH);
		}
	    }
	    if (pcyln == pcyln2) {	/* sending packet back out on same */
					/* line that it came in on. Hmm... */
		cyMakeLogRec(CYL_BADROUTE, ln, pm);
	    }
#ifdef sgi
	    ASSERT(pcyln2->cy_flags & CYF_ISTTY);
	    if (cy_toutput(pcyln2, pm))
		break;
#else sgi
	    if (pcyln2->cy_flags & CYF_ISTTY)
		error = cy_toutput(pcyln2, pm);
	    else {
		struct sockaddr saddr;
		saddr.sa_family = AF_CYPRESS;
		*(int *)saddr.sa_data = dst;
		error =(*pcyln2->cyl_output)(pcyln2->cyl_if, pm, &saddr);
	    }
	    if (error)
		break;
#endif sgi

	    if (pcyln == NULL)
		pcyln2->cyl_cprip++;

	    pcyln2->cyl_cpsdirect++;
	    cycdirects++;
	    return(0);
	case CYH_FLOOD:
	case CYH_RPF:
	    for (clnMax=0, ln=0; ln <CLINEMAX; ln++)
		if ((rgcyln[ln].cy_flags &(CYF_LINEUP|CYF_CARRIER)) ==
		    (CYF_LINEUP|CYF_CARRIER)) clnMax++;
	    if (clnMax == 0) {	/* No serial lines configured */
		m_freem(pm);
		return(EHOSTUNREACH);
	    } else if ((pcyln != NULL) && (--clnMax == 0)) {
		m_freem(pm);	/* only one line, it must */
		return(0);	/* be the one the packet arrived on */
	    }
	    for (pcyln2 = rgcyln; pcyln2 < &rgcyln[CLINEMAX]; pcyln2++){
		if (((pcyln2-> cy_flags&(CYF_CARRIER|CYF_LINEUP)) ==
		      (CYF_LINEUP|CYF_CARRIER)) && (pcyln2 != pcyln)) {
		    if (--clnMax > 0) {
			if ((pmNew = m_copy(pm, 0, M_COPYALL)) == NULL) {
			    cyMakeLogRec(CYL_NOMBUFS, mpcylnln(pcyln2), pm);
			    m_freem(pm);
			    return(ENOBUFS);
			}
		    } else if (clnMax == 0) {
			pmNew = pm;
		    } else {
			printf("cysendpkt: clnMax inconsistency\n");
			break;
		    }

#ifdef sgi
		    ASSERT(pcyln2->cy_flags & CYF_ISTTY);
		    if (cy_toutput(pcyln2, pmNew))
			continue;
#else sgi
		    if (pcyln2->cy_flags & CYF_ISTTY)
			error = cy_toutput(pcyln2, pmNew);
		    else {
			struct sockaddr saddr;
			saddr.sa_family = AF_CYPRESS;
			*(int *)saddr.sa_data = dst;
			error =(*pcyln2->cyl_output)(pcyln2->cyl_if, pmNew, &saddr);
		    }
		    if (error)
			continue;
#endif sgi
		    if (pcyln == NULL)
			pcyln2->cyl_cprip++;
		    if (handling == CYH_FLOOD) {
			cycfls++;
			pcyln2->cyl_cpsflood++;
		    } else {
			pcyln2->cyl_cpsrpf++;
			cycrpfs++;
		    }
		}
	    }
	    break;
	case CYH_NN:			/* Nearest Neighbor */
	    if (pcyln)
		panic("cy_sendpkt: CYH_NN");
	    if ((ln=dst) < 0 || ln >CLINEMAX) {
		m_freem(pm);
		return(EHOSTUNREACH);
	    }
	    if ((pcyln2 = &rgcyln[ln])->cy_tp == NULL ||
		((pcyln2->cy_flags&CYF_CARRIER) == 0)) {
		m_freem(pm);
		return(EHOSTUNREACH);
	    }
#ifdef sgi
	    ASSERT(pcyln2->cy_flags & CYF_ISTTY);
	    if (cy_toutput(pcyln2, pm))
		break;
#else sgi
	    if (pcyln2->cy_flags & CYF_ISTTY)
		error = cy_toutput(pcyln2, pm);
	    else {
		struct sockaddr saddr;
		saddr.sa_family = AF_CYPRESS;
		*(int *)saddr.sa_data = dst;
		error =(*pcyln2->cyl_output)(pcyln2->cyl_if, pm, &saddr);
	    }
	    if (error)
		break;
#endif sgi
	    pcyln2->cyl_cpsNN++;
	    cycnns++;
	    return(0);
	default:
	    cyMakeLogRec(CYL_BADPKTTYPE, mpcylnln(pcyln), pm);
	    m_freem(pm);
	    break;
    }
    return(0);
}
/*
 * ============================================================
 * cy_input - Called from lower layers when a complete Cypress packet
 * has been placed in an mbuf.
 * ============================================================
 */
#ifdef sgi
cy_input(pm, pcyln)
register struct mbuf *pm;		/* packet itself */
struct cyln *pcyln;		/* line that packet arrived on */
#else sgi
cy_input(pm, pif, pcyln)
register struct mbuf *pm;		/* packet itself */
struct ifnet *pif;
struct cyln *pcyln;		/* line that packet arrived on */
#endif
{
    register struct mbuf *pmNew;
    register struct cy_hdr *pcy_hdr;
    register int handling;
#ifdef sgi
    int ln;
#else sgi
    int w, ln;
#endif sgi

/*
 * Is this a packet to be forwarded?
 */
    cy_dev.cyd_cipln++;

/*
 * m_pullup turns out to be expensive. the first mbuf in the chain
 * is copied even when there are already sizeof(struct cyhdr)
 * bytes in the first mbuf in the chain.
 */
    if (pm->m_len < 2) {
	cyMakeLogRec(CYL_BADPKTLEN, mpcylnln(pcyln), pm);
	m_freem(pm);
	return(0);
    }
    pcy_hdr = mtod(pm, struct cy_hdr *);
    handling = (pcy_hdr->cyh_handling & 0x3);
    switch (handling) {
	case CYH_DIRECT:
	    cycdirect++;
	    pcyln->cyl_cprdirect++;
	    if (mphln(pcy_hdr->cyh_dest) == CYR_THISHOST) {
		/* packet for this host */
		cyForMe(pm, pcyln);
	    } else if (pcy_hdr->cyh_hopcnt-- == 0) {	/* toss the packet its old */
		cyMakeLogRec(CYL_HOPCNTZERO, mpcylnln(pcyln), pm);
		m_freem(pm);

		return(0);
				/* should log a messages here */
	    } else{
		cy_sendpkt(pm, pcy_hdr->cyh_dest, pcyln, CYH_DIRECT);
	    }
	    break;
	case CYH_FLOOD:
	    cycfl++;
	    pcyln->cyl_cprflood++;
	    pmNew = m_copy(pm, 0, M_COPYALL);
	    if (pmNew == NULL) {
		m_freem(pm);
		return(ENOBUFS);
	    }
	    cyForMe(pmNew, pcyln);
	    if (pcy_hdr->cyh_hopcnt-- == 0) {
		cyMakeLogRec(CYL_HOPCNTZERO, mpcylnln(pcyln), pm);
		m_freem(pm);
		return(0);
	    }
	    cy_sendpkt(pm, pcy_hdr->cyh_dest, pcyln, CYH_FLOOD);
	    break;
	case CYH_RPF:
	    cycrpf++;
	    pcyln->cyl_cprrpf++;
	    if (((ln=mphln(pcy_hdr->cyh_dest)) > 0) &&
		(ln < CLINEMAX)) {
		if ((&rgcyln[ln])->cy_tp != NULL) {
		    /* we have a route to the source of the packet */
		    if (pcyln != (&rgcyln[ln])) {    /* old copy, just toss it */
			m_freem(pm);
			return(0);
		    }
		}
	    }
	    pmNew = m_copy(pm, 0, M_COPYALL);
	    if (pmNew == NULL) {
		m_freem(pm);
		return(ENOBUFS);
	    }
	    cyForMe(pmNew, pcyln);
	    if (pcy_hdr->cyh_hopcnt-- == 0) {
		cyMakeLogRec(CYL_HOPCNTZERO, mpcylnln(pcyln), pm);
		m_freem(pm);
		return(0);
	    }
	    cy_sendpkt(pm, pcy_hdr->cyh_dest, pcyln, CYH_RPF);
	    break;
	case CYH_NN:
	    cycnn++;
	    pcyln->cyl_cprNN++;
	    cyForMe(pm, pcyln);
	    break;
	default:
	    cyMakeLogRec(CYL_BADPKTTYPE, mpcylnln(pcyln), pm);
	    m_freem(pm);
	    break;
    }
    return(0);
}



/*
 * ========================================================================
 * cy_toutput - send out a packet over the given (Cypress) tty line
 * ========================================================================
 */

cy_toutput(pcyln, pm)
register struct cyln *pcyln;
register struct mbuf *pm;
{
    register struct ifqueue *pifq = &pcyln->cy_send;
#ifdef sgi
    register queue_t *wq;

    if (!pcyln->cy_tp) {		/* refuse if link is closed */
	IF_DROP(pifq);
	cy_dev.cyd_if.if_oerrors++;
	m_freem(pm);
	return(EBADLINE); 
    }

    wq = pcyln->cy_tp->cy_wq;
    ASSERT(wq->q_ptr == (caddr_t)pcyln->cy_tp);
#endif sgi

    if (IF_QFULL(pifq) ||
	(CY_QDRAIN(pifq) && m_length(pm) > cy_small)) {
	IF_DROP(pifq);
	cy_dev.cyd_if.if_oerrors++;
	m_freem(pm);
	return(ENOBUFS); /* other lines may not be congested */
    }

    IF_ENQUEUE(pifq, pm);
#ifdef sgi
    if (canenable(wq))
	qenable(wq);
#else sgi
    cy_filldmabuf(pcyln);
#endif sgi
    return(0);
}



#ifndef sgi
/*
 * ============================================================
 * cyUnstuff - take a buffer of raw characters (from the serial line)
 * and put it into an mbuf.
 * ============================================================
 */
struct mbuf *cyFrameToMbuf(rgch, len)
char rgch[];			/* the buffer or raw characters */
int len;			/* number of characters  */
				/* (guaranteed < size of buffer) */
{
    register char *pch = rgch;
    register struct mbuf *pm, *mbase;
    int mleft, n;
    pm = mbase = m_get(M_DONTWAIT, MT_DATA);
    if (pm == 0) {
	cy_dev.cyd_if.if_ierrors++;
	return(NULL);
    }
    pm->m_off = MMINOFF;
    pm->m_len = 0;
    while (len > 0) {
	mleft = MMAXOFF - (pm->m_len + pm->m_off);
	if (mleft == 0) {
   /*
    * Filled up an mbuf, get another one..
    */
	    pm->m_next = m_get(M_DONTWAIT, MT_DATA);
	    if (pm->m_next == 0) {
   /*
    * Free up our mbufs and punt..
    */
		m_freem(mbase);
		cy_dev.cyd_if.if_ierrors++;
		continue;
	    }
	    pm = pm->m_next;
	    pm->m_off = MMINOFF;
	    pm->m_len = 0;
	    mleft = MMAXOFF - pm->m_off;
	}
	n = MIN(mleft, len);
	n -= locc(DLE, n, pch);
	if (n) {
   /*
    * Copy a chunk of n characters into the mbuf..
    */
	    bcopy((caddr_t)pch, &pm->m_dat[pm->m_len], n);
	    pm->m_len += n;
	    mleft -= n;
	    len -= n;
	    pch += n;
	}
	while ((*pch&0xff) == DLE && mleft > 0 && len > 0) {
   /*
    * Handle escaped characters..
    */
	    switch (*++pch & 0xff) {
		case DLE_DLE:
		    *pch = DLE;
		    break;
		case DLE_STX:
		    *pch = STX;
		    break;
		default:
		    cy_dev.cyd_if.if_ierrors++;
		    break;
	    }
	    pm->m_dat[pm->m_len++] = *pch++;
	    len -= 2;
	    mleft--;
	}
    }
    return(mbase);

}
#endif sgi

/*
 * ============================================================
 * cyMakeLogRec - Add a log record to the queue of unread log messages
 * ============================================================
 */
cyMakeLogRec(type, ln, pm)
register int type,		/* code for type of log record */
	 ln;			/* which line the packet came in on (or -1) */
struct mbuf *pm;		/* pointer to the packet */
{
    register char *pch;
    register struct cy_logrec *pcy_logrec;
    register int len, len2;
    int s = splimp();
    pcy_logrec = &cy_dev.cyd_rglogrec[cy_dev.cyd_logrectl];
    pcy_logrec->cyl_time = time;
    pcy_logrec->cyl_type = type;
    pcy_logrec->cyl_ln = ln;
    if (pm) {
	len = MIN(m_length(pm), CCHLDATA);
	pcy_logrec->cyl_len = len;
	pch = pcy_logrec->cyl_data;
	while((len > 0) && pm) {
	    len2 = MIN(len, pm->m_len);
#ifdef sgi
	    bcopy(mtod(pm,char *), pch, len2);
#else sgi
	    bcopy(mtod(pm,u_char *), pch, len2);
#endif sig
	    pch += len2;
	    if (len2 == pm->m_len);
		pm = pm->m_next;
	    len -= len2;
	}
    } else
	pcy_logrec->cyl_len = -1;
    if ((cy_dev.cyd_logrectl=CYLOGNEXT(cy_dev.cyd_logrectl)) ==
	cy_dev.cyd_logrechd)
	cy_dev.cyd_logrechd = CYLOGNEXT(cy_dev.cyd_logrechd);
    wakeup((caddr_t) cy_dev.cyd_rglogrec);
    splx(s);
}
/*
 * ============================================================
 * cyMonitor - Timer process that snoops around the Cypress data
 * structures gathering various statistics.
 * ============================================================
 */
cyMonitor()
{
    register struct cyln *pcyln;

    for (pcyln=rgcyln; pcyln < &rgcyln[CLINEMAX]; pcyln++) {
	if ((pcyln->cy_flags & CYF_LINEUP) == 0)
	    continue;
#ifdef sgi
	if (pcyln->cy_tp->cy_wq->q_next->q_first != 0)
#else sgi
	if (pcyln->cy_tp->t_state & TS_BUSY)
#endif sgi
	    pcyln->cy_ctpbusy++;
	else
	    pcyln->cy_ctpidle++;
    }
    timeout(cyMonitor, (caddr_t) NULL, CYL_MONITORTIMEOUT);
}
/*
 * ============================================================
 * cyGetLogRec - Remove a log record from the queue of statistics
 * ============================================================
 */
cyGetLogRec(data)
caddr_t data;
{
    register struct cy_ioctlarg *pcy_ioctlarg = (struct cy_ioctlarg *) data;
    register struct cy_logrec *pcy_logrec;
    int s;

    while (cy_dev.cyd_logrechd == cy_dev.cyd_logrectl)	/* empty queue */
	sleep((caddr_t)cy_dev.cyd_rglogrec, CYPRIO);
    s = splimp();
    pcy_logrec = &cy_dev.cyd_rglogrec[cy_dev.cyd_logrechd];
    if (CYLOGNEXT(cy_dev.cyd_logrectl) == cy_dev.cyd_logrechd)
	pcy_ioctlarg->cyi_logrec.cyl_type = CYL_QOVERFLOW;
    else
	pcy_ioctlarg->cyi_logrec.cyl_type = pcy_logrec->cyl_type;
    pcy_ioctlarg->cyi_logrec.cyl_time = pcy_logrec->cyl_time;
    pcy_ioctlarg->cyi_logrec.cyl_ln= pcy_logrec->cyl_ln;
    pcy_ioctlarg->cyi_logrec.cyl_len= pcy_logrec->cyl_len;
    if(pcy_logrec->cyl_len > 0 )
	bcopy(pcy_logrec->cyl_data, pcy_ioctlarg->cyi_logrec.cyl_data,
	pcy_logrec->cyl_len);
    cy_dev.cyd_logrechd=CYLOGNEXT(cy_dev.cyd_logrechd);
    splx(s);
}

#ifdef sgi

/*
 * convert mbufs to stream buffers, and ship them to the serial line
 */
static
cy_wsrv(wq)
register queue_t *wq;			/* our write queue */
{
    register int mblen, mblim, strlim;
    register u_char *mbptr, *strptr;
    register struct mbuf *pm;
    register mblk_t *bp;
    mblk_t *bp0;
    register struct st_c *tp = (struct st_c*)wq->q_ptr;
    register struct cyln *pcyln;
    register int s;

    ASSERT(wq == tp->cy_wq);
    pcyln = &rgcyln[tp->cy_unit];
    ASSERT(pcyln->cy_tp == tp);

    if (!canput(wq->q_next)) {		/* if constitpated, */
	noenable(wq);			/* go to sleep */
	return;
    }

    bp0 = bp = 0;
    s = splstr();			/* resume old mbuf chain */
    pm = tp->cy_wmbuf;
    tp->cy_wmbuf = 0;
    splx(s);
    mblim = tp->cy_wlim;
    mbptr = tp->cy_wptr;
    for (;;) {
	if (!pm) {			/* if no old chain, */
	    s = splimp();		/* get a new chain of mbufs */
	    IF_DEQUEUE(&pcyln->cy_send, pm);
	    splx(s);
	    if (!pm) break;
	    cy_dev.cyd_if.if_opackets++;
	    mblim = pm->m_len;
	    mbptr = mtod(pm, u_char*);
	}

	if (!bp) {			/* get a new buffer */
	    bp = str_allocb(CYMTU, wq, BPRI_LO);
	    if (!bp) break;
	}

/* We have to examine each byte we send in order to convert STX to DLE_STX,
 *	so we may as well copy one byte at a time.
 *
 *	This should be improved into a loop that would fetch, check, and
 *	store 32 bits at a time.  It could check for STX by masking against
 *	0x80808080, since most traffic will be text, and UNIX text is
 *	generally 7-bit ASCII.
 *
 *	This is a bit kludgy, since it depends on the values of DLE, STX, etc.
 */
	for (;;) {
	    register int lim = mblim;
	    strptr = bp->b_wptr;
	    strlim = bp->b_datap->db_lim - strptr;
	    if (lim > strlim) lim = strlim;
	    mblen = 0;
	    do {
		register u_char c = *mbptr++;
		if ((c & 0x80)
		    && (c == STX || c == DLE)) {
		    if (--strlim < 1) {
			mbptr--;
			break;
		    }
		    *strptr++ = DLE;
		    c = (c == STX ? DLE_STX : DLE_DLE);
		    mblim -= mblen;	/* recompute buffer limit */
		    lim = mblim;
		    if (lim > strlim) lim = strlim;
		    mblen = 0;
		}
		*strptr++ = c;
	    } while (++mblen < lim);

	    if (bp->b_datap->db_lim <= strptr) {
		if (bp0)
		    bp0->b_cont = bp;
		else
		    bp0 = bp;
		bp = 0;
	    }

	    mblim -= mblen;
	    if (mblim <= 0) {
		pm = m_free(pm);
		if (0 != pm) {		/* save next mbuf in the chain */
		    mblim = pm->m_len;
		    mbptr = mtod(pm, u_char*);

		} else {		/* Packet end sequence */
		    if (!bp)
			bp = str_allocb(1, wq, BPRI_HI);
		    if (!bp) break;
		    *bp->b_wptr++ = STX;
		    break;
		}
	    }
	}
    }
    tp->cy_wmbuf = pm;
    tp->cy_wlim = mblim;
    tp->cy_wptr = mbptr;

    if (0 != bp0)			/* send what we have */
	putnext(wq,bp0);
}
#else sgi
/*
 * ============================================================
 * cy_enqueue - Enqueue a packet (in the fixed size buffer) for the
 * Cypress layer to process
 * ============================================================
 */
cy_enqueue(pcy_buf)
register struct cy_buf *pcy_buf;
{
    if (pcy_buf->cyb_state != CYBS_ALLOCATED)
	panic("cy_enqueue");
    pcy_buf -> cyb_state = CYBS_QUEUED;
    pcy_buf -> cyb_pbuf = cy_dev.cyd_pbufqhd;
    cy_dev.cyd_pbufqhd = pcy_buf;
}
/*
 * ============================================================
 * cy_getbuf - Get a buffer for use by a tty
 * ============================================================
 */
struct cy_buf *cy_getbuf()
{
    register struct cy_buf *pcy_buf;
    for (pcy_buf = cy_dev.cyd_buf; pcy_buf < &cy_dev.cyd_buf[CCYBUFMAX];
	pcy_buf++)
	if (pcy_buf->cyb_state == CYBS_FREE) {
	    pcy_buf->cyb_state = CYBS_ALLOCATED;
	    return(pcy_buf);
	}
    return(NULL);
}
/*
 * ============================================================
 * cy_freebuf - return a tty buffer to the pool of free buffers
 * ============================================================
 */
cy_freebuf(pcy_buf)
register struct cy_buf *pcy_buf;
{
    pcy_buf->cyb_state = CYBS_FREE;
}
/*
 * ============================================================
 * cy_error - increment the number of input errors. This is actually
 * called from the dmf input interrupt handler on silo overflows.
 * ============================================================
 */
cy_silooverflow(tp)
register struct tty *tp;
{
    register struct cyln *pcyln = mptpcyln(tp);
    pcyln-> cyl_csilo++;
    cy_dev.cyd_if.if_ierrors++;
/*
 * on silo overflows, we have lost a character so the packet is bad.
 * Flush the characters out of the buffer.
 */

    cyMakeLogRec(CYL_SILOOVERFLOW, tp->cy_unit, NULL);
    pcyln->cyl_cchr += tp->cy_inbuf;
    tp->cy_cp = tp->cy_bufp->cyb_rgch;
    tp->cy_inbuf = 0;
}
/*
 * ============================================================
 * cy_freedmabuf - release a dma buffer. This is called from the tty
 * driver at transmit complete interrupt time.
 * ============================================================
 */
cy_freedmabuf(pt)
struct tty *pt;
{
    register struct cyln *pcyln = &rgcyln[pt->cy_unit];
    if ((pcyln->cyl_pdmabuf1->cyd_state == CYDMAINPROGRESS) &&
	(pcyln->cyl_pdmabuf2->cyd_state == CYDMAINPROGRESS))
	panic("cy_freedmabuf");
    if (pcyln->cyl_pdmabuf1->cyd_state == CYDMAINPROGRESS) {
	pcyln->cyl_pdmabuf1->cyd_state = CYDFREE;
	schednetisr(NETISR_CY);
	return;
    }
    if (pcyln->cyl_pdmabuf2->cyd_state == CYDMAINPROGRESS) {
	pcyln->cyl_pdmabuf2->cyd_state = CYDFREE;
	schednetisr(NETISR_CY);
	return;
    }
    panic("cy_freedmabuf: none allocated\n");
}
/*
 * ============================================================
 * cy_getdmabuf - called by tty driver when it goes idle and wants
 * another buffer to start dma from.
 * ============================================================
 */
struct cy_dmabuf *cy_getdmabuf(pt)
struct tty *pt;
{
    register struct cyln *pcyln = &rgcyln[pt->cy_unit];
    if (pcyln->cyl_pdmabuf1->cyd_state == CYDMAREADY) {
	pcyln->cyl_pdmabuf1->cyd_state = CYDMAINPROGRESS;
	pcyln->cyl_cchs += pcyln->cyl_pdmabuf1->cyd_cch;
	return(pcyln->cyl_pdmabuf1);
    }
    if (pcyln->cyl_pdmabuf2->cyd_state == CYDMAREADY) {
	pcyln->cyl_pdmabuf2->cyd_state = CYDMAINPROGRESS;
	pcyln->cyl_cchs += pcyln->cyl_pdmabuf2->cyd_cch;
	return(pcyln->cyl_pdmabuf2);
    }
    return(NULL);
}
/*
 * ============================================================
 * cy_filldmabuf - check for empty dma buffers that could have a
 * packet copied into it. This routine is usually called via software
 * interrupts after the tty driver has released a dma buffer.
 * ============================================================
 */
cy_filldmabuf(pcyln)
register struct cyln *pcyln;
{
    register u_char *pch, *pchbuf;
    register struct mbuf *pm, *pm2;
    register int len;
    int  cch, cch1, cch2;
    struct cy_dmabuf *pdmabuf;
    register int s = splimp();

    if (pcyln->cyl_pdmabuf1->cyd_state != CYDFREE &&
	pcyln->cyl_pdmabuf2->cyd_state != CYDFREE) {
	splx(s);
	return;
    }
    IF_DEQUEUE(&pcyln->cy_send, pm);
    if (pm == 0) {
	splx(s);
	return;
    }
    if (pcyln->cyl_pdmabuf1->cyd_state == CYDFREE)
	pdmabuf = pcyln->cyl_pdmabuf1;
    else
	pdmabuf = pcyln->cyl_pdmabuf2;
    pdmabuf->cyd_state = CYDALLOCATED;
    splx(s);
    pchbuf = (u_char *) pdmabuf->cyd_rgch;
/*
 * Do this stuff for each mbuf in the mbuf chain which contains
 * this output packet.  Efficiency is important here.
 */
    cch = 0;
    while (pm) {
	pch = mtod(pm, u_char *);
	len = pm->m_len;
	while (len > 0) {
/*
 * Find out how many bytes in the string we can
 * handle without doing something special.
 */
	    cch1 = locc(DLE, len, pch);
	    cch2 = locc(STX, len, pch);
	    cch1 = MAX(cch1, cch2);
	    cch1 = len - cch1;
	    if (cch1) {
		bcopy(pch, pchbuf, cch1);
		len -= cch1;
		pch += cch1;
		pchbuf += cch1;
		cch += cch1;
	    }
/*
 * If there are characters left in the mbuf,
 * the first one must be special..
 * Put it out in a different form.
 */
	    if (len) {
		*pchbuf++ = DLE;
		if ((*pch&0xff) == DLE)
		    *pchbuf++ = DLE_DLE;
		else
		    *pchbuf++ = DLE_STX;
		pcyln->cy_cesc++;
		pch++;
		len--;
		cch += 2;
	    }
	}
	MFREE(pm, pm2);
	pm = pm2;
    }

/*
 * Packet end sequence
 */
    *pchbuf++ = STX;
    cch++;
    pdmabuf->cyd_cch = cch;
    if (cch > 2*CYMTU) {
	struct mbuf *pm = cyFrameToMbuf(pdmabuf->cyd_rgch);
	cyMakeLogRec(CYL_PKTTOOBIG, mpcylnln(pcyln), pm);
	if (pm)
	    m_freem(pm);
	printf("cy0: Huge packet, possible buffer corruption\n");
    }
    pdmabuf->cyd_state = CYDMAREADY;
    cy_dev.cyd_if.if_opackets++;
    ttstart(pcyln->cy_tp);

}
#endif sgi
/*
 * ============================================================
 * cyIPenqueue - enqueue an mbuf in the IP queue
 * ============================================================
 */
cyIPenqueue(pm)
register struct mbuf *pm;
{
    register struct ifqueue *pifq = &ipintrq;
#ifdef BSD43
    register struct mbuf *pmifnet;
#endif BSD43

    if (IF_QFULL(pifq)) {
	IF_DROP(pifq);
	cy_dev.cyd_if.if_collisions++;
	m_freem(pm);
	return;
    }

#ifdef BSD43
    if ((pmifnet =  m_get(M_DONTWAIT, MT_DATA)) ==NULL) {
	IF_DROP(pifq);
	cy_dev.cyd_if.if_collisions++;
	m_freem(pm);
	return;
    } else {
	pmifnet->m_off = MMINOFF;
	pmifnet->m_len = (sizeof (struct ifnet *));
	pmifnet->m_next = pm;
	*(mtod(pmifnet, struct ifnet **)) = &cy_dev.cyd_if;
    }
    IF_ENQUEUE(pifq, pmifnet);
#else
    IF_ENQUEUE(pifq, pm);
#endif
    schednetisr(NETISR_IP);
    cy_dev.cyd_copup++;
    cy_dev.cyd_if.if_ipackets++;
}

/*
 * ============================================================
 * m_length - Compute size of packet in this mbuf.
 * ============================================================
 */
m_length(pm)
register struct mbuf *pm;
{
    register int len = 0;
    while (pm) {
	len += pm->m_len;
	pm = pm->m_next;
    }
    return(len);
}
/*
 * ============================================================
 * mpcylnln - what is the line number of the specified cyln structure?
 * ============================================================
 */
mpcylnln(pcyln)
register struct cyln *pcyln;
{
    if (pcyln == NULL || pcyln->cy_tp == NULL)
	return(-1);
    if (pcyln < rgcyln || pcyln >= &rgcyln[CLINEMAX])
	panic("mpcylnln");
    return(pcyln - rgcyln);
}
/*
 * ============================================================
 * cyForMe - Packet is addressed to this implet. Pass on to
 * appropriate protocol.
 * ============================================================
 */
cyForMe(pm, pcyln)
register struct mbuf *pm;
struct cyln *pcyln;
{
    register struct cy_hdr *pcyhdr;
    register int type;
#ifndef sgi
    register struct ifqueue *pifq;
    register struct mbuf *pmifnet;
#endif sgi
    char *pch = mtod(pm,char *);
    pcyhdr = mtod(pm,struct cy_hdr *);
    type = (pcyhdr->cyh_type & 0x3);
    if (cydebug) {
	printf("cyForMe: packet = 0x%x%x%x%x\n", (*pch) & 0xff, (*(pch+1)) & 0xff,
	    (*(pch+2)) & 0xff, (*(pch+3)& 0xff));
	printf("cyForMe: type = 0x%x, dest = 0x%x, handling 0x%x\n",
	    type, pcyhdr->cyh_dest, pcyhdr->cyh_handling);
	    }

    switch (type) {
	case CYT_IP:
	    m_adj( pm, sizeof(struct cy_hdr));
	    cyIPenqueue(pm);
	    if (pcyln != NULL)
		pcyln->cyl_cpsip++;
	    break;
	case CYT_CP:		/* not handled yet */
	    m_freem(pm);
	    break;
	case CYT_XX:
	case CYT_EXTENDED:
	    cyMakeLogRec(CYL_BADPKTTYPE, mpcylnln(pcyln), pm);
	    m_freem(pm);
	    break;
	default:
	    panic("cyForMe");
	    break;

    }
}
/*
 * ============================================================
 * cyCheckNetNumber - Verify that the net number is consistent with
 * what we have seen used so far.
 * ============================================================
 */
cyValidNetNumber(psai)
register struct sockaddr_in *psai;
{
    if (cy_dev.cyd_naddr.s_addr) {
	if (in_netof(psai->sin_addr) == cy_dev.cyd_naddr.s_addr)
	    return(1);
	printf("cyCheckNetNumber: Inconsistent network addresses in use.\n");
	printf("Expected network 0x%x, network address 0x%x used.\n",
	    cy_dev.cyd_naddr.s_addr, psai->sin_addr.s_addr);
	return(0);
    } else {
	cy_dev.cyd_naddr.s_addr = in_netof(psai->sin_addr);
    }
    return(1);
}



#ifndef sgi
/*
 * ========================================================================
 * cy_state - One of the Cypress lines has changed states.
 * ========================================================================
 */

cy_state(pcyln, state)
register struct cyln *pcyln;
register int state;			/* what happened to device? */
{
    switch (state) {
      case CYLS_LOSTCARRIER:
	pcyln->cy_flags &= ~CYF_CARRIER;
	cyMakeLogRec(CYL_LOSTCARRIER, mpcylnln(pcyln), NULL);
	break;
      case CYLS_REGAINCARRIER:
	pcyln->cy_flags |= CYF_CARRIER;
	cyMakeLogRec(CYL_REGAINCARRIER, mpcylnln(pcyln), NULL);
	break;
      case CYLS_INVALIDDMVADDR:
	cyMakeLogRec(CYL_INVALIDDMVADDR, mpcylnln(pcyln), NULL);
	break;
      default:
	cyMakeLogRec(CYL_BADSTATECHANGE, mpcylnln(pcyln), NULL);
	break;
    }
    return;
}



/*
 * ========================================================================
 * cy_tmodem - one of the serial lines had a modem state change.
 * ========================================================================
 */

cy_tmodem(tp, state)
register struct tty *tp;
register int state;
{
    printf("cy_tmodem: line %d, state %d\n", mptpcyln(tp), state);
    if (state == 0)		/* lost carrier */
	cy_state(mptpcyln(tp), CYLS_LOSTCARRIER);
    else if (state == 1)
	cy_state(mptpcyln(tp), CYLS_REGAINCARRIER);
    else
	printf("cy_tmodem: unknow state change 0x%x\n", state);
/*    return(0);*/
    return(1);
}


#if	!vax
/*
 * This is a single vax instruction..
 * If using this code on another machine with a similar instruction,
 * by all means, take advantage of it..
 */
locc(ch, len, cp)
register char	ch;
register int	len;
register char  *cp;
{
	do {
		if (*cp++ == ch)
			break;
	} while (--len > 0);
	return len;
}
#endif	!vax
#endif sgi
#endif NCYPRESS > 0
