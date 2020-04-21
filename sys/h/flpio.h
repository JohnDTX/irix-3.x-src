/*
** floppy disk drive structure for formatting.
*/

/* structure for FLP_IOCTOP - mag tape op command */
struct	flpop	{
	short	flp_op;		/* operations defined below */
	daddr_t	flp_count;	/* how many of them */
};

/* operations */
#define FLP_FORMAT	0	/* Format the Diskette */
#define FLP_HELP	8	/* Help command */

/* structure for status command */

/*
** This structure will surely have to change
*/
struct	flpget	{
	short	flp_type;
	short	flp_hard_error;
	short	flp_soft_error;
	short	flp_file_mark;
	short	flp_retries;
	daddr_t	flp_fileno;
	daddr_t	flp_blkno;
};

/*
** Flags
*/
#define WR_PROT		0x04
#define HARD_ERROR	0x08
#define SOFT_ERROR	0x10

/*
** Constants for mt_type byte
*/
#define	FLP_ISQUME		0x01	/* Qume 592 */
#define	FLP_IST101		0x02	/* Tandon 101-4 */

#define	FLP_IOCTOP	(('m'<<8)|1)
#define	FLP_IOCGET	(('m'<<8)|2)
