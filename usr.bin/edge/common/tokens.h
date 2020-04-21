/*
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/tokens.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:46:09 $
 */
#define T_NOT 	0
#define T_NOTEQ 1
#define T_MOD 	2
#define T_ASGMOD 3
#define T_ANDAND 4
#define T_AND 5
#define T_ASGAND 6
#define T_LP
#define T_RP
#define T_MUL 9
/* 10 */
#define T_ASGMUL 10
#define T_PLUS 	11
#define T_INCR  12
#define T_ASGPLUS  13
#define T_CM	14
#define T_MINUS  15
#define T_DECR  16
#define T_ASGMINUS 17
#define T_INDSTREF 18
#define T_STREF 19
/* 20 */
/*
#define T_ELLIPSES2 (T_STREF + 1)
#define T_ELLIPSES3 (T_ELLIPSES2 + 1)
*/
#define T_ELLIPSES3 (T_STREF + 1)
#define T_DIV  (T_ELLIPSES3 + 1)
#define T_ASGDIV (T_DIV+1)
#define T_COLON	(T_ASGDIV+1)
#define T_SM	(T_COLON+1	)
#define T_LT 	(T_SM+1)
#define T_LSHIFT (T_LT+1)
#define T_ASGLSHIFT (T_LSHIFT+1)
#define T_LE 	(T_ASGLSHIFT+1)
#define T_EQ 	(T_LE+1)

/* 30 */
#define T_EQEQ (T_EQ+1)
#define T_GT (T_EQEQ +1)
#define T_GE (T_GT+1) 
#define T_RSHIFT  (T_GE +1)
#define T_ASGRSHIFT (T_RSHIFT +1)
#define T_QUEST (T_ASGRSHIFT+1)
#define T_LBRACKET (T_QUEST+1)
#define T_RBRACKET (T_LBRACKET+1)
#define T_XOR   (T_RBRACKET+1)
#define T_ASGXOR  (T_XOR+1)

/* 40 */
#define T_EOFTOKEN (T_ASGXOR+1)	
#define T_AUTO	(T_EOFTOKEN+1)		
#define T_BREAK (T_AUTO+1)
#define T_CASE	(T_BREAK+1)
#define T_CHAR 	(T_CASE+1)
#define T_CONST	(T_CHAR+1)
#define T_CONTINUE (T_CONST+1)
#define T_DEFAULT (T_CONTINUE+1)
#define T_DO	(T_DEFAULT+1)
#define T_DOUBLE (T_DO +1)

/* 50 */
#define T_ELSE  (T_DOUBLE+1)	
#define T_ENUM	(T_ELSE+1)	
#define T_EXTERN (T_ENUM+1)
#define T_FLOAT	(T_EXTERN+1)
#define T_FOR (T_FLOAT+1)
#define T_GOTO (T_FOR+1)
#define T_IDENT	(T_GOTO+1)
#define T_IF	(T_IDENT+1)
#define T_INT	(T_IF+1)
#define T_LONG	(T_INT+1)

/* 60 */
#define T_NAMEDTYPE (T_LONG+1)
#define T_NUMBER (T_NAMEDTYPE+1)
#define T_REGISTER (T_NUMBER+1)
#define T_RETURN (T_REGISTER+1)
#define T_SHORT	(T_RETURN+1)	
#define T_SIGNED (T_SHORT+1)
#define T_SIZEOF (T_SIGNED+1)
#define T_STATIC (T_SIZEOF+1)
#define T_STRING (T_STATIC+1)
#define T_STRUCT (T_STRING+1)

/* 70 */
#define T_SWITCH (T_STRUCT+1)
#define T_TYPEDEF (T_SWITCH+1)
#define T_UNION	 (T_TYPEDEF+1)
#define T_UNSIGNED (T_UNION+1)
#define T_VOID	(T_UNSIGNED+1)
#define T_VOLATILE (T_VOID+1)
#define T_WHILE	(T_VOLATILE+1)
#define T_LBRACE (T_WHILE+1)
#define T_OR  (T_LBRACE+1)	
#define T_ASGOR (T_OR+1)

/* 80 */
#define T_OROR 	(T_ASGOR+1)
#define T_RBRACE (T_OROR+1)
#define T_COMPL	 (T_RBRACE+1)
#define T_ASGCOMPL  (T_COMPL+1)

