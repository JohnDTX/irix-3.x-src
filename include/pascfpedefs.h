/* 
 **************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************
 
 
 	These constants define the bits in the longint parameters for
 	the floating point error handling routines setfpe and fpecleanup.
 
 	setfpe is called at the beginning of a program to set the mode
 	of floating point error handling.  setfpe takes two arguments:
 
 			setfpe(mode,errormask)
 
 	where the bits in the var longint parameter mode indicate the
 	mode that floating point exception handling is to operate in
 	and the bits in the var longint parameter errormask indicate which
 	types of floating point exception are to be IGNORED.
 
 	The errormask parameter to setfpe has the same format as the
 	single paramter which is passed to the user-defined error-handling
 	routine fpecleanup.  In this routine, the bits in this parameter
 	indicate the type of floating point error which recently occurred.
 	More than one bit may be set in this parameter, and the user is
 	cautioned to use AND operations to check the bits, rather 
	than a comparison.
 	
	The declaration of setfpe is in the include file pascfpe.h 
	and is as follows:

procedure setfpe(var mode,errormask:longint); external;

	Following are the parameters defining the bits in the errormask
 	parameter to setfpe, and the single parameter for fpecleanup.
 
 	The meaning of these errors is as follows:
 
 		DIVZERO	   -	Divide by zero has occurred.  This includes
 				   both x/0 and mod(x,0).	
 		OVERFLOW   -	The calculation overflowed.
 		UNDERFLOW  - 	The calculation underflowed.
 		DENORM	   -	One of the operands was found to be 
 				   denormalized.
 		ZEROVALS   -	Either the operation underflowed, or one of
 				   the operands was denormalized.
 				   NOTE: the same status must be used for
 				   both DENORM and UNDERFLOW errors.  If
 				   one is to be ignored, so must the other,
 				   and if one is reported, so is the other.
 		NANOP	   -	One of the operands was the special encoding
 				   Not-a-number.
 		ILLEGALOP  -	An illegal operation was requested.  Examples
 				   of illegal operations are 0 * INF, INF/INF,
 				   mod(INF,x) ....
 		INEXACT    - 	The result of the operation was inexact.
 				   NOTE: as inexact errors often occur, e.g.,
 				   the operation 1.0/3.0 is inexact, this
 				   error is NEVER reported alone.  It may,
 				   however, be reported in conjunction with
 				   another error.
 		UNKNOWN    -	This indicates an inconsistancy in the 
 				   floating point error handling code and
 				   should not occur.
 		ALL	   -    All errors should be ignored.
*/
#define DIVZERO 	$1
#define OVERFLOW 	$2
#define UNDERFLOW	$4
#define DENORM		$8
#define NANOP		$10
#define ILLEGALOP	$20
#define INEXACT		$40
#define UNKNOWN		$80
#define ZEROVALS	$88 (* (UNDERFLOW|DENORM) *)
#define ALL		$FF
/* 
 	The following bits set the mode in which error handling is
 	to operate.  If no call has been made to setfpe at the beginning
 	of the program, ZEROVALS errors (i.e., UNDERFLOW and DENORM errors 
 	are not reported 
 	(calculations which UNDERFLOW have their values set to zero, and
 	DENORMalized operands are set to zero). Any other 
 	floating point errors which occur result in
 	a program abort with a core dump.  If a call has been 
 	made to setfpe at the beginning of the program, the 
 	normal course of events when an error which
 	is NOT to be ignored occurs is as follows:
 
 		* The user handler fpecleanup is called with the error mask.
 
 		* A routine is called to print a message informing the
 		  user of the nature of the error.  (i.e., the message	
 		  will indicate divide-by-zero, etc).
 
 		* The program is aborted with a core dump.
 
 	The mode bits defined below alter this course of action when setfpe
 	is used.  Their definition is as follows:
 
 		NOCOREDUMP   -	Dont take a core dump when aborting.
 		NOMESSAGE    -  Dont print a message indicating the error
 				   before exiting.	
 		NOFPECLEANUP -  Dont call the user handler fpecleanup before
 				   aborting.
*/ 
#define NOCOREDUMP	$10
#define NOMESSAGE 	$8000
#define NOFPECLEANUP	$800
/* 
 	As an example, if the user wanted to close a file when a floating
 	point error occurred, to ignore underflow and denormalized operand
 	errors, and to NOT take a core dump at abort, (s)he would call setfpe
 	as follows:
 
 		call setfpe(NOCOREDUMP,ZEROVALS)
 
 	Upon other errors, the routine fpecleanup (which the user should
 	define), is called as follows:
 
 			fpecleanup(errors)
 
 	where errors is an integer*4 parameter which has the corresponding
 	bit set for those errors which occurred.  If fpecleanup returns,
 	the program will be aborted without a core dump.

*/
