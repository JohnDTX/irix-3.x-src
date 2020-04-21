/* 	up_i - return base^pow where pow is an integer */

#include <math.h>
#include <fperr.h>
#include <fpregs.h>
#include <nan.h>
#include <fpopcodes.h>

float up_i(base,pow)
float base;
int pow;
{
	register short s;
	static float f;
	static errtype;
		switch (pow) {

			case 0:	if (base == 0.0)  {
						_fperror.val.fval = 0.0;
						_mathfunc_id = UP_I;
						return(_raise_fperror(MATH,DOMAIN_ERROR));
					}
					return(1.0);
			case 1: return(base);
			default: 
					/* float the power and raise it. */
					f = pow;
					*SKYCOMREG = HW_POW;
					*SKYFLREG = base;
					while ((s = *SKYSTATREG) > 0) ;
					*SKYFLREG = f;
					/* now do the exception checking */

					_mathfunc_id = UP_I;
					errtype = 0;
					if (ISFMaxExp(base))  {
						errtype++;
						_fperror.val.fval = base;
					}
					else if (ISFMaxExp(f)) {
						errtype++;
						_fperror.val.fval = f;
					}
	
					while ((s = *SKYSTATREG) > 0) ;
					f = *SKYFLREG;
					if (errtype) errtype = 
						FMANT(_fperror.val.fval)?INVALID_OP_A:INVALID_OP_F2;
					else if (f==0.0) errtype = UNDERFL;
					else if (ISFMaxExp(f)) errtype = OVERFL;
					else return(f);

					return(_raise_fperror(MATH,errtype));

		}
}
