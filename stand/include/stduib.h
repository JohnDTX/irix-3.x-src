/*
* $Source: /d2/3.7/src/stand/include/RCS/stduib.h,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:13:53 $
*/

/*
 * This uib is used to pre-initialize a drive before we read the label.
 * THESE VALUES ARE FOR THE VERTEX, ST/506.
 */
#define	UIB_VERTEX	0x06		/* MFM, ST/506 */
#define UIB_HITACHI	0x2F		/* ESDI Hard Sectored */
#define UIB_MAXTOR	0x27		/* ESDI Soft (Address Mark) */
#define	UIB_OPTIONS (CACHEENABLE | ZEROLATENCY | CBREQENABLE)
#define	UIB_XOPTIONS (ZEROLATENCY | CBREQENABLE)
struct	uib sii_st506_uib = {
	32,				/* sectors per track */
	1,				/* heads per cylinder */
	MB(BLKSIZE),			/* bytes per sector */
	LB(BLKSIZE),
	0x0b,				/* bytes in gap 2 */
	0x09,				/* bytes in gap 1 */
	2,				/* retry count before ecc is used */
	1,				/* interleave */
	1,				/* reseek on */
	1,				/* ecc enabled */
	1,				/* increment by head */
	0,				/* don't move bad data */
	0,				/* don't interrupt on status change */
	0,				/* reserved */
	0,				/* group size */
	3,				/* spiral skew factor */
	UIB_OPTIONS,			/* operation options byte */
	0,				/* motor off and head unload time */
	0x20,				/* bytes in gap 3 */
	UIB_VERTEX,			/* drive descriptor byte */
	1,				/* step pulse width */
	0x20,				/* motor and step control */
	MB(1),				/* step pulse interval */
	LB(1),
	5,				/* track to track seek time */
	0,				/* head load or settling time */
	MB(1),				/* number of cylinders */
	LB(1),
	MB(0xFFFF),			/* write precomp starting cyl */
	LB(0xFFFF),
	MB(0xFFFF),			/* reduce write current starting cyl */
	LB(0xFFFF),
};

struct	uib sii_hesdi_uib = {
	32,				/* sectors per track */
	1,				/* heads per cylinder */
	MB(BLKSIZE),			/* bytes per sector */
	LB(BLKSIZE),
	0x0b,				/* bytes in gap 2 */
	0x09,				/* bytes in gap 1 */
	2,				/* retry count before ecc is used */
	1,				/* interleave */
	1,				/* reseek on */
	1,				/* ecc enabled */
	1,				/* increment by head */
	0,				/* don't move bad data */
	0,				/* don't interrupt on status change */
	0,				/* reserved */
	0,				/* group size */
	3,				/* spiral skew factor */
	UIB_XOPTIONS,			/* operation options byte */
	0,				/* motor off and head unload time */
	0x20,				/* bytes in gap 3 */
	UIB_HITACHI,			/* drive descriptor byte */
	1,				/* step pulse width */
	0x20,				/* motor and step control */
	MB(1),				/* step pulse interval */
	LB(1),
	5,				/* track to track seek time */
	0,				/* head load or settling time */
	MB(1),				/* number of cylinders */
	LB(1),
	MB(0xFFFF),			/* write precomp starting cyl */
	LB(0xFFFF),
	MB(0xFFFF),			/* reduce write current starting cyl */
	LB(0xFFFF),
};

struct	uib sii_sesdi_uib = {
	32,				/* sectors per track */
	1,				/* heads per cylinder */
	MB(BLKSIZE),			/* bytes per sector */
	LB(BLKSIZE),
	0x0b,				/* bytes in gap 2 */
	0x09,				/* bytes in gap 1 */
	2,				/* retry count before ecc is used */
	1,				/* interleave */
	1,				/* reseek on */
	1,				/* ecc enabled */
	1,				/* increment by head */
	0,				/* don't move bad data */
	0,				/* don't interrupt on status change */
	0,				/* reserved */
	0,				/* group size */
	3,				/* spiral skew factor */
	UIB_XOPTIONS,			/* operation options byte */
	0,				/* motor off and head unload time */
	0x20,				/* bytes in gap 3 */
	UIB_MAXTOR,			/* drive descriptor byte */
	1,				/* step pulse width */
	0x20,				/* motor and step control */
	MB(1),				/* step pulse interval */
	LB(1),
	5,				/* track to track seek time */
	0,				/* head load or settling time */
	MB(1),				/* number of cylinders */
	LB(1),
	MB(0xFFFF),			/* write precomp starting cyl */
	LB(0xFFFF),
	MB(0xFFFF),			/* reduce write current starting cyl */
	LB(0xFFFF),
};

struct	uib sii_flp_uib = {
	8,				/* sectors per track */
	2,				/* heads per cylinder */
	MB(BLKSIZE),			/* bytes per sector */
	LB(BLKSIZE),
	0x11,				/* bytes in gap 2 */
	0x11,				/* bytes in gap 1 */
	2,				/* retry count before ecc is used */
	1,				/* interleave */
	0,				/* reseek on */
	2,				/* ecc enabled */
	1,				/* increment by head */
	0,				/* don't move bad data */
	0,				/* don't interrupt on status change */
	0,				/* reserved */
	0,				/* group size */
	0,				/* spiral skew factor */
	2,				/* operation options byte */
	0x11,				/* motor off and head unload time */
	0x32,				/* bytes in gap 3 */
	0x44,				/* drive descriptor byte */
	3,				/* step pulse width */
	0x6,				/* motor and step control */
	MB(200),			/* step pulse interval */
	LB(200),
	15,				/* track to track seek time */
	0xf,				/* head load or settling time */
	MB(80),				/* number of cylinders */
	LB(80),
	MB(0x0),			/* write precomp starting cyl */
	LB(0x0),
	MB(0xFFFF),			/* reduce write current starting cyl */
	LB(0xFFFF),
};
