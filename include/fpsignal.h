/*
 * $Source: /d2/3.7/src/include/RCS/fpsignal.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:11:30 $
 */

#ifndef __FPSIGNAL__
#define __FPSIGNAL__

/*	fpsignal.h - necessary defines for floating point
	signal catchers 
*/

#define INHIBIT_FPMESSAGE 0x8000
#define IGN_FORTHANDLER 0x800
#define CONTINUE_AFTER_FPERROR 1
#define INHIBIT_DUMP 0x10
#define FORT_HANDLER 0x20
#define NOFPE_DUMP INHIBIT_DUMP
#define NOFPE_ABORT CONTINUE_AFTER_FPERROR
#define NOFPE_MSG INHIBIT_FPMESSAGE
#define IGN_DIVZERO 	0x10000
#define IGN_OVERFLOW 	0x20000
#define IGN_UNDERFLOW	0x40000
#define IGN_DENORM	0x80000
#define IGN_NANOP	0x100000
#define IGN_ILLEGALOP	0x200000
#define IGN_INEXACT	0x400000
#define IGN_UNKNOWN	0x800000
#define IGN_ZEROVALS	(IGN_UNDERFLOW|IGN_DENORM)
#define IGN_ALL		0xFF0000

#endif

