
/*
 * This uib is used to pre-initialize a drive before we read the label.
 * THESE VALUES ARE FOR THE VERTEX, ST/506.
 */
#define	UIB_VERTEX	0x06		/* MFM, ST/506 */
#ifdef	notdef
#define	UIB_OPTIONS	(CACHEENABLE | ZEROLATENCY | UPDATEIOPB | CBREQENABLE)
#else
#define	UIB_OPTIONS	(CACHEENABLE | ZEROLATENCY)
#endif
struct	uib default_uib = {
	50,				/* sectors per track */
	50,				/* heads per cylinder */
	MB(BLKSIZE),			/* bytes per sector */
	LB(BLKSIZE),
	17,				/* bytes in gap 2 */
	17,				/* bytes in gap 1 */
	10,				/* retry count before ecc is used */
	1,				/* interleave (NOISE) */
	1,				/* reseek on */
	1,				/* ecc enabled */
	1,				/* increment by head */
	0,				/* don't move bad data */
	0,				/* don't interrupt on status change */
	0,				/* NOT USED */
	0,				/* NOT USED  */
	2,				/* spiral skew factor (NOISE) */
	UIB_OPTIONS,			/* operation options byte */
	0,				/* motor off and head unload time */
	24,				/* bytes in gap 3 */
	UIB_VERTEX,			/* drive descriptor byte */
	1,				/* step pulse width */
	0x20,				/* motor and step control */
	MB(1),				/* step pulse interval */
	LB(1),
	5,				/* track to track seek time */
	0,				/* head load or settling time */
	MB(2000),			/* number of cylinders */
	LB(2000),
	MB(0xFFFF),			/* write precomp starting cyl */
	LB(0xFFFF),
	MB(0xFFFF),			/* reduce write current starting cyl */
	LB(0xFFFF),
};
