
typedef int		(*intfunc) ();

#ifdef library

typedef struct Atnode {
	char		*name;
	struct Atnode	*left;
	struct Atnode	*right;
} Atnode;

typedef struct Name {
	char		*name;
	int		sym;
	int		type;
	intfunc		val;
} Name;

#endif

typedef struct Attr {
	char		*atname;
	int		argc;
	char		**argv;
} Attr;

typedef struct Rec {
	char		*srcpath;
	char		*dstpath;
	unsigned short	type;
	unsigned short	mode;
	char		*user;
	char		*group;
	int		nattr;
	Attr		*attr;
} Rec;

typedef struct Memset {
	int		size;
	int		allocated;
	char		**list;
} Memset;

typedef struct Node {
	int		type;
	intfunc		func;
	struct Node	*n1;
	struct Node	*n2;
} Node;

#ifdef library

/* tokens for statements, other */

#define And		1
#define Andif		2
#define Assign		112
#define Attribute	3
#define Break		108
#define Cat		4
#define Colon		5
#define Comma		6
#define Comp		7
#define Cont		110
#define Dot		8
#define Dotdot		9
#define Else		106
#define Eof		201
#define Eor		10
#define Error		11
#define For		107
#define Func		12
#define Iconst		13
#define If		105
#define Invert		14
#define Lbrace		103
#define Lbrack		15
#define Lpar		16
#define Minus		17
#define Newline		18
#define Not		19
#define Or		20
#define Orif		21
#define Over		22
#define Plus		23
#define Question	24
#define Rbrace		102
#define Rbrack		25
#define Return		109
#define Rpar		26
#define Sconst		27
#define Semi		111
#define Stmtlist	101
#define Subst		28
#define Times		29
#define Var		30
#define While		104
#define Xor		31

/* comparison operators; ordinal */

#define Less		0
#define Lequal		1
#define Equal		2
#define Gequal		3
#define Greater		4
#define Nequal		5
#define Match		6
#define Notmatch	7

#ifndef hidden
#define hidden static
#endif

#endif

/* expression value types; ordinal  */

#define Int		0
#define String		1
#define Bool		2	/* requests only; never set internally */

/* other */

#define esc(c)		((char) (c | 0200))
#define unesc(c)	((char) (c & ~0200))

#define Buffsize	4096
#define Memsetinc	64
#define RBASE		"/root"
#define SBASE		"/usr/src"

/* stdio and some alterations */

#include <stdio.h>
#define peekch(f)	(ungetc (getc (f), f))
FILE		*sopen ();		/* open a string as a FILE */

/* user-callable functions */

extern void	idb_addarg ();
extern void	idb_addargs ();
extern Attr	*idb_addattr ();
extern void	idb_addint ();
extern void	idb_addlong ();
extern char	*idb_addset ();
extern char	*idb_atname ();
extern char	*idb_cat ();
extern void	idb_delattr ();
extern int	idb_expr ();
extern int	idb_freeset ();
extern Attr	*idb_getattr ();
extern char	*idb_getmem ();
extern char	*idb_getmore ();
extern int	idb_gid ();
extern char	*idb_gname ();
extern char	*idb_itoc ();
extern Attr	*idb_newattr ();
extern Memset	*idb_newset ();
extern void	idb_pack ();
extern Node	*idb_parse ();
extern Node	*idb_parsef ();
extern Node	*idb_parses ();
extern int	idb_passwd ();
extern Rec	*idb_read ();
extern void	idb_repattr ();
extern char	*idb_repset ();
extern char	*idb_rpath ();
extern Rec	*idb_select ();
extern void	idb_setbase ();
extern char	*idb_spath ();
extern char	*idb_stash ();
extern int	idb_typec ();
extern int	idb_typei ();
extern int	idb_uid ();
extern char	*idb_uname ();
extern void	idb_write ();

/* well-known external values */

char		rbase [Buffsize];
char		sbase [Buffsize];
char		idb [Buffsize];
