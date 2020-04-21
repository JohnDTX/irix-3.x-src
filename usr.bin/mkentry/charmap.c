
#include "lex.h"
#include "tokens.h"

/* character map */
int cmap[128] = {

/* 	nul	soh	stx	etx	eot	enq	ack	bel 	*/
	ERROR,	ERROR,	ERROR,	ERROR,	ERROR,	ERROR,	ERROR,	ERROR,
/* 	bs	ht	nl	vt	np	cr	so	si  	*/
	ERROR,	IGNORE,	NL,	IGNORE,	ERROR,	IGNORE,	ERROR,	ERROR,
/* 	dle	dc1	dc2	dc3	dc4	nak	syn	etb	*/
	ERROR,	ERROR,	ERROR,	ERROR,	ERROR,	ERROR,	ERROR,	ERROR,
/* 	can	em	sub	esc	FS	GS	RS	US	*/
	ERROR,	ERROR,	ERROR,	ERROR,	ERROR,	ERROR,	ERROR,	ERROR,
/* 	SP	!	"	#	$	%	&	'	*/
	IGNORE,	ERROR,	ERROR,	CPP,	ERROR,	ERROR,	ERROR,	ERROR,	
/* 	(	)	*	+	,	-	.	/	*/
	SP|LP,	SP|RP,	BC2,	ERROR,	SP|CM,	ERROR,	ERROR,	BC,
/*	0	1	2	3	4	5	6	7	*/
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
/*	8	9	:	;	<	=	>	?	*/
	ALPH,	ALPH,	ERROR,	SP|SM,	ERROR,	ERROR,	ERROR,	ERROR,
/*	@	A	B	C	D	E	F	G	*/
	ERROR,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
/*	H	I	J	K	L	M	N	O	*/
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
/*	P	Q	R	S	T	U	V	W	*/
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
/*	X	Y	Z	[	\	]	^	_	*/
	ALPH,	ALPH,	ALPH,	SP|LBRACKET,ERROR, SP|RBRACKET,ERROR, ALPH,
/*	`	a	b	c	d	e	f	g	*/
	ERROR,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
/*	h	i	j	k	l	m	n	o	*/
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
/*	p	q	r	s	t	u	v	w	*/
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
/*	x	y	z	{	|	}	~	EOF	*/
	ALPH,	ALPH,	ALPH,	SP|LBRACE, ERROR,  SP|RBRACE, ERROR,  ENDFILE
	};


