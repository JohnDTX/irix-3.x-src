#include "lex.h"
#include "tokens.h"

#define MAXTOKENS 20
token_t token[MAXTOKENS] = {
	/* types */
	"int",		INT|TYPE,
	"short",	SHORT|TYPE,
	"char",		CHAR|TYPE,
	"float",	FLOAT|TYPE,
	"double",	FLOAT|TYPE,
			/* fix for scr1943 */
	"void",		VOID|TYPE,
			/* end of fix for scr1943 */
	"pointer",	PTR|TYPE,
	"struct",	SU,
	"union",	SU,
	/* modifiers */
	"static",	STATIC|CLASS,
	"long",		LONG|CLASS,
	"unsigned",	CLASS|UNSIGNED,
	"register",	CLASS,
	/* end marker */
	"",0
	};



