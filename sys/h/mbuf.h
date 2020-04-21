/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)mbuf.h	7.2 (Berkeley) 9/11/86
 */

/*
 * Constants related to memory allocator.
 */
#define	MSIZE		128			/* size of an mbuf */

#define	MMINOFF		12			/* mbuf header length */
#define	MTAIL		4
#define	MMAXOFF		(MSIZE-MTAIL)		/* offset where data ends */
#define	MLEN		(MSIZE-MMINOFF-MTAIL)	/* mbuf data length */
#ifndef sgi
#define	NMBCLUSTERS	256
#else
#ifdef mips
#define NMBCLUSTERS	64			/* max # of clusters */
#else /* mips */
#define CLBYTES NBPG			/* this should be cleaned up someday */
#endif /* mips */
#endif /* sgi */
#define	NMBPCL		(CLBYTES/MSIZE)		/* # mbufs per cluster */

/*
 * Macros for type conversion
 */

#ifndef mips
/* network cluster number to virtual address, and back */
#define	cltom(x) ((struct mbuf *)((int)usrpt + ((x) << PGSHIFT)))
#define	mtocl(x) (((int)x - (int)usrpt) >> PGSHIFT)
#endif

/* address in mbuf to mbuf head */
#define	dtom(x)		((struct mbuf *)((int)x & ~(MSIZE-1)))

/* mbuf head, to typed data */
#define	mtod(x,t)	((t)((int)(x) + (x)->m_off))

struct mbuf {
	struct	mbuf *m_next;		/* next buffer in chain */
	u_long	m_off;			/* offset of data */
	short	m_len;			/* amount of data in this mbuf */
	short	m_type;			/* mbuf type (0 == free) */
#ifdef sgi
	union {
		/*
		 * For sgi systems, clusters can be special.  Special
		 * clusters have their underlying memory somewhere else.
		 * The freefunc is used with the given arguments when the
		 * buffer is done being used.
		 */
		struct {
			int	(*mu_freefunc)();
			long	mu_farg;
			int	(*mu_dupfunc)();
			long	mu_darg;
		} m_us;
		u_char	mu_dat[MLEN];	/* data storage */
	} m_u;
#else
	u_char	m_data[MLEN];		/* data storage */
#endif
	struct	mbuf *m_act;		/* link in higher-level mbuf list */
};
#ifdef sgi
#define	m_dat		m_u.mu_dat		/* compatability */
#define	m_freefunc	m_u.m_us.mu_freefunc	/* lazy */
#define	m_farg		m_u.m_us.mu_farg
#define	m_dupfunc	m_u.m_us.mu_dupfunc
#define	m_darg		m_u.m_us.mu_darg
#endif

/* mbuf types */
#define	MT_FREE		0	/* should be on free list */
#define	MT_DATA		1	/* dynamic (data) allocation */
#define	MT_HEADER	2	/* packet header */
#define	MT_SOCKET	3	/* socket structure */
#define	MT_PCB		4	/* protocol control block */
#define	MT_RTABLE	5	/* routing tables */
#define	MT_HTABLE	6	/* IMP host tables */
#define	MT_ATABLE	7	/* address resolution tables */
#define	MT_SONAME	8	/* socket name */
#define	MT_ZOMBIE	9	/* zombie proc status */
#define	MT_SOOPTS	10	/* socket options */
#define	MT_FTABLE	11	/* fragment reassembly header */
#define	MT_RIGHTS	12	/* access rights */
#define	MT_IFADDR	13	/* interface address */
#ifdef sgi
#define MT_MAX		14	/* 'largest' type */
#endif

/* flags to m_get */
#define	M_DONTWAIT	0
#define	M_WAIT		1

/* flags to m_pgalloc */
#define	MPG_MBUFS	0		/* put new mbufs on free list */
#define	MPG_CLUSTERS	1		/* put new clusters on free list */
#define	MPG_SPACE	2		/* don't free; caller wants space */

/* length to m_copy to copy all */
#define	M_COPYALL	1000000000

/*
 * m_pullup will pull up additional length if convenient;
 * should be enough to hold headers of second-level and higher protocols. 
 */
#define	MPULL_EXTRA	32

#ifdef sgi
#define	MFREE(m, n)	(n) = m_free(m)
#define	MGET(m, i, t)	(m) = m_get(i, t)

