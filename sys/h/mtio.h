/*
 * Structures and definitions for mag tape io control commands
*/

/* structure for MTIOCTOP - mag tape op command */
struct	mtop	{
	short	mt_op;		/* operations defined below */
	daddr_t	mt_count;	/* how many of them */
};

/* operations */
#define MTWEOF	0	/* write an end-of-file record */
#define MTFSF	1	/* forward space file */
#define MTBSF	2	/* backward space file */
#define MTFSR	3	/* forward space record */
#define MTBSR	4	/* backward space record */
#define MTREW	5	/* rewind */
#define MTOFFL	6	/* offline - not used */
#define MTNOP	7	/* no operation, sets status only */
#define MTERASE	8	/* Erase function */
#define MTRET	9	/* Retention function */
#define MTBLKSIZE 10	/* Return Default Block Size from ioctl */
#define MTRESET 11	/* Reset the controller */

/* structure for MTIOCGET - mag tape get status command */

struct	mtget	{
	short	mt_type;	/* type of magtape device */
	short	mt_hard_error0;	/* drive/controller status register */
	short	mt_hard_error1;	/* hard error register */
	short	mt_soft_error0;	/* soft error register */
	short	mt_at_bot;
	int	mt_resid;	/* residual count (not done in prev I/O) */
	daddr_t	mt_status;
	daddr_t	mt_fileno;	/* file number of current position */
	daddr_t	mt_blkno;	/* block number of current position */
};

/*
 * flags for the drive status
 */
#define AT_BOT		0x01
#define NO_TAPE		0x02
#define WR_PROT		0x04
#define HARD_ERROR	0x08
#define SOFT_ERROR	0x10
#define NOT_ONLINE	0x20

/*
 * Constants for mt_type byte
 */
#define	MT_ISDSD	0x01		/* DSD and 1/4" tape drives */
#define	MT_ISTMT	0x02		/* Ciprico TM-1000 and 1/2" drives */
#define	MT_ISSTT	0x03		/* Storager and 1/4" tape drives */

/* mag tape io control commands */
#define	MTIOCTOP	(('m'<<8)|1)
#define	MTIOCGET	(('m'<<8)|2)
