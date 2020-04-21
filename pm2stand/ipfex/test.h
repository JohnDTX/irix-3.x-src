/*
** $Source: /d2/3.7/src/pm2stand/ipfex/RCS/test.h,v $
** $Date: 89/03/27 17:11:28 $
** $Revision: 1.1 $
*/

#define	BIGNUM	2000000000

long	ioport;		 	/* I/O Port of controller */
extern long ip_ioaddr;
extern long md_ioaddr;
long mdioport;

USHORT	secsize;		/* Size of a sector */
long	dcyl, dhd, dsec;
char	dunit;			/* Which disk unit on that controller */
char	numunits;		/* How many disks on the controller */
ULONG	zero;			/* Always zero */
char	verbose;		/* On->lots of verbose output */
char	ip_flags;		/* IP flags */
char	md_flags;		/* DSD flags */
ULONG	emode;			/* Block entry mode */
char	*emodes[];		/* Names for modes */
char	writelock;		/* Disk write lock */
ULONG	ilv;			/* Interleave factor */
ULONG	clea;			/* cmpl Error Address */
ULONG	cleis;			/* cmpl Error Data */
ULONG	isuptodate[NUNIT];	/* Label is up to date flag */
ULONG	isinited[NUNIT];	/* Unit has been initialized */
extern char groupsize;
extern char cacheenable;
extern char directmode;
extern long gap1;
extern long gap2;
extern long spiralskew;

struct dtypes dtypes[], *dtype;

struct	drive {
	struct	disk_label label;
	struct	disk_bbm bbm[MAXBBM];
	short	spc;		/* Sectors per cylinder */
	short	ncyl;		/* Usable cylinders */
} drives[NUNIT], *drivep;

#define	CYL	(drivep->ncyl)
#define	HD	(drivep->label.d_heads)
#define	SEC	(drivep->label.d_sectors)
#define	SPC	(drivep->spc)

char	*getline();
char	*itoa();

extern  iopb_t *iopb;
extern  uib_t *uib;
extern  mdiopb_t *mdiop;
struct wub *wub;
struct ccb *ccb;
struct cib *cib;
struct inib *iip;
struct tapest *tpstatus;

#define	VP0(s) if(verbose && !quiet) printf(s)
#define	VP1(s,a) if(verbose && !quiet) printf(s,a)
#define	VP2(s,a,b) if(verbose && !quiet) printf(s,a,b)
#define	VP3(s,a,b,c) if(verbose && !quiet) printf(s,a,b,c)
