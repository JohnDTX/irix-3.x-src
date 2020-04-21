#include <fpsignal.h>
#include <fperr.h>

unsigned long _hw_errmask = (IGN_ALL);

unsigned long
fakehwerror() 
{
	/* given the indicated software error code, translate it to
	   the corresponding turbo fp error.  If the 
	   error is masked, set the result value to the one returned
	   by the board.
	*/
	unsigned long result;

	switch (_fperror.type) {

		case INVALID_OP_D1	:
		case INVALID_OP_D3	:
		case INVALID_OP_E1	:
		case DIVZERO		:
				result = IGN_DIVZERO;
				break;

/* this is the equivalent of the turbo fpa OVERFLOW */
		case INVALID_OP_E2	: /* mod(INF,x) -- emulate the h/w */
		case INVALID_OP_G	: /* conversion flt->int overflowed */
		case OVERFL		:
				result = IGN_OVERFLOW;
				break;

/* these are the equivalent of the turbo fpa UNDERFLOW */
		case UNDERFL		:
		case UNDERFLOW_A	:
		case UNDERFLOW_B	:
				result = IGN_UNDERFLOW;
				break;

/* INVALID_OP_F3 is  the equivalent of turbo fpa DENORM */
		case INVALID_OP_F3	:
				result = IGN_DENORM;
				break;

/* these are the equivalent of turbo fpa NANOP */
		case INVALID_OP	:
		case INVALID_OP_A	:
				result = IGN_NANOP;
				break;

/* this is the equivalent of INEXACT using the turbo fpa */
		case PARTIAL_SLOSS	:
		case INEXACT	:
				result = IGN_INEXACT;
				break;

		case INVALID_OP_B1	:
		case INVALID_OP_B	:
		case INVALID_OP_B2	:
		case INVALID_OP_C	:
		case INVALID_OP_D2	:
		case INVALID_OP_F2	:
		case INVALID_OP_H:
		case INVALID_OP_H1	:
		case DOMAIN_ERROR	:
		case CANT_REDUCE_RANGE  :
		case ILLEGAL_OP 	:
				result = IGN_ILLEGALOP;
				break;
		default:
			result = IGN_UNKNOWN;
			break;
	}

	/*
	   Set the result to the default result to be returned.
	*/

	if (result == IGN_UNDERFLOW)
		_fperror.val.dval = 0.0;
	else if (_fperror.precision == DOUBLE)
		_fperror.val.uval = 0xffe00001;
	else
		_fperror.val.uval = 0x7f800001;

	if (_hw_errmask & result) {
		/* the error is to be ignored.  
		*/
		result = 0;
	}

	return(result); 
}

