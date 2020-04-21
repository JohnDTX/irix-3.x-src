/*	sa.h 1.6 of 6/28/82	*/
/*	sa.h contains struct sa and defines variable used 
		in sadc.c and sar.c.
	The nlist setup table in sadc.c is arranged as follows:
	For VAX and PDP11 machines,
		- device status symbol names for controllers with 8 drives ,
			headed by _hpstat. 
			(not include the _gdstat)
		- device status symbol names for controllers with 4 drives,
			headed by _rlstat. 
		- device status symbol names for controllers with 1 drive,
			headed by _tsstat. 
		- general disk driver status system name.
		- symbol name for _sysinfo.
		- symbol names for system tables: inode, file,
			text and process.
		- symbol name for _var.
		- symbol name for _rl_cnt.
		- symbol name for _gd_cnt.
		- symbol name for system error table:

	For 3b20S system,
		- symbol name of dskinfo table
		- symbol name of UN52 tape drive.
		- symbol name for _sysinfo.
		- symbol names for system tables: inode, file,
			text and process.
		- symbol name for _var.
		- symbol name for dsk_cnt.
		- symbol name for un52_cnt.
		- symbol name for system error table:
	For IBM 370 system,
		- symbol name for sysinfo.
		- symbol names for system tables: inode, file,
				text and process.
		- symbol name for var.
		- symbol name for system error table:
			Note that this is always the last entry in setup
			table, since the number of entries of setup table
			is used in sadc.c.
*/
 
 
 
#include <nlist.h>
static char Sccssa[]="@(#)sa.h	1.6";
/*	The following variables define the positions of symbol
	table names in setup table:
*/
 
#if vax || pdp11 || defined(m68000)
#define HPS	0
#define HMS	1
#define HSS	2
#define RFS	3
#define RKS	4
#define RPS	5
#define RLS	6
#define GTS	7
#define HTS	8
#define TMS	9
#define TSS	10
#define GDS	11
#define SINFO	12
#define INO	13
#define FLE	14
#define TXT	15
#define PRO	16
#define V	17
#define RLCNT	18
#define GDCNT	19
#define SERR	20
#endif
 
#ifdef u3b
#define DSKINFO 0
#define UN52	1
#define SINFO	2
#define INO	3
#define FLE	4
#define TXT	5
#define PRO	6
#define V	7
#define DSKCNT  8
#define UN52CNT 9
#define SERR	10
#endif
 
#ifdef u370
#define	SINFO	0
#define	INO	1
#define	FLE	2
#define	TXT	3
#define	PRO	4
#define	V	5
#define	SERR	6
#endif

#if vax || pdp11 || defined(m68000)
struct nlist setup[] = {
	{"_hpstat"},
	{"_hmstat"},
	{"_hsstat"},
	{"_rfstat"},
	{"_rkstat"},
	{"_rpstat"},
	{"_rlstat"},
	{"_gtstat"},
	{"_htstat"},
	{"_tmstat"},
	{"_tsstat"},
	{"_gdstat"},
	{"_sysinfo"},
	{"_inode"},
	{"_file"},
	{"_text"},
	{"_proc"},
	{"_v"},
	{"_rl_cnt"},
	{"_gd_cnt"},
	{"_syserr"},
	{0},
};
#endif

#ifdef u3b
struct nlist setup[] = {
	{"dskinfo"},
	{"un52_un52"},
	{"sysinfo"},
	{"inode"},
	{"file"},
	{"text"},
	{"proc"},
	{"v"},
	{"dsk_cnt"},
	{"un52_cnt"},
	{"syserr"},
	{0},
};
#endif

#ifdef u370
struct nlist setup[] = {
	{"sysinfo"},
	{"inode"},
	{"file"},
	{"text"},
	{"proc"},
	{"v"},
	{"syserr"},
	{0},
};
#endif

#if vax || pdp11 || defined(m68000)
#define NCTRA	6  /* number of 8-drive disk controllers  */
#define NCTRB	4  /* number of 4-drive disk and tape controllers  */
#define NCTRC	1  /* number of general disk controllers  */
		   /* and number of ts11 tape controller	*/
#define NDRA	8  /* number of data units for a 8-drive disk controller  */
#define NDRB	4  /* number of data units for a 4-drive disk controller  */
#define NDRC	32 /* number of data units for a general disk controller  */
#define NDRD	1  /* number of data units for a ts11 tape controller  */
/*	this is for gd device	*/
/*	NDEVS defines number of total data units 
*/
#define NDEVS NCTRA *NDRA +NCTRB * NDRB + NCTRC * NDRC + NCTRC * NDRD
/*	iotbsz, devnm tables define the number of drives,
	controller name  of devices
	hpstat, hmstat, hsstat, rfstat, rkstat, rpstat, rlstat,
	gtstst, htstat, tmstat, tsstat,
	gdstat.
	Note that the ordering of them is consistent with the ordering 
	of device status symbol names in setup table.
*/
 
