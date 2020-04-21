#include <sys/signal.h>
#include <fpsignal.h>
#include <fperr.h>

int _is_fortran;

unsigned long fakehwerror(); 

float 
_raise_fperror(op,type)
{
	/* raise a floating point exception on the last operation */
	_fperror.type = type;
	_fperror.operation = op;
	_fperror.precision = (op == FIX)?UNKNOWN:SINGLE;
	_is_software = 1;
	if (op != MATH) _fperror.val.fval = 0.0;
	if (fakehwerror() != 0) 
		kill (getpid(),SIGFPE);
	_is_software = 0;
	return(_fperror.val.fval);
}


long float 
_lraise_fperror(op,type)

{
	/* raise a floating point exception on the last operation */
	_fperror.type = type;
	_fperror.operation = op;
	_fperror.precision = DOUBLE;
	_is_software = 1;
	if (op != MATH) _fperror.val.dval= 0.0;
	if (fakehwerror() != 0) 
		kill (getpid(),SIGFPE);
	_is_software = 0;
	return(_fperror.val.dval);
}

_UPERR(type)
{
	_is_software = 1;
	_is_fortran = 1;
	_fperror.type = type;
	_fperror.operation = 0;
	_fperror.precision = UNKNOWN;
	_fperror.val.fval = 0.0;
	if (fakehwerror() != 0) 
		kill (getpid(),SIGFPE);
	_is_software = 0;
	_is_fortran = 0;
	return(0);
}

float 
_FPERR(type)
{
	_is_software = 1;
	_is_fortran = 1;
	_fperror.type = type;
	_fperror.operation = 0;
	_fperror.precision = SINGLE;
	_fperror.val.fval = 0.0;
	if (fakehwerror() != 0) 
		kill (getpid(),SIGFPE);
	_is_software = 0;
	_is_fortran = 0;
	return(_fperror.val.fval);
}

long float 
_LFPERR(type)
{
	_is_software = 1;
	_is_fortran = 1;
	_fperror.type = type;
	_fperror.operation = 0;
	_fperror.precision = DOUBLE;
	_fperror.val.dval = 0.0;
	if (fakehwerror() != 0) 
		kill (getpid(),SIGFPE);
	_is_software = 0;
	_is_fortran = 0;
	return(_fperror.val.dval);
}


