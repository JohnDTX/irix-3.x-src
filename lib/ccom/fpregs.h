/*	fpregs.h - register definitions for the sky floating point
	processor board.
*/

#define SKYB		0x1040
#define SKYBASE 	(unsigned short *)SKYB
#define SKYCOMREG	(unsigned short *)(SKYB)
#define SKYSTATREG  (short *)(SKYB+2)
#define SKYDTREG	(long *)(SKYB+4)

#define SKYFLREG	(float *)(SKYB+4)

/*	JUNIPER FPA register definitions.
*/

#define FPA_BASE	0x8000

#define FPA_CCR		FPA_BASE+0x700
