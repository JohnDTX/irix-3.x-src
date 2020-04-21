/*
 * $Source: /d2/3.7/src/include/RCS/nan.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:11:41 $
 */

/* Handling of Not_a_Number's (only in IEEE floating-point standard) */

/* uses signal.h, values.h,  and fperr.h */
#include <fperr.h>
#ifndef SIG_IGN
#include <signal.h>
#endif
#include <values.h> 

#define Nan(X)	(((union { long float d; struct { unsigned :1, e:11; } s; } \
			*)&X)->s.e == 0x7ff)



#define ISFMaxExp(X)	(((union { float d; \
				   struct { unsigned :1, e:8; } s; \
				   }  *)&X)->s.e == 0xff)
#define ISMaxExp(X)	(((union { long float d; \
				   struct { unsigned :1, e:11; } s; \
				   } *)&X)->s.e == 0x7ff)

#define FMANT(X)	(((union { float d; \
				   struct { unsigned :9, m:23;} s; \
				   }  *)&X)->s.m != 0 )
#define MANT(X)		((((union { long float d; \
				   struct { unsigned :12, m:20; } s; \
				   } *)&X)->s.m != 0 ) || \
			(((union { long float d; \
				   struct { long l1,l2; } s;  \
				   } *)&X)->s.l2 != 0 ) )

#define KILLNaN(X) if (ISMaxExp(X)) { \
		     if (MANT(X)) X = _lraise_fperror(CONVERT,INVALID_OP_A); \
		     else X=_lraise_fperror(CONVERT,CONVERT_INFINITY);\
			 X = (*(long *)&X < 0)?-MAXDOUBLE:MAXDOUBLE; \
		   }

#define KILLFNaN(X) if (ISFMaxExp(X)) { \
		     if (FMANT(X)) X = _raise_fperror(CONVERT,INVALID_OP_A); \
		     else X=_raise_fperror(CONVERT,CONVERT_INFINITY);\
			 X = (*(long *)&X < 0)?-MAXFLOAT:MAXFLOAT; \
		   }

#define KILLFPE()	(void) kill(getpid(), SIGFPE)
