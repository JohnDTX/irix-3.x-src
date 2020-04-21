/*
 *	Kurt Akeley			13 May 1983
 *
 *	Convenient definitions for planecodes and groups of planecodes.
 */


#ifndef BPC_CODES
#define BPC_CODES

#define A0		0x00000001	/* Colorcode and wecode bit	*/
#define A1		0x00000002	/*   assignments		*/
#define A2		0x00000004
#define A3		0x00000008
#define A4		0x00000010
#define A5		0x00000020
#define A6		0x00000040
#define A7		0x00000080

#define B0		0x00000100
#define B1		0x00000200
#define B2		0x00000400
#define B3		0x00000800
#define B4		0x00001000
#define B5		0x00002000
#define B6		0x00004000
#define B7		0x00008000

#define C0		0x00010000
#define C1		0x00020000
#define C2		0x00040000
#define C3		0x00080000
#define C4		0x00100000
#define C5		0x00200000
#define C6		0x00400000
#define C7		0x00800000

#define D0		0x01000000
#define D1		0x02000000
#define D2		0x04000000
#define D3		0x08000000
#define D4		0x10000000
#define D5		0x20000000
#define D6		0x40000000
#define D7		0x80000000

#define XA0		C0
#define XA1		C1
#define XA2		C2
#define XA3		C3
#define XA4		C4
#define XA5		C5
#define XA6		C6
#define XA7		C7

#define XB0		D0
#define XB1		D1
#define XB2		D2
#define XB3		D3
#define XB4		D4
#define XB5		D5
#define XB6		D6
#define XB7		D7

#define XC0		A0
#define XC1		A1
#define XC2		A2
#define XC3		A3
#define XC4		A4
#define XC5		A5
#define XC6		A6
#define XC7		A7

#define XD0		B0
#define XD1		B1
#define XD2		B2
#define XD3		B3
#define XD4		B4
#define XD5		B5
#define XD6		B6
#define XD7		B7

#define BOARD0CODE	(A0|A1|B0|B1)
#define BOARD1CODE	(A2|A3|B2|B3)
#define BOARD2CODE	(A4|A5|B4|B5)
#define BOARD3CODE	(A6|A7|B6|B7)
#define BOARD4CODE	(C0|C1|D0|D1)
#define BOARD5CODE	(C2|C3|D2|D3)

#define ACODE		(A0|A1|A2|A3|A4|A5|A6|A7)
#define BCODE		(B0|B1|B2|B3|B4|B5|B6|B7)
#define CCODE		(C0|C1|C2|C3|C4|C5|C6|C7)
#define DCODE		(D0|D1|D2|D3|D4|D5|D6|D7)

#endif BPC_CODES
