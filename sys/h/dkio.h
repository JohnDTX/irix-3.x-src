/*
** Structures and definitions for disk io control commands
** $Author: root $
** $State: Exp $
** $Source: /d2/3.7/src/sys/h/RCS/dkio.h,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:29:19 $
*/

/* structure for DKIOCTOP - disk op command */
struct	dkop	{
	short	dk_op;		/* operations defined below */
	daddr_t	dk_count;	/* how many of them */
};

/* operations */
#define DKRETURNINFO	0	/* Created for storager II debugging */

/* structure for DTIOCGET - disk get status command */

struct	dkget	{
	unsigned short	dk_type;	/* type of disk device */
	unsigned short	dk_status;	/* drive/controller status register */
	unsigned long 	dk_siidone;
	unsigned long 	dk_siineed;
	unsigned long 	dk_retries;
	unsigned long 	dk_fileno;
	unsigned long 	dk_blkno;
};

/* disk io control commands */
#define	DKIOCTOP	(('m'<<8)|1)
#define	DKIOCGET	(('m'<<8)|2)
