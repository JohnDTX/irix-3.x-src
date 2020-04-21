/*	@(#)Object.h	1.2	*/
#include	"Objdefs.h"
#include	"Acttab.h"
#include	"Bits.h"

#define	DOBJECT(xrel)	"xrel",
#define	OBJDEFS		char *Obdefs[] = {
#define	ENDOB		, NULL };
#define	DUMDEF		char *Obdefs[] = { NULL };
#define	DUMDEF2	Acttab Oftab[] = { NULL };
#define	OBJECT		struct _Objent *
#define	OBJECTQ		struct _Objent **
#define	OBJTYPE		struct _Objtab *
#define	NOOBITS		1
#define	SFBUFSZ		256
#define	ODELIMC		'\t'		/* Delimiter in save file */
#define	LOCKFS		".lck"		/* Lockfile suffix */

/*
**	Structure for generic object types
*/

struct	_Objtab	{
	char		*o_type;
	char		*o_sfile;
	struct	_Objent	**o_q;
	struct	_Objtmp	**o_temp;
};

typedef struct _Objtab	Objtab;

/*
**	Per-type template
*/

struct	_Objtmp	{
	char		o_ftype;
	char		o_iomode;
	int		o_bits[NOOBITS];
	char		*o_fname;
	char		*o_pre;
	char		**o_post;
	char		*o_ostr;
	char		*o_istr;
	char		*o_help;
	char		*o_verify;
	char		*o_objt;	/* For ftype OBJ or OBJQ */
};

typedef struct _Objtmp	Objtmp;

#define	FT_NULL	0
#define	FT_STR	1
#define	FT_STRQ	2
#define	FT_BOOL	3
#define	FT_OBJ	4
#define	FT_OBJQ	5

/*
**	Structure for specific objects
*/

struct	_Objent	{
	char		*o_name;
	union  _Objval	**o_vq;
	struct _Objtab *o_otp;		/* Back ptr for optimizing Obj refs */
};					/* in Objval below. */

typedef struct _Objent	Objent;

union	_Objval	{
	char		*v_str;
	char		**v_strq;
	int		v_int;
	struct	_Objent	*v_obj;
	struct	_Objent	**v_objq;
};

typedef union _Objval	Objval;

/*
**	Externs:
*/

extern	Objtab	**Otab;
extern	char	*Obdefs[];
extern	Acttab	Oftab[];
extern	Bit	ob_bits[];
extern	char	*Ftypes[];
extern	char	*Fattrs[];
extern	char	*Oattrs[];
extern	char	*Ohelp;
extern	char	Sfbuf[];
extern	char	*_spar();
extern	char	*oname();
extern	Objtab	**getobj();
extern	OBJTYPE	otlook();
extern	OBJTYPE	otcreat();
extern	OBJECT	otcycle();
extern	OBJECT	olook();
extern	OBJECT	ocreat();
extern	OBJECTQ	otqlook();

/*
**	Optional field attributes
*/

#define	FA_IN		0
#define	FA_OUT		1
#define	FA_IO		2
#define	FA_IO2		3
#define	FA_IO3		4
#define	FA_VERIFY	5
#define	FA_PRE		6
#define	FA_POST		7
#define	FA_REQ		8
#define	FA_HELP		9
#define	FA_MULTI	10
#define	FA_NAMEF	11

/*
**	Optional object atributes
*/

#define	OA_SFILE	0

/*
**	otcycle() modes:
*/

#define	CYSTART	0
#define	CYREAD	1
#define	CYEND	2

/*
**	Bit macros
*/

#define	is_multi(t)	(bit_tst(&t->o_bits[0], "Multi", &ob_bits[0]))
#define	is_req(t)	(bit_tst(&t->o_bits[0], "Req", &ob_bits[0]))
#define	is_namef(t)	(bit_tst(&t->o_bits[0], "Namef", &ob_bits[0]))

#define	obic(t, bn)	(bit_clr(&t->o_bits[0], bn, &ob_bits[0]))
#define	obis(t, bn)	(bit_set(&t->o_bits[0], bn, &ob_bits[0]))
#define	obit(t, bn)	(bit_tst(&t->o_bits[0], bn, &ob_bits[0]))
