/*
 * following defines are bits in the ready vector - the device status mask
 */
#define 	FROMHOST	(1<<0)
#define 	TOHOST		(1<<1)
#define 	KEYIN		(1<<2)
#define 	KEYPANEL	(1<<3)
#define 	FROMENET	(1<<4)
#define 	TOENET		(1<<5)
#define		TOSCREEN	(1<<6)	/* pseudo-device SCREEN */
#define 	ALARM		(1<<7)

#define 	CLOCK1MS	(1<<8)
#define 	CLOCK10MS	(1<<9)
#define 	CLOCK100MS	(1<<10)
#define 	CLOCK1S		(1<<11)

/* #define 	ALARM		(1<<12) */
#define		NOTSTOPPED	(1<<13)

#define 	CLOCKBITS	(CLOCK1MS|CLOCK10MS|CLOCK100MS|CLOCK1S)

#define 	READTIMEOUT	(-2)


/* alternate Port A attachments */
#define	ASCII	1		/* options for global config variable	*/
#define MICSW	2		/* 'keyboard'				*/
