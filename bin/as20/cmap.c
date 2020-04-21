#include "tokens.h"

/* character map */
char cmap[] = {

/* 	nul		soh		stx		etx		 */
	C_ERR,		C_ERR,		C_ERR,		C_ERR,

/*	eot		enq		ack		bel 		*/
	C_ERR,		C_ERR,		C_ERR,		C_ERR,

/* 	bs		ht		nl		vt		 */
	C_ERR,		C_IGN,		C_NL,		C_IGN,

/*	np		cr		so		si  		*/
	C_IGN,		C_IGN,		C_ERR,		C_ERR,

/* 	dle		dc1		dc2		dc3		 */
	C_ERR,		C_ERR,		C_ERR,		C_ERR,

/*	dc4		nak		syn		etb		*/
	C_ERR,		C_ERR,		C_ERR,		C_ERR,

/* 	can		em		sub		esc		 */
	C_ERR,		C_ERR,		C_ERR,		C_ERR,

/*	FS		GS		RS		US		*/
	C_ERR,		C_ERR,		C_ERR,		C_ERR,

/* 	SP		!		"		#		 */
	C_IGN,		C_ERR,		C_DQ,		C_VHASH,

/*	$		%		&		'		*/
	C_ALPHA,	C_ALPHA,	C_ERR,		C_SQ,		

/* 	(		)		*		+		 */
	C_SPECIAL,	C_SPECIAL,	C_SPECIAL,	C_SPECIAL,

/*	,		-		.		/		*/
	C_SPECIAL,	C_SPECIAL,	C_ALPHA,	C_ERR,

/*	0		1		2		3		 */
	C_ALPHA,	C_ALPHA,	C_ALPHA,	C_ALPHA,

/*	4		5		6		7		*/
	C_ALPHA,	C_ALPHA,	C_ALPHA,	C_ALPHA,

/*	8		9		:		;		 */
	C_ALPHA,	C_ALPHA,	C_SPECIAL,	C_NL,

/*	<		=		>		?		*/
	C_ERR,		C_SPECIAL,	C_ERR,		C_ERR,

/*	@		A		B		C		 */
	C_SPECIAL,	C_ALPHA,	C_ALPHA,	C_ALPHA,

/*	D		E		F		G		*/
	C_ALPHA,	C_ALPHA,	C_ALPHA,	C_ALPHA,

/*	H		I		J		K		 */
	C_ALPHA,	C_ALPHA,	C_ALPHA,	C_ALPHA,

/*	L		M		N		O		*/
	C_ALPHA,	C_ALPHA,	C_ALPHA,	C_ALPHA,

/*	P		Q		R		S		 */
	C_ALPHA,	C_ALPHA,	C_ALPHA,	C_ALPHA,

/*	T		U		V		W		*/
	C_ALPHA,	C_ALPHA,	C_ALPHA,	C_ALPHA,

/*	X		Y		Z		[		 */
	C_ALPHA,	C_ALPHA,	C_ALPHA,	C_SPECIAL,

/*	\		]		^		_		*/
	C_ERR, 		C_SPECIAL,	C_ERR, 		C_ALPHA,

/*	`		a		b		c		 */
	C_ERR,		C_ALPHA,	C_ALPHA,	C_ALPHA,

/*	d		e		f		g		*/
	C_ALPHA,	C_ALPHA,	C_ALPHA,	C_ALPHA,

/*	h		i		j		k		 */
	C_ALPHA,	C_ALPHA,	C_ALPHA,	C_ALPHA,

/*	l		m		n		o		*/
	C_ALPHA,	C_ALPHA,	C_ALPHA,	C_ALPHA,

/*	p		q		r		s		 */
	C_ALPHA,	C_ALPHA,	C_ALPHA,	C_ALPHA,

/*	t		u		v		w		*/
	C_ALPHA,	C_ALPHA,	C_ALPHA,	C_ALPHA,

/*	x		y		z		{		 */
	C_ALPHA,	C_ALPHA,	C_ALPHA,	C_SPECIAL,

/*	|		}		~		EOF		*/
	C_COMMENT,	C_SPECIAL, 	C_ERR,  	C_ERR,
/* chars 128..255 */
	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,
	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,
	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,
	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,
	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,
	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,
	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,
	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,
	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,
	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,
	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,
	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,
	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,
	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,
	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,
	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ERR,	C_ENDOFBLOCK
	
	};

/* map used to translate between the first character of a token and
   the token number.  The only exception is T_DOT, which is taken care
   of by code (scan.c).
*/
char char_to_token[] = {

/* 	nul		soh		stx		etx		 */
	0,		0,		0,		0,

/*	eot		enq		ack		bel 		*/
	0,		0,		0,		0,

/* 	bs		ht		nl		vt		 */
	0,		0,		0,		0,

/*	np		cr		so		si  		*/
	0,		0,		0,		0,

/* 	dle		dc1		dc2		dc3		 */
	0,		0,		0,		0,

/*	dc4		nak		syn		etb		*/
	0,		0,		0,		0,

/* 	can		em		sub		esc		 */
	0,		0,		0,		0,

/*	FS		GS		RS		US		*/
	0,		0,		0,		0,

/* 	SP		!		"		#		 */
	0,		0,		0,		T_IMM,

/*	$		%		&		'		*/
	0,		0,		0,		T_CHAR,

/* 	(		)		*		+		 */
	T_LP,		T_RP,		T_MUL,		T_PLUS,

/*	,		-		.		/		*/
	T_CM,		T_MINUS,	0,		0,

/*	0		1		2		3		 */
	T_NUMBER,	T_NUMBER,	T_NUMBER,	T_NUMBER,

/*	4		5		6		7		*/
	T_NUMBER,	T_NUMBER,	T_NUMBER,	T_NUMBER,

/*	8		9		:		;		 */
	T_NUMBER,	T_NUMBER,	T_COLON,	T_SM,

/*	<		=		>		?		*/
	0,		T_EQ,		0,		0,

/*	@		A		B		C		 */
	T_IND,		0,		0,		0,

/*	D		E		F		G		*/
	0,		0,		0,		0,

/*	H		I		J		K		 */
	0,		0,		0,		0,

/*	L		M		N		O		*/
	0,		0,		0,		0,

/*	P		Q		R		S		 */
	0,		0,		0,		0,

/*	T		U		V		W		*/
	0,		0,		0,		0,

/*	X		Y		Z		[		 */
	0,		0,		0,		T_LB,

/*	\		]		^		_		*/
	0,		T_RB,		0,		0,

/*	`		a		b		c		 */
	0,		0,		0,		0,

/*	d		e		f		g		*/
	0,		0,		0,		0,

/*	h		i		j		k		 */
	0,		0,		0,		0,

/*	l		m		n		o		*/
	0,		0,		0,		0,

/*	p		q		r		s		 */
	0,		0,		0,		0,

/*	t		u		v		w		*/
	0,		0,		0,		0,

/*	x		y		z		{		 */
	0,		0,		0,		T_LBRACE,

/*	|		}		~		EOF		*/
	0,		T_RBRACE,	0,		0,
/* chars 128..255 */
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0
	
	};
