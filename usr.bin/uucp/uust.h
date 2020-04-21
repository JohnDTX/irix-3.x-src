/* @(#)uust.h	1.4 */
/* $Source: /d2/3.7/src/usr.bin/uucp/RCS/uust.h,v $*/
/* $Revision: 1.1 $*/
/* $Date: 89/03/27 18:31:01 $*/

#ifdef UUSTAT
#define US_RRS(a,b)	us_rrs(a,b)
#define US_CRS(a)	us_crs(a)
#define US_SST(a)	us_sst(a)
#define	USRF(flag)	Usrf |= flag
#else
#define US_RRS(a,b)	 
#define US_CRS(a)	 
#define US_SST(a)	 
#define	USRF(flag)
#endif
 
#define L_stat		"/usr/lib/uucp/L_stat"	/* system status */
#define R_stat		"/usr/lib/uucp/R_stat"	/* request status */
#define LCKLSTAT	"/usr/spool/uucp/LCK.LSTAT" /* L_stat lock */
#define LCKRSTAT	"/usr/spool/uucp/LCK.RSTAT" /* R_stat lock */

/*
 * L_stat format
 */
struct	us_ssf {
	 char	sysname[SYSNSIZE+1];	/* system name */
	 time_t	sti;			/* status time */
	 time_t	sucti;			/* successful status time */
	 short	sstat;			/* system status */
};
 
/*
 * R_stat format
 */
struct	us_rsf {
	 short	jobn;		 /* job # */
	 char	user[SYSNSIZE+1];/* login user id */
	 char	rmt[SYSNSIZE+1]; /* remote system name */
	 time_t	qtime;		 /* time the command is queued */
	 time_t	stime;		 /* status time */
	 short	ustat;		 /* job status */
};
 
 

/*
 * system status flags
 */
#define	US_S_OK		0	/* conversation succeeded */
#define	US_S_SYS	1	/* bad system */
#define	US_S_TIME	2	/* wrong time to call */
#define	US_S_LOCK	3	/* system locked */
#define US_S_DEV	4	/* no device available */
#define	US_S_DIAL	5	/* dial failed */
#define	US_S_LOGIN	6	/* login failed */
#define	US_S_HAND	7	/* handshake failed */
#define	US_S_START	8	/* startup failed */
#define	US_S_GRESS	9	/* conversation in progress */
#define	US_S_CF		10	/* conversation failed */
#define US_S_COK	11	/* call succeeded */
 
 

/*
 * request status flags
 */
#define	USR_CFAIL	01	/* copy fail */
#define	USR_LOCACC	02	/* local access to file denied */
#define	USR_RMTACC	04	/* remote access to file denied */
#define	USR_BADUUCP	010	/* bad uucp command */
#define	USR_RNOTMP	020	/* remote can't create temp file */
#define USR_RMTCP	040	/* can't copy to remote directory */
#define USR_LOCCP	0100	/* can't copy to local directory */
#define USR_LNOTMP	0200	/* local can't create temp file */
#define	USR_XUUCP	0400	/* can't execute uucp */
#define	USR_COK		01000	/* copy (partially) succeeded */
#define	USR_COMP	02000	/* copy completed, job deleted */
#define	USR_QUEUE	04000	/* job is queued */
#define	USR_KCOMP	010000	/* job is killed completely */
#define	USR_KINC	020000	/* job is killed incompletely */
 

/*
 * define USR flag
 */
extern short Usrf;	/* declaration in uucpdefs.c */
 
