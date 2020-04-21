/*	fpregs.h - register definitions for the sky floating point
	processor board.
*/

#define SKYB		(MBIO_VBASE + 0x0040)
#define SKYBASE 	(unsigned short *)(SKYB)
#define SKYCOMREG	(unsigned short *)(SKYB)
#define SKYSTATREG  	(short *)(SKYB+2)
#define SKYDTREG	(long *)(SKYB+4)

#define SKYFLREG	(float *)(SKYB+4)

#define SKYDT1REG	(short *)(SKYB+4)
#define SKYDT2REG	(short *)(SKYB+6)

/* bits in the status and control register */
#define SKYIODIR	0x2000
#define SKYIORDY	0x8000
#define SKYIDLE		0x4000

/* status register commands */
#define SINGLE_STEP	0x60
#define RESET		0x80
#define RUN		0x40
