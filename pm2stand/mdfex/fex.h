/*
** fex.h	- Copyright (C) Silicon Graphics - Mountain View, CA 94043
**		- Author: chase
**		- Date: April 1985
**		- Any use, copy or alteration is strictly prohibited
**		- and gosh darn awfully -- morally inexcusable
**		- unless authorized by SGI.
**
**	$Author: root $
**	$State: Exp $
**	$Source: /d2/3.7/src/pm2stand/mdfex/RCS/fex.h,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:11:42 $
*/

#define	index	strchr		/* System V ism */
#define	BIGNUM	2000000000

ULONG	ioport;		 	/* I/O Port of controller */

USHORT	secsize;		/* Size of a sector */
ULONG	dcyl, dhd, dsec;	/* Results from explode() */
char	dunit;			/* Which disk unit on that controller */
char	tunit;			/* Which Tape unit on that controller */
ULONG	zero;			/* Always zero */
char	verbose;		/* On->lots of verbose output */
ULONG	sd_flags;		/* Managed by sisubs */
char	emode;			/* Block entry mode */
char	*emodes[];		/* Names for modes */
char 	firmretry;		/* Firmware retries are enabled */
char	Retry;			/* Error Retries */
char	incompex;		/* Error printout */
char	writelock;		/* Disk write lock */
char	secnotfound;		/* Added for 4.6 tktest */
ULONG	lbas_left;		/* Added for 4.6 tktest */
ULONG	ilv;			/* Interleave factor */
ULONG	clea;			/* cmpl Error Address */
ULONG	cleis;			/* cmpl Error Data */
char	isuptodate[NUNIT];	/* Label is up to date flag */
char	isinited[NUNIT];	/* Unit has been initialized */
extern	ULONG time;
extern	ULONG inc_time;
extern	ULONG total_time;
extern	ULONG hour_time;
extern	ULONG globltime;
extern	char keepdate;
extern	char keeptime;
extern	char clkinit;
extern	char dateinit;
extern USHORT switches;
extern char autosetup;
extern char graphicsscreen;

struct dtypes dtypes[], *dtype;

struct	drive {
	struct	disk_label label;
	struct	disk_bbm bbm[MAXBBM];
	short	tdev;		/* DSD Device type */
	short	spc;		/* Sectors per cylinder */
	short	ncyl;		/* Usable cylinders */
} drives[NUNIT], floppydrive, *drivep;

#define	CYL	(drivep->ncyl)
#define	HD	(drivep->label.d_heads)
#define	SEC	(drivep->label.d_sectors)
#define	SPC	(drivep->spc)

char	*getline();
char	*itoa();

extern wub_t *wub;
extern ccb_t *ccb;
extern cib_t *cib;
extern struct	iopb	*iop;
extern struct	inib	*iip;
extern struct tapest *tpstatus;				/* 217 Tape status */

#define	VP0(s) if(verbose && !quiet) printf(s)
#define	VP1(s,a) if(verbose && !quiet) printf(s,a)
#define	VP2(s,a,b) if(verbose && !quiet) printf(s,a,b)
#define	VP3(s,a,b,c) if(verbose && !quiet) printf(s,a,b,c)

#ifdef NOTDEF
#define spl0()	asm("movw #0x2000,sr");
#define spl5()	asm("movw #0x2500,sr");
#define spl7()  asm("movw #0x2700,sr");
#endif