int iotbsz[SINFO] = {
	NDRA,NDRA,NDRA,NDRA,NDRA,NDRA,NDRB,NDRB,NDRB,NDRB,NDRD,NDRC
};
 
char devnm[SINFO][6] ={
	"rp06-",
	"rm05-",
	"rs04-",
	"rf11-",
	"rk05-",
	"rp03-",
	"rl02-",
	"tape-",
	"tm03-",
	"tm11-",
	"ts11-",
	"dsk-"
};
#endif

#ifdef u3b
#define NDEVS 100
/*	iotbsz, devnm tables define the initial value of number of drives
	and name of devices.
*/
int iotbsz[SINFO] = {
	0,0
};
char devnm[SINFO][6] ={
	"dsk-",
	"tape-"
};
#endif

#ifdef u370
#define NDRUM 1
#define NDISK 22
#define NDEV NDRUM + NDISK

/*	The structure procinfo contains data about process table */ 
struct	procinfo { 
	int	sz;	/* number of processes */ 
	int	run;	/* number that are running	*/ 
	int	wtsem;	/* number that are not process 	*/  
			/* group leaders that are 	*/ 
			/* waiting on semaphores	*/ 
	time_t  wtsemtm;/*acc wait time	*/
	int	wtio;	/* number that are not process 	*/ 
			/* group leaders that are 	*/ 
			/* waiting on i/o		*/ 
	time_t wtiotm;	/*acc wait time		*/
	}; 
/* The following structure has information */ 
/*	about drums and disks.			*/ 
struct	iodev { 
	long	io_sread; 
	long	io_pread; 
	long	io_swrite; 
	long	io_pwrite; 
	long	io_total;
}; 
#endif 
 
/*	structure sa defines the data structure of system activity data file
*/
 
struct sa {
	struct	sysinfo si;	/* defined in /usr/include/sys/sysinfo.h  */
	int	szinode;	/* current size of inode table  */
	int	szfile;		/* current size of file table  */
	int	sztext;		/* current size of text table  */
	int	szproc;		/* current size of proc table  */
	int	mszinode;	/* maximum size of inode table  */
	int	mszfile;	/* maximum size of file table  */
	int	msztext;	/* maximum size of text table  */
	int	mszproc;	/* maximum size of proc table  */
	long	inodeovf;	/* cumulative overflows of inode table
					since boot  */
	long	fileovf;	/* cumulative overflows of file table
					since boot  */
	long	textovf;	/* cumulative overflows of text table
					since boot  */
	long	procovf;	/* cumulative overflows of proc table
					since boot  */
	time_t	ts;		/* time stamp  */
#ifdef u370
	time_t	elpstm;		/* elapsed time - normally  */
 			/* gotten from sysinfo structure */	/*370*/
	time_t	curtm;		/* time since 1/1/1970 - for Q option */
	double	tmelps;		/* elapsed time in micro secs  */
				/* obtained from tss table	*/
	short	nap;		/* the number of processors	*/
	int	lines;		/* the number of lines - for q option */
/* The following represent times obtained from the tss 	*/ 
/* system statistics table. They are in micro secs.	*/ 
	double	vmtm;		/* cumulative vm time since ipl	*/ 
	double	usrtm;		/* cumulative user (vm) time since ipl	*/
	double	usuptm;		/* cumulative unix superviser (vm) time 
				since ipl */
	double	idletm;		/* cumulative idle time since ipl	*/ 
	double	ccv;		/* current clock value	*/ 
/* The following are from the tss table and give info	*/ 
/* about scheduling and dispatching			*/ 
	int	intsched;	/*no. of times the internal scheduler */ 
				/* was entered since ipl	*/ 
	int	tsend;		/* no. of time slice ends since ipl	*/ 
	int	mkdisp;		/* no. of tasks which entered 	*/ 
				/* dispatchable list since ipl	*/ 
	struct procinfo pi;	/* process table info	*/ 
	struct iodev io[NDEV];	/*drum - disk info	*/ 
#endif

#ifndef u370
	long	devio[NDEVS][4]; /* device unit information  */
#endif 

#define	IO_OPS	0  /* number of I /O requests since boot  */
#define	IO_BCNT	1  /* number of blocks transferred since boot */
#define	IO_ACT	2  /* cumulative time in ticks when drive is active  */
#define	IO_RESP	3  /* cumulative I/O response time in ticks since boot  */
};
extern struct sa sa;
