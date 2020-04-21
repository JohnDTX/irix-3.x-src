/*
 * $Source: /d2/3.7/src/sys/ipII/RCS/conf.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:37 $
 */

#include "../h/param.h"
#include "../h/conf.h"
#include "../h/termio.h"
#include "../streams/stream.h"
#include "../streams/stty_ld.h"

extern	nodev(), nulldev();
extern	mmread(), mmwrite(), mmioctl();
extern	syopen(), syread(), sywrite(), syioctl();
extern  struct streamtab duinfo;

#ifndef	KOPT_NOGL
extern	struct streamtab wninfo;
#define WNINFO (&wninfo)
short win_dev = 5;			/* cdevsw entry # */
#else
#define	WNINFO 0
#endif

short duart_dev = 3;			/* duart entry # */
short console_dev = 0;			/* system console */

extern struct streamtab clninfo;


#include "md.h"
#if NMD > 0
extern	mdopen(), mdread(), mdwrite(), mdstrategy(), mdprint(),
	mfopen(), mfread(), mfwrite(), mfstrategy(), mfioctl(), mfprint(),
	qicopen(), qicclose(), qicread(), qicwrite(), qicioctl();
#else
#define	mdopen		nodev
#define	mdread		nodev
#define	mdwrite		nodev
#define	mdstrategy	nodev
#define	mdprint		nodev
#define	mfopen		nodev
#define	mfread		nodev
#define	mfwrite		nodev
#define	mfstrategy	nodev
#define	mfprint		nodev
#define	mfioctl		nodev
#define	qicopen		nodev
#define	qicclose	nodev
#define	qicread		nodev
#define	qicwrite	nodev
#define	qicioctl	nodev
#endif

#include "nfs.h"
#if NNFS > 0
#include "../nfs/nfs_export.h"

dev_t	nfs_major = 3;
#else
#define	nfs_strategy	nodev
#endif

#include "ip.h"
#if NIP > 0
extern	ipopen(), ipread(), ipwrite(), ipstrategy(), ipprint();
#else
#define	ipopen		nodev
#define	ipread		nodev
#define	ipwrite		nodev
#define	ipstrategy	nodev
#define	ipprint		nodev
#endif

#include "sd.h"
#if NSD > 0
extern	stdopen(), stdread(), stdwrite(), stdstrategy(), stdprint(),
	stddump();
#else
#define	stdopen		nodev
#define	stdread		nodev
#define	stdwrite	nodev
#define	stdstrategy	nodev
#define	stdprint	nodev
#define	stddump		nodev
#endif

#include "si.h"
#if NSI > 0
extern	siiopen(), siiread(), siiwrite(), siistrategy(), siiprint();
extern	sifopen(), sifclose(), sifread(), sifwrite(), sifstrategy(), sifprint();
extern  sifioctl();
#define	siidump	nodev
/*	siidump(); */
#else
#define	siiopen		nodev
#define	siiread		nodev
#define	siiwrite	nodev
#define	siistrategy	nodev
#define	siiprint	nodev
#define	siidump		nodev
#define	sifopen		nodev
#define	sifread		nodev
#define	sifwrite	nodev
#define	sifstrategy	nodev
#define	sifprint	nodev
#define sifioctl	nodev
#define sifclose	nodev
#endif

#include "sq.h"
#if NSQ > 0
extern	siqopen(), siqclose(), siqread(), siqwrite(), siqioctl();
#else
#define	siqopen		nodev
#define siqclose	nodev
#define	siqread		nodev
#define	siqwrite	nodev
#define	siqioctl	nodev
#endif

#include "ad.h"
#if NAD > 0
extern adopen(), adclose(), adread(), adwrite(), adioctl();
#else
#define adopen		nodev
#define	adclose		nodev
#define	adread		nodev
#define	adwrite		nodev
#define	adioctl		nodev
#endif

struct bdevsw bdevsw[] = {
	{ nodev,	nodev,		nodev,
	  nodev,	nodev },				/* 0 */
	{ mdopen,	nulldev,	mdstrategy,
	  mdprint,	nodev },				/* 1 */
	{ mfopen,	nulldev,	mfstrategy,
	  mfprint,	nodev },				/* 2 */
	{ nodev,	nodev,		nfs_strategy,
	  nodev,	nodev },				/* 3 */
	{ ipopen,	nulldev,	ipstrategy,
	  ipprint,	nodev },				/* 4 */
	{ stdopen,	nulldev,	stdstrategy,
	  stdprint,	stddump },				/* 5 */
	{ nodev,	nodev,		nodev,
	  nodev,	nodev },				/* 6 */
	{ siiopen,	nulldev,	siistrategy,
	  siiprint,	siidump },				/* 7 */
	{ sifopen,	sifclose,	sifstrategy,
	  sifprint,	nodev },				/* 8 */
};
short	bdevcnt = sizeof(bdevsw)/sizeof(bdevsw[0]);

