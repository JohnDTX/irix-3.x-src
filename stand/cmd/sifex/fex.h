/*
**	fex.h		- Copyright (C), JCS Computer Services 1983
**			- Author: chase
**			- Date: January 1985
**			- Any use, copying or alteration is prohibited
**			- and morally inexcusable,
**			- unless authorized in writing by JCS.
**	$Source: /d2/3.7/src/stand/cmd/sifex/RCS/fex.h,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:13:14 $
*/

#define	BIGNUM	2000000000

long		st_ioport;	 	/* I/O Port of controller */
long		dcyl, dhd, dsec;

u_short		secsize[NUNIT];		/* Size of a sector */
char		errhalt;
u_long		esditype[NUNIT];
char		dunit;			/* Which disk unit on that controller */
char		verbose;		/* On->lots of verbose output */
char		st_flags;		/* IP flags */
char		writelock;		/* Disk write lock */
char		informat;

u_long		emode;			/* Block entry mode */
char		*emodes[];		/* Names for modes */

u_long		ilv;			/* Interleave factor */
u_long		Fmtwait;		/* Time delay in formatting */
u_long		floppy;
u_long		clea;			/* cmpl Error Address */
u_long		cleis;			/* cmpl Error Data */
u_long		isuptodate[NUNIT];	/* Label is up to date flag */
u_long		isinited[NUNIT];	/* Unit has been initialized */
char		zero;

extern char 	cacheenable;
extern char 	zerolatency;
extern char 	stretries;		/* Retries in the storager firmware */
extern char	steccon;
extern char	stmvbad;
extern char	streseek;
extern char	stinchd;
extern long 	gap1;
extern long 	gap2;
extern long 	gap3;
extern long 	spiralskew;
extern u_long 	nwait;

extern u_long	time;
extern u_long	inc_time;
extern u_long	total_time;
extern char 	keepdate;
extern char 	keeptime;
extern char 	dateinit;
extern char 	clkinit;

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

extern  iopb_t *iop;
extern	iopb_t *tiop;
extern  uib_t *uib;

#define	VP0(s) if(verbose && !quiet) printf(s)
#define	VP1(s,a) if(verbose && !quiet) printf(s,a)
#define	VP2(s,a,b) if(verbose && !quiet) printf(s,a,b)
#define	VP3(s,a,b,c) if(verbose && !quiet) printf(s,a,b,c)

extern short password;
extern short passloop;
