/*
 * $Source: /d2/3.7/src/include/RCS/execargs.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:11:21 $
 */

#if vax
char **execargs = (char**)(0x7ffffffc);
#endif

#if pdp11
char **execargs = (char**)(-2);
#endif

#if u3b
/* empty till we can figure out what to do for the shell */
#endif

#if sgi
/* don't quite know what to put here */
ERROR ERROR ERROR who uses me? ERROR ERROR ERROR
#endif