#include "nx.h"
#if NNX > 0
extern	nxopen(), nxclose(), nxread(), nxwrite(),
	nxioctl();
dev_t	nx_dev = 6;			/* index in cdevsw */
#else
#define	nxopen		nodev
#define	nxclose		nodev
#define	nxread		nodev
#define	nxwrite		nodev
#define	nxioctl		nodev
#endif

#include "pty.h"
#if NPTY > 0
extern struct streamtab ptcinfo, ptsinfo;
#define PTCINFO (&ptcinfo)
#define PTSINFO (&ptsinfo)
#else
#define PTCINFO 0
#define PTSINFO 0
#endif

#include "ib.h"
#if NIB > 0
extern	ibopen(), ibclose(), ibread(), ibwrite(), ibioctl();
extern	ibropen(), ibrclose(), ibrread(), ibrwrite();
#else
#define ibopen		nodev
#define ibclose		nodev
#define ibread		nodev
#define ibwrite		nodev
#define ibioctl		nodev
#define ibrread		nodev
#define ibropen		nodev
#define ibrclose	nodev
#endif

#include "pxd.h"
#if NPXD > 0
extern	pxdopen(), pxdclose(), pxdread(), pxdwrite(),
	pxdioctl();
#else
#define	pxdopen		nodev
#define	pxdclose	nodev
#define	pxdread		nodev
#define	pxdwrite	nodev
#define	pxdioctl	nodev
#endif

#include "ik.h"
#if NIK > 0
extern	ikcopen(), ikcclose(), ikcwrite(), ikcioctl();
#define ikcread		nodev
#else
#define	ikcopen		nodev
#define ikcclose	nodev
#define	ikcread		nodev
#define	ikcwrite	nodev
#define ikcioctl	nodev
#endif

#include "tm.h"
#if NTM > 0
extern	tmtopen(), tmtclose(), tmtread(), tmtwrite(),
	tmtstrategy(), tmtioctl(), tmtprint();
#else
#define	tmtopen		nodev
#define tmtclose	nodev
#define	tmtread		nodev
#define	tmtwrite	nodev
#define	tmtstrategy	nodev
#define	tmtprint	nodev
#define tmtioctl	nodev
#endif

#include "cd.h"
#if NCD > 0
extern  struct streamtab cdsioinfo;
#define	CDSIOINFO	(&cdsioinfo)
#else
#define	CDSIOINFO	(0)
#endif

#include "hy.h"
#if NHY > 0
extern rhyopen(), rhyclose(), rhyread(), rhywrite(), rhyioctl();
#else
#define rhyopen		nodev
#define rhyclose	nodev
#define rhyread		nodev
#define rhywrite	nodev
#define rhyioctl	nodev
#endif

extern	int qdopen();
dev_t	queue_dev = makedev(7,0);		/* major of this device */

