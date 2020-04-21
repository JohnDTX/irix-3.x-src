/*	@(#)Bits.h	1.1	*/
struct	_Bit
{
	String	b_name;			/* name of bit */
	char	b_indx;			/* offset in array */
	char	b_bitno;		/* offset in int */
};

typedef	struct	_Bit	Bit;		/* new type: Bit */

/* Externals */
extern	Bit	ob_bits[];

#define	ob_bic(o, b)	bit_clr(&o->o_flags[0],	b, &ob_bits[0])
#define	ob_bis(o, b)	bit_set(&o->o_flags[0],	b, &ob_bits[0])
#define	ob_bit(o, b)	bit_tst(&o->o_flags[0],	b, &ob_bits[0])
