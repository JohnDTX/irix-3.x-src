/*
 * $Source: /d2/3.7/src/sys/h/RCS/setjmp.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:57 $
 */

#ifndef _JBLEN

#if vax
#define _JBLEN	10
#endif

#if pdp11
#define _JBLEN	3
#endif

#if u370
#define _JBLEN	4
#endif

#if u3b
#define _JBLEN	11
#endif

#if m68000
#define _JBLEN	13
#endif

typedef int jmp_buf[_JBLEN];

extern int	setjmp();
extern void	longjmp();

#endif
