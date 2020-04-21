/*
 *
 *	charclasses.h - 
 *
 *		classes of characters for the scanner for the SGI
 *		C compiler.
 *
 */


#define C_TYPE  0xff00

#define C_SIMPLE 0
#define C_IGN 	(0|C_SIMPLE)
#define C_LET	(1|C_SIMPLE)
#define C_XDIG  (2|C_SIMPLE)
#define C_DDIG	(4|C_SIMPLE)
#define C_ODIG	(8|C_SIMPLE)
#define C_ANYDIG (C_XDIG|C_DDIG|C_ODIG)

#define C_ISATOKEN	(0x100)
#define C_LP	(1|C_ISATOKEN)
#define C_RP	(2|C_ISATOKEN)
#define C_SM	(3|C_ISATOKEN)
#define C_COLON	(4|C_ISATOKEN)
#define C_CM	(5|C_ISATOKEN)
/*#define C_STMEM	(6|C_ISATOKEN) */
#define C_QUEST	(7|C_ISATOKEN)
#define C_LBRACKET	(8|C_ISATOKEN)
#define C_RBRACKET	(9|C_ISATOKEN)
#define C_LBRACE	(0xa|C_ISATOKEN)
#define C_RBRACE	(0xb|C_ISATOKEN)

#define C_ISOP	(0x200)
/* several things to consider:
	can the resultant operator be an assign operator? (a bit to try again)
	(this is true with the SHIFTs)

	all  can be assign ops

	besides assign ops are <<,>>,&&,||,++,--,->

	map to selves until failure, then map to real operators.
*/
#define C_ASG (0x10)
#define C_DOUBLE (0x20)
#define C_BASEOP (0xf)
#define C_NOPS (0x41	/* one for STREF */)
#define C_NOT	(C_ISOP|0x1)
#define C_MUL	(C_ISOP|0x2)
#define C_AND	(C_ISOP|0x3)
#define C_MOD	(C_ISOP|0x4)
/* PLUS PLUS -> INCR */
#define C_PLUS	(C_ISOP|0x5)
/* MINUS MINUS -> DECR */
#define C_MINUS	(C_ISOP|0x6)
#define C_DIV	(C_ISOP|0x7)
/* LT LT -> LSHIFT */
#define C_LT	(C_ISOP|0x8)
/* GT GT -> RSHIFT */
#define C_GT	(C_ISOP|0x9)
/* EQ EQ -> EQEQ */
#define C_EQ	(C_ISOP|0xa)
#define C_XOR	(C_ISOP|0xb)
/* OR OR -> OROR */
#define C_OR	(C_ISOP|0xC)
#define C_COMPL	(0xd|C_ISOP)
#define C_STMEM (0xe|C_ISOP)
#define C_STREF (0x41|C_ISOP)

#define C_LSHIFT (C_DOUBLE|C_LT)
#define C_RSHIFT (C_DOUBLE|C_GT)
#define C_DECR	(C_DOUBLE|C_MINUS)
#define C_ANDAND (C_DOUBLE|C_AND)
#define C_OROR	(C_DOUBLE|C_OR)
#define C_INCR	(C_DOUBLE|C_PLUS)
#define C_ELLIPSES (C_DOUBLE|C_STMEM)

#define C_SPECIAL (0x300)
#define C_ENDFILE (0x1|C_SPECIAL)
#define C_ERR	(0x2|C_SPECIAL)
#define C_ESC	(0x4|C_SPECIAL)
#define C_DQ	(0x5|C_SPECIAL)
#define C_SQ	(0x6|C_SPECIAL)
#define C_NL	(0|C_SPECIAL)

/* comment characters */
#define C_BC	C_DIV
#define C_EC	C_DIV
#define C_BC2	C_MUL