struct cdevsw cdevsw[] = {
	{ nodev,	nodev,		nodev,
	  nodev,	nodev,		0,	/* 0: console */
	  0 },
	{ syopen,	nulldev,	syread,
	  sywrite,	syioctl,	0,	/* 1: /dev/tty */
	  0 },
	{ nulldev,	nulldev,	mmread,
	  mmwrite,	mmioctl,	0,	/* 2: /dev/{mem,kmem,swap} */
	  0 },
	{ nodev,	nodev,		nodev,
	  nodev,	nodev,		0,	/* 3: duarts */
	  &duinfo },
	{ mdopen,	nulldev,	mdread,
	  mdwrite,	nulldev,	0,	/* 4: dsd disk */
	  0 },
	{ nodev,	nodev,		nodev,
	  nodev,	nodev,		0,	/* 5: window (graphics) */
	  WNINFO },
	{ nxopen,	nxclose,	nxread,
	  nxwrite,	nxioctl,	0,	/* 6: /dev/ttyn* (xns) */
	  0 },
	{ qdopen,	nodev,		nodev,
	  nodev,	nodev,		0,	/* 7: /dev/queue */
	  0 },
	{ nodev,	nodev,		nodev,
	  nodev,	nodev,		0,	/* 8: RESERVED */
	  0 },
	{ nodev,	nodev,		nodev,
	  nodev,	nodev,		0,	/* 9: RESERVED */
	  0 },
	{ nodev,	nodev,		nodev,
	  nodev,	nodev,		0,	/* 10: RESERVED */
	  0 },
	{ nodev,	nodev,		nodev,
	  nodev,	nodev,		0,	/* 11: RESERVED */
	  0 },
	{ mfopen,	nulldev,	mfread,
	  mfwrite,	mfioctl,	0,	/* 12: dsd floppy */
	  0 },
	{ qicopen,	qicclose,	qicread,
	  qicwrite,	qicioctl,	0,	/* 13: dsd cart tape */
	  0 },
	{ ipopen,	nulldev,	ipread,
	  ipwrite,	nulldev,	0,	/* 14: ip disk */
	  0 },
	{ nodev,	nulldev,	nulldev,
	  nulldev,	nulldev,	0,	/* 15: cd3100 sio card */
	  CDSIOINFO },
	{ nodev,	nodev,		nodev,
	  nodev,	nodev,		0,	/* RESERVED 16: dr11w */
	  0 },
	{ ibopen,	ibclose,	ibread,
	  ibwrite,	ibioctl,	0,	/* 17: gpib-796 */
	  0 },
	{ stdopen,	nulldev,	stdread,
	  stdwrite,	nulldev,	0,	/* 18: Storager 1 disk */
	  0 },
	{ nodev,	nulldev,	nulldev,
	  nulldev,	nulldev,	0,	/* 19: RESERVED */
	  0 },
	{ tmtopen,	tmtclose,	tmtread,
	  tmtwrite,	tmtioctl,	0,	/* 20: Tapemaster 1000 */
	  0 },
	{ nodev,	nodev,		nodev,
	  nodev,	nodev,		0,	/* 21: ptc (1/2 pty) */
	  PTCINFO },
	{ nodev,	nodev,		nodev,
	  nodev,	nodev,		0,	/* 22: pts (1/2 pty) */
	  PTSINFO },
	{ pxdopen,	pxdclose,	pxdread,
	  pxdwrite,	pxdioctl,	0,	/* 23: pxd comms */
	  0 },
	{ nodev,	nodev,		nodev,
	  nodev,	nodev,		0,	/* 24: clone 'driver' */
	  &clninfo },
	{ nodev,	nulldev,	nulldev,
	  nulldev,	nulldev,	0,	/* 25: RESERVED */
	  0 },
	{ adopen,	adclose,	adread,	/* 26: Adaptec M1530 */
	  adwrite,	adioctl,	0,
	  0 },
	{ ikcopen,	ikcclose,	ikcread,
	  ikcwrite,	ikcioctl,	0,	/* 27: Ikon Centronics */
	  0 },
	{ siiopen,	nulldev,	siiread,
	  siiwrite,	nulldev,	0,	/* 28: Storager 2 */
	  0 },
	{ siqopen,	siqclose,	siqread,
	  siqwrite,	siqioctl,	0,	/* 29: Storager 2 Tape */
	  0 },
	{ sifopen,	sifclose,	sifread,
	  sifwrite,	sifioctl,	0,	/* 30: Storager 2 Floppy */
	  0 },
	{ rhyopen,	rhyclose,	rhyread,
	  rhywrite,	rhyioctl,	0,	/* 31: Hyperchannel Raw If */
	  0 },
};
short	cdevcnt = sizeof(cdevsw)/sizeof(cdevsw[0]);



/* XXX tty output low and high water marks */
#define M	3
#define N	1
int	tthiwat[16] = {
	0*M,	60*M,	60*M,	60*M,	60*M,	60*M,	60*M,	120*M,
	120*M,	180*M,	180*M,	240*M,	240*M,	240*M,	100*M,	100*M,
};
int	ttlowat[16] = {
	0*N,	20*N,	20*N,	20*N,	20*N,	20*N,	20*N,	40*N,
	40*N,	60*N,	60*N,	80*N,	80*N,	80*N,	50*N,	50*N,
};

/* XXX default terminal characteristics */
char	ttcchar[NCC] = { CINTR, CQUIT, CERASE, CKILL, CEOF, 0, 0, 0 };

/* XXX the preceding stuff should go away when the streams tty code
 *	completely replaces the old tty stuff.
 */



/* default/initial streams 'line discipline' settings */
struct stty_ld def_stty_ld = {
	{				/* st_termio			*/
		ICRNL|ISTRIP,		/* st_iflag: input modes	*/
		OPOST|ONLCR|TAB3,	/* st_oflag: output modes	*/
		SSPEED|CS8|HUPCL|CLOCAL,/* st_cflag: control modes	*/
		ISIG|ICANON|ECHO|ECHOK,	/* st_lflag: line discipline modes */
		LDISC0,			/* st_line:  line discipline	*/
		{CINTR,CQUIT,CERASE,CKILL,CEOF,0,0,0},	/* st_cc	*/
	},
	CWERASE,			/* st_werase:word erase		*/
	CRPRNT,				/* st_retype:retype line	*/
	CESC,				/* st_esc:   take next literally*/
	CFLUSH,				/* st_flushc:flush output	*/
};


int	cmask = CMASK;		/* default file creation mask */
int	cdlimit = CDLIMIT;	/* default file size limit */

dev_t	rootdev = makedev(0, 0);
dev_t	swapdev = makedev(0, 1);	/* XXX */


short	rootfs = 0xFF;		/* root fs to use when forced */
daddr_t	swplo = 0;		/* swap base address to use */
daddr_t	nswap = 0;		/* swap length to use */
