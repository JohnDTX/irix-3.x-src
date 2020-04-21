/*	@(#)Param.h	1.1	*/

#include	<errno.h>
#include	<stdio.h>
#include	<ctype.h>

#define	reg	register
#define	uns	unsigned

typedef	unsigned long	int	ulong;
typedef	char	*String;
typedef	unsigned int	bit_fld;	/* may be machine dependant */
typedef	int	(*Routine)();		/* Indirect subroutining */
typedef	char	**queue;		/* simply an address of an address */
typedef	char	*qelem;			/* needs to be of pointer to */

#define	FALSE	0			/* The value of falsehood */
#define	TRUE	1			/* The value of truth (in a bit) */
#define	QUIT	-1			/* Has nothing to do with SIGQUIT */
#define	NAMSIZ	16			/* Size of NS and Symtab Names */

extern	String	falloc();
extern	String	frealloc();
extern	String	str_cat();
extern	String	str_scat();
extern	String	strs_cat();
extern	String	strn_cat();
extern	String	str_cpy();
extern	String	getf();
extern	String	gail();
extern	String	dirnm();
extern	String	filnm();
extern	String	rdcmd();
extern	String	cvtsccs();
extern	String	stou();
extern	String	*word_vec();
extern	String	hdp();
extern	int	Rdrc;
extern	int	wc;
extern	int	errno;

/* From the C library: */
extern	String	ctime();
extern	String	getlogin();
extern	String	getenv();
extern	String	strcat();
extern	String	strchr();
extern	String	strrchr();
extern	String	strcpy();
extern	String	strtok();
extern	String	strncat();
extern	String	mktemp();
extern	FILE	*popen();
extern	long	time();

/*
**	allocator macros:
*/

#define	get_one(type)		((type *) falloc((uns) sizeof (type),(uns) 1))
#define	get_some(type,num)	((type *) falloc((uns) sizeof (type),(uns) num))

/*
**	queue macros:
*/

#define	q_a(q, e)	q_add((queue *) q, (qelem) e)
#define	q_r(q, e)	q_rem((queue *) q, (qelem) e)
#define	q_rs(q, e)	q_rems((queue *) q, (qelem) e)

/*
**	string macros:
*/

#define	str_equ(s0,s1)	((s0 && s1) && ((s0 == s1) || strcmp(s0, s1) == 0))
#define	str_len(s)	(1+strlen(s))
#define	zapnl(s)	s[strlen(s)-1] = '\0'
#define	czapnl(s)	if(s[strlen(s)-1] == '\n') s[strlen(s)-1] = '\0'
#define	lastch(s)	s[strlen(s)-1]

/*
**	print macros
*/

#define	Fp(x)	fprintf(stderr, x);

/*
**	Group IDs:
*/

#define	GR_VIS	9999
#define	GR_SCCS	199
#define	GR_PAG	1500
