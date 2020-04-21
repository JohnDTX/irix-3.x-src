/*
 * $Source: /d2/3.7/src/include/RCS/varargs.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:12:15 $
 */

typedef char *va_list;
#define va_dcl int va_alist;
#define va_start(list) list = (char *) &va_alist
#define va_end(list)
#ifdef u370
#define va_arg(list, mode) ((mode *)(list = \
	(char *) ((int)list + 2*sizeof(mode) - 1 & -sizeof(mode))))[-1]
#else
#define va_arg(list, mode) ((mode *)(list += sizeof(mode)))[-1]
#endif