#else
#define	MGET(m, i, t) \
	{ int ms = splimp(); \
	  if ((m)=mfree) \
		{ if ((m)->m_type != MT_FREE) panic("mget"); (m)->m_type = t; \
		  mbstat.m_mtypes[MT_FREE]--; mbstat.m_mtypes[t]++; \
		  mfree = (m)->m_next; (m)->m_next = 0; \
		  (m)->m_off = MMINOFF; } \
	  else \
		(m) = m_more(i, t); \
	  splx(ms); }
/*
 * Mbuf page cluster macros.
 * MCLALLOC allocates mbuf page clusters.
 * Note that it works only with a count of 1 at the moment.
 * MCLGET adds such clusters to a normal mbuf.
 * m->m_len is set to CLBYTES upon success.
 * MCLFREE frees clusters allocated by MCLALLOC.
 */
#define	MCLALLOC(m, i) \
	{ int ms = splimp(); \
	  if (mclfree == 0) \
		(void)m_clalloc((i), MPG_CLUSTERS, M_DONTWAIT); \
	  if ((m)=mclfree) \
	     {++mclrefcnt[mtocl(m)];mbstat.m_clfree--;mclfree = (m)->m_next;} \
	  splx(ms); }
#define	M_HASCL(m)	((m)->m_off >= MSIZE)
#define	MTOCL(m)	((struct mbuf *)(mtod((m), int)&~CLOFSET))

#define	MCLGET(m) \
	{ struct mbuf *p; \
	  MCLALLOC(p, 1); \
	  if (p) { \
		(m)->m_off = (int)p - (int)(m); \
		(m)->m_len = CLBYTES; \
	  } \
	}
#define	MCLFREE(m) { \
	if (--mclrefcnt[mtocl(m)] == 0) \
	    { (m)->m_next = mclfree;mclfree = (m);mbstat.m_clfree++;} \
	}
#define	MFREE(m, n) \
	{ int ms = splimp(); \
	  if ((m)->m_type == MT_FREE) panic("mfree"); \
	  mbstat.m_mtypes[(m)->m_type]--; mbstat.m_mtypes[MT_FREE]++; \
	  (m)->m_type = MT_FREE; \
	  if (M_HASCL(m)) { \
		(n) = MTOCL(m); \
		MCLFREE(n); \
	  } \
	  (n) = (m)->m_next; (m)->m_next = mfree; \
	  (m)->m_off = 0; (m)->m_act = 0; mfree = (m); \
	  splx(ms); \
	  if (m_want) { \
		  m_want = 0; \
		  wakeup((caddr_t)&mfree); \
	  } \
	}
#endif	/* sgi */

/*
 * Mbuf statistics.
 */
struct mbstat {
	short	m_mbufs;	/* mbufs obtained from page pool */
	short	m_clusters;	/* clusters obtained from page pool */
	short	m_clfree;	/* free clusters */
	short	m_drops;	/* times failed to find space */
#ifdef sgi
	short	m_mtypes[MT_MAX];	/* type specific mbuf allocations */
#else
	short	m_mtypes[256];	/* type specific mbuf allocations */
#endif
};

#ifdef sgi
#ifdef	KERNEL
struct	mbstat mbstat;
#ifdef mips
struct	mbuf *mbufree;		/* work around the other 'mfree' in mount */
#else
struct	mbuf *mfree;
#endif
int	m_want;
char	mclrefcnt[NMBCLUSTERS + 1];
#ifndef mips
extern	struct pte	usrpt[];	/* virtual address of usrpt ... */
#endif

extern	struct mbuf	*m_get();	/* alloc an mbuf */
extern	struct mbuf	*m_getclr();	/* alloc a cluster */
extern	struct mbuf	*m_free();	/* free an mbuf/cluster */
extern	struct mbuf	*m_copy();	/* copy an mbuf */
extern	struct mbuf	*m_pullup();	/* condense an mbuf */
extern	int		m_clfree();	/* free a "normal" cluster */
extern	int		m_cldup();	/* share a "normal" cluster */
extern	struct mbuf	*m_clget();	/* get a cluster */
extern	caddr_t		m_pgget();	/* allocate a page */
#endif

#else /* !sgi */
#ifdef	KERNEL
extern	struct mbuf mbutl[];		/* virtual address of net free mem */
extern	struct pte Mbmap[];		/* page tables to map Netutl */
struct	mbstat mbstat;
int	nmbclusters;
struct	mbstat mbstat;
struct	mbuf *mfree, *mclfree;
char	mclrefcnt[NMBCLUSTERS + 1];
int	m_want;
struct	mbuf *m_get(),*m_getclr(),*m_free(),*m_more(),*m_copy(),*m_pullup();
caddr_t	m_clalloc();
#endif
#endif
