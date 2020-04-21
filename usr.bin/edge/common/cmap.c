/*
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/cmap.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:45:50 $
 */

#include <stdio.h>
#include "chclass.h"
#include "tokens.h"

/* character map */
short cmap[] = {

/* 	nul		soh		stx		etx		 */
	0,		0,		0,		0,

/*	eot		enq		ack		bel 		*/
	0,		0,		0,		0,

/* 	bs		ht		nl		vt		 */
	0,		C_IGN,		C_NL,		C_IGN,

/*	np		cr		so		si  		*/
	0,		C_IGN,		0,		0,

/* 	dle		dc1		dc2		dc3		 */
	0,		0,		0,		0,

/*	dc4		nak		syn		etb		*/
	0,		0,		0,		0,

/* 	can		em		sub		esc		 */
	0,		0,		0,		0,

/*	FS		GS		RS		US		*/
	0,		0,		0,		0,

/* 	SP		!		"		#		 */
	0,		0,		0,		0,

/*	$		%		&		'		*/
	0,		0,		0,		0,		

/* 	(		)		*		+		 */
	0,		0,		0,		0,

/*	,		-		.		/		*/
	0,		0,		0,		0,

/*	0		1		2		3		 */
	C_ANYDIG,	C_ANYDIG,	C_ANYDIG,	C_ANYDIG,

/*	4		5		6		7		*/
	C_ANYDIG,	C_ANYDIG,	C_ANYDIG,	C_ANYDIG,

/*	8		9		:		;		 */
	C_ANYDIG,	C_ANYDIG,	0,		0,

/*	<		=		>		?		*/
	0,		0,		0,		0,

/*	@		A		B		C		 */
	0,		C_LET|C_XDIG,	C_LET|C_XDIG,	C_LET|C_XDIG,

/*	D		E		F		G		*/
	C_LET|C_XDIG,	C_LET|C_XDIG,	C_LET|C_XDIG,	C_LET,

/*	H		I		J		K		 */
	C_LET,		C_LET,		C_LET,		C_LET,

/*	L		M		N		O		*/
	C_LET,		C_LET,		C_LET,		C_LET,

/*	P		Q		R		S		 */
	C_LET,		C_LET,		C_LET,		C_LET,

/*	T		U		V		W		*/
	C_LET,		C_LET,		C_LET,		C_LET,

/*	X		Y		Z		[		 */
	C_LET,		C_LET,		C_LET,		0,

/*	\		]		^		_		*/
	0, 		0,		0, 		C_LET,

/*	`		a		b		c		 */
	0,		C_LET|C_XDIG,	C_LET|C_XDIG,	C_LET|C_XDIG,

/*	d		e		f		g		*/
	C_LET|C_XDIG,	C_LET|C_XDIG,	C_LET|C_XDIG,	C_LET,

/*	h		i		j		k		 */
	C_LET,		C_LET,		C_LET,		C_LET,

/*	l		m		n		o		*/
	C_LET,		C_LET,		C_LET,		C_LET,

/*	p		q		r		s		 */
	C_LET,		C_LET,		C_LET,		C_LET,

/*	t		u		v		w		*/
	C_LET,		C_LET,		C_LET,		C_LET,

/*	x		y		z		{		 */
	C_LET,		C_LET,		C_LET,		0,

/*	|		}		~		EOF		*/
	0,	  	0, 		0,	  	0
	
	};


char	*
find_end(linestr, sel_start, sel_end, start, end)
char	*linestr;
int	sel_start;
int	sel_end;
int	*start;
int	*end;
{

	char	*linep;

	for (linep = &(linestr[sel_start]); 
			(int) (linep - linestr) < sel_end; linep++) {
		if ((cmap[*linep] & (C_LET|C_ANYDIG)) == 0) {
			*start = sel_start;
			*end = sel_end;
			return(&linestr[sel_start]);
		}	
	}
	for (linep = &(linestr[sel_end]); *linep != 0; linep++) {
		if ((cmap[*linep] & (C_LET|C_ANYDIG)) == 0) {
			*start = sel_start;
			*end = (int) ((linep) - linestr);
			break;
		}
	}
	for (linep = &(linestr[sel_start]); linep > &(linestr[0]); linep--) {
		if ((cmap[*linep] & (C_LET|C_ANYDIG)) == 0) {
			*start = (sel_start - (int) (&(linestr[sel_start]) - 
				linep)) + 1;
			return(&(linestr[*start]));
		}
	}
	return(&linestr[*start]);
}

