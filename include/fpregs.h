/*	fpregs.h - register definitions for the sky floating point
	processor board.
*/

#define SKYB		0x1040
#define SKYBASE 	(unsigned short *)SKYB
#define SKYCOMREG	(unsigned short *)(SKYB)
#define SKYSTATREG  (short *)(SKYB+2)
#define SKYDTREG	(long *)(SKYB+4)

#define SKYFLREG	(float *)(SKYB+4)


