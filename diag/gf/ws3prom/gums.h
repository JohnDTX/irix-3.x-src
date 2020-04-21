/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/
/* gums.h	GF1/PM1
 */

#ifndef  GMSDEF
#define  GMSDEF

#include	"gf2.h"
#include	"betacodes.h"

#define	gsetup		register unsigned short *GE = &GEPORT; \
			FBCflags = gl_fbcstatus = RUNMODE

#define gshort(x)	*(short *)GE = x
#define glong(x)	*(long *)GE = x
#define gpassthru(n)	*(short *)GE = GEpassthru|((n-1)<<8)

#define feedsetup	register unsigned short *FD = &FBCdata; \
			register unsigned short *FF = &FBCflags
#define fsetup		feedsetup;	*FF = RUNSUBST;
#define grestore	FBCflags = RUNMODE;

#define fcycle		*FF = RUNSUBST & ~FORCEREQ_BIT_; *FF = RUNSUBST
#define fshort(x)	*FD = (x); fcycle
#define flong(x)	{register _x = (x); *FD = _x>>16; fcycle; \
					    *FD = (short)_x; fcycle;  }
#define fwait()		while (!(FBCflags & GET_BIT)) ;
#define fpassthru()	*FD = 0x808; fcycle
				/* note -- can only say 808 ! */

#define feedwait()	while (*FF & INTERRUPT_BIT_) ;
#define feeddata	*FD
#define endfeedback()	GEflags = gl_gestatus

/*=======================================================================*/
#endif GMSDEF
