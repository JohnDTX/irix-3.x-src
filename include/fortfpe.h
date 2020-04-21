C
C
C**************************************************************************
C*									  *
C* 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
C*									  *
C*  These coded instructions, statements, and computer programs  contain  *
C*  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
C*  are protected by Federal copyright law.  They  may  not be disclosed  *
C*  to  third  parties  or copied or duplicated in any form, in whole or  *
C*  in part, without the prior written consent of Silicon Graphics, Inc.  *
C*									  *
C**************************************************************************/
C
C
C	These constants define the bits in the integer*4 parameters for
C	the floating point error handling routines setfpe and fpecleanup.
C
C	setfpe is called at the beginning of a program to set the mode
C	of floating point error handling.  setfpe takes two arguments:
C
C			setfpe(mode,errormask)
C
C	where the bits in the integer*4 parameter mode indicate the
C	mode that floating point exception handling is to operate in
C	and the bits in the integer*4 parameter errormask indicate which
C	types of floating point exception are to be IGNORED.
C
C	The errormask parameter to setfpe has the same format as the
C	single paramter which is passed to the user-defined error-handling
C	routine fpecleanup.  In this routine, the bits in this parameter
C	indicate the type of floating point error which recently occurred.
C	More than one bit may be set in this parameter, and the user is
C	cautioned to use IAND to check the bits, rather than a comparison.
C	
C	Following are the parameters defining the bits in the errormask
C	parameter to setfpe, and the single parameter for fpecleanup.
C
C	The meaning of these errors is as follows:
C
C		DIVZERO	   -	Divide by zero has occurred.  This includes
C				   both x/0 and mod(x,0).	
C		OVERFLOW   -	The calculation overflowed.
C		UNDERFLOW  - 	The calculation underflowed.
C		DENORM	   -	One of the operands was found to be 
C				   denormalized.
C		ZEROVALS   -	Either the operation underflowed, or one of
C				   the operands was denormalized.
C				   NOTE: the same status must be used for
C				   both DENORM and UNDERFLOW errors.  If
C				   one is to be ignored, so must the other,
C				   and if one is reported, so is the other.
C		NANOP	   -	One of the operands was the special encoding
C				   Not-a-number.
C		ILLEGALOP  -	An illegal operation was requested.  Examples
C				   of illegal operations are 0 * INF, INF/INF,
C				   mod(INF,x) ....
C		INEXACT    - 	The result of the operation was inexact.
C				   NOTE: as inexact errors often occur, e.g.,
C				   the operation 1.0/3.0 is inexact, this
C				   error is NEVER reported alone.  It may,
C				   however, be reported in conjunction with
C				   another error.
C		UNKNOWN    -	This indicates an inconsistancy in the 
C				   floating point error handling code and
C				   should not occur.
C		ALL	   -    All errors should be ignored.
C
C	NOTE:
C
	INTEGER*4 DIVZERO,OVERFLOW,UNDERFLOW,DENORM,NANOP,
     1		  ILLEGALOP,INEXACT,UNKNOWN,ALL,ZEROVALS
	PARAMETER (DIVZERO = $1, OVERFLOW = $2, UNDERFLOW = $4, DENORM = $8,
     1		NANOP = $10, ILLEGALOP = $20, INEXACT = $40, UNKNOWN = $80,
     2		ALL = $7F, ZEROVALS = $C )
C
C	The following bits set the mode in which error handling is
C	to operate.  If no call has been made to setfpe at the beginning
C	of the program, ZEROVALS errors (i.e., UNDERFLOW and DENORM errors 
C	are not reported 
C	(calculations which UNDERFLOW have their values set to zero, and
C	DENORMalized operands are set to zero). Any other 
C	floating point errors which occur result in
C	a program abort with a core dump.  If a call has been 
C	made to setfpe at the beginning of the program, the 
C	normal course of events when an error which
C	is NOT to be ignored occurs is as follows:
C
C		* The user handler fpecleanup is called with the error mask.
C
C		* A routine is called to print a message informing the
C		  user of the nature of the error.  (i.e., the message	
C		  will indicate divide-by-zero, etc).
C
C		* The program is aborted with a core dump.
C
C	The mode bits defined below alter this course of action when setfpe
C	is used.  Their definition is as follows:
C
C		NOCOREDUMP   -	Dont take a core dump when aborting.
C		NOMESSAGE    -  Dont print a message indicating the error
C				   before exiting.	
C		NOFPECLEANUP -  Dont call the user handler fpecleanup before
C				   aborting.
C
	INTEGER*4 NOCOREDUMP,NOMESSAGE,NOFPECLEANUP
	PARAMETER (NOCOREDUMP = $10, NOMESSAGE = $8000, NOFPECLEANUP = $800)
C
C
C	As an example, if the user wanted to close a file when a floating
C	point error occurred, to ignore underflow and denormalized operand
C	errors, and to NOT take a core dump at abort, (s)he would call setfpe
C	as follows:
C
C		call setfpe(NOCOREDUMP,ZEROVALS)
C
C	Upon other errors, the routine fpecleanup (which the user should
C	define), is called as follows:
C
C			fpecleanup(errors)
C
C	where errors is an integer*4 parameter which has the corresponding
C	bit set for those errors which occurred.  If fpecleanup returns,
C	the program will be aborted without a core dump.
