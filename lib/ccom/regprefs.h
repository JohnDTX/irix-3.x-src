/* regprefs.h - GB SGI

	float/double s/w routines attempt to have their arguments passed
	in registers.  This is a set of register defines (necessary because
	the default defines for register symbols wasn't unique when OR'd).

*/

/* SGI_NODE is used to mark a node as being a special sgi type */
#define SGI_NODE   0x800000

/* various qualifiers on the operator nodes - types of operators */
/* SGIARG is set if the node is to eventually become an argument */
#define SGIARG  0x10000
#define COMMUTEOP 0x40000
#define DOUBLEOP  0x80000
#define INDIRECTOP 0x100000
#define UNARYOP	0x200000

/* PUSHED is set if the arg is done */
#define PUSHED  0x400000


/* HWOP is set if the node can be handled by one of the fpas.  In this
	case, the hw_opcode field of the node is set to the opcode needed. */
#define HWNODE 0x1000000
#define HWOP 0x2000000
/* this op can be ignored for the JUNIPER_FPA */
#define HWPOLL 0x4000000
#define HWCCRESULT 0x8000000


#define WRITE_HWOP	0x10000000
#define HWSTACKRESULT 0x20000000

#define SAVE_REGS 0xF00

#define SGIARGBITS SGI_NODE|COMMUTEOP|DOUBLEOP|INDIRECTOP|UNARYOP|SGIARG\
					|HWOP|HWPOLL|HWNODE


#define SAVE_D0 0x100
#define SAVE_D1 0x200
#define SAVE_A0 0x400
#define NEED_CC 0x800
