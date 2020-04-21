#ifndef PRINTFDEF
#define PRINTFDEF
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

/*
 * Defines for the kernel fprintf
 *
 * $Source: /d2/3.7/src/gl2/gl2/include/RCS/printf.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 15:57:14 $
 */

int (*con_putchar)(), (*con_getchar)();
int grputchar(), grgetchar();
int duputchar(), dugetchar();

/* flags to console putchar routines */
#define	CO_BEGIN	(-1)		/* start output */
#define	CO_END		(-2)		/* end output (flush) */
#define	CO_RESET	(-3)		/* reset the console */

#endif PRINTFDEF
