/*
 * $Source: /d2/3.7/src/include/RCS/assert.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:11:13 $
 */

#ifdef NDEBUG
#define assert(EX)
#else
extern void _assert();
#define assert(EX) if (EX) ; else _assert("EX", __FILE__, __LINE__)
#endif
