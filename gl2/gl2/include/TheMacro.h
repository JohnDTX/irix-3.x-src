#ifndef THEMACRO 
#define THEMACRO
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

	/* Dedicated to Herb */

#define beginpicmandef(size)    				\
{								\
    if (gl_openobjhdr == 0)					\
	gl_currentpos = &(gl_temporary_object[0]);		\
    else {							\
	if ((gl_checkspace)(size) == 0) return;			\
    }								\
}

#define endpicmandef	 					\
{								\
    if (gl_openobjhdr == 0) {					\
	extern int i_retsym();					\
	*(long *)gl_currentpos = (long)i_retsym;		\
	gl_interpret(gl_temporary_object);			\
    }								\
}

#ifdef DEBUG
#define BEGINCOMPILE(x)	{ register long *_curpos = (long *)gl_currentpos;\
			  long _temp = (x)
#define ENDCOMPILE   if (_temp != (((long)_curpos - (long)gl_currentpos)>>1))\
			gl_endcompile_error(_temp,_curpos,gl_currentpos);\
			gl_currentpos = (short *)_curpos; }
#else
#define BEGINCOMPILE(x) {register long *_curpos = (long *)gl_currentpos
#define ENDCOMPILE	gl_currentpos = (short *)_curpos; }
#endif DEBUG

#define ADDSHORT(x)	*(short *)_curpos++ = (short)(x)
#define ADDLONG(x)	*_curpos++ = (x)
#define ADDADDR(x)	*_curpos++ = (long)(x)
#define ADDSCOORD(x)	*(short *)_curpos++ = (short)(x)
#define ADDICOORD(x)	*_curpos++ = (x)
#define ADDFLOAT(x)	*(float *)_curpos++ = (x)

/* some pretty complex but powerful macros by Gary that save mucho typing */

#define ROOT(root) \
	ROOT_2F(root/**/2) ROOT_2I(root/**/2i) ROOT_2S(root/**/2s) \
	ROOT_3F(root) ROOT_3I(root/**/i) ROOT_3S(root/**/s)


#define ROOT_0(root) \
void root () \
{ \
    extern int i_/**/root (); \
    if (gl_openobjhdr == 0) { \
	im_setup; \
	im_/**/root(); \
	im_cleanup; \
	return; \
    } \
    if ((gl_checkspace)(2) == 0) return; \
    BEGINCOMPILE(2); \
    ADDADDR(i_/**/root); \
    ENDCOMPILE; \
}

#define ROOT_1S(root) \
void root (x) \
short x; \
{ \
    extern int i_/**/root (); \
    if (gl_openobjhdr == 0) { \
	im_setup; \
	im_/**/root(x); \
	im_cleanup; \
	return; \
    } \
    if ((gl_checkspace)(3) == 0) return; \
    BEGINCOMPILE(3); \
    ADDADDR(i_/**/root); \
    ADDSHORT(x); \
    ENDCOMPILE; \
}

#define ROOT_1I(root) \
void root (x) \
Icoord x; \
{ \
    extern int i_/**/root (); \
    if (gl_openobjhdr == 0) { \
	im_setup; \
	im_/**/root(x); \
	im_cleanup; \
	return; \
    } \
    if ((gl_checkspace)(4) == 0) return; \
    BEGINCOMPILE(4); \
    ADDADDR(i_/**/root); \
    ADDLONG(x); \
    ENDCOMPILE; \
}

#define ROOT_1F(root) \
void root (x) \
float x; \
{ \
    extern int i_/**/root (); \
    if (gl_openobjhdr == 0) { \
	im_setup; \
	im_/**/root(x); \
	im_cleanup; \
	return; \
    } \
    if ((gl_checkspace)(4) == 0) return; \
    BEGINCOMPILE(4); \
    ADDADDR(i_/**/root); \
    ADDFLOAT(x); \
    ENDCOMPILE; \
}

#define ROOT_2S(root) \
void root (x,y) \
short x,y; \
{ \
    extern int i_/**/root (); \
    if (gl_openobjhdr == 0) { \
	im_setup; \
	im_/**/root(x,y); \
	im_cleanup; \
	return; \
    } \
    if ((gl_checkspace)(4) == 0) return; \
    BEGINCOMPILE(4); \
    ADDADDR(i_/**/root); \
    ADDSHORT(x); \
    ADDSHORT(y); \
    ENDCOMPILE; \
}

#define ROOT_2I(root) \
void root (x,y) \
Icoord x,y; \
{ \
    extern int i_/**/root (); \
    if (gl_openobjhdr == 0) { \
	im_setup; \
	im_/**/root(x,y); \
	im_cleanup; \
	return; \
    } \
    if ((gl_checkspace)(6) == 0) return; \
    BEGINCOMPILE(6); \
    ADDADDR(i_/**/root); \
    ADDICOORD(x); \
    ADDICOORD(y); \
    ENDCOMPILE; \
}

#define ROOT_2F(root) \
void root (x,y) \
float x,y; \
{ \
    extern int i_/**/root (); \
    if (gl_openobjhdr == 0) { \
	im_setup; \
	im_/**/root(x,y); \
	im_cleanup; \
	return; \
    } \
    if ((gl_checkspace)(6) == 0) return; \
    BEGINCOMPILE(6); \
    ADDADDR(i_/**/root); \
    ADDFLOAT(x); \
    ADDFLOAT(y); \
    ENDCOMPILE; \
}

#define ROOT_3S(root) \
void root (x,y,z) \
short x,y,z; \
{ \
    extern int i_/**/root (); \
    if (gl_openobjhdr == 0) { \
	im_setup; \
	im_/**/root(x,y,z); \
	im_cleanup; \
	return; \
    } \
    if ((gl_checkspace)(5) == 0) return; \
    BEGINCOMPILE(5); \
    ADDADDR(i_/**/root); \
    ADDSHORT(x); \
    ADDSHORT(y); \
    ADDSHORT(z); \
    ENDCOMPILE; \
}

#define ROOT_3I(root) \
void root (x,y,z) \
Icoord x,y,z; \
{ \
    extern int i_/**/root (); \
    if (gl_openobjhdr == 0) { \
	im_setup; \
	im_/**/root(x,y,z); \
	im_cleanup; \
	return; \
    } \
    if ((gl_checkspace)(8) == 0) return; \
    BEGINCOMPILE(8); \
    ADDADDR(i_/**/root); \
    ADDICOORD(x); \
    ADDICOORD(y); \
    ADDICOORD(z); \
    ENDCOMPILE; \
}

#define ROOT_3F(root) \
void root (x,y,z) \
float x,y,z; \
{ \
    extern int i_/**/root (); \
    if (gl_openobjhdr == 0) { \
	im_setup; \
	im_/**/root(x,y,z); \
	im_cleanup; \
	return; \
    } \
    if ((gl_checkspace)(8) == 0) return; \
    BEGINCOMPILE(8); \
    ADDADDR(i_/**/root); \
    ADDFLOAT(x); \
    ADDFLOAT(y); \
    ADDFLOAT(z); \
    ENDCOMPILE; \
}

#define ROOT_4S(root) \
void root (x,y,z,w) \
short x,y,z,w; \
{ \
    extern int i_/**/root (); \
    if (gl_openobjhdr == 0) { \
	im_setup; \
	im_/**/root(x,y,z,w); \
	im_cleanup; \
	return; \
    } \
    if ((gl_checkspace)(6) == 0) return; \
    BEGINCOMPILE(6); \
    ADDADDR(i_/**/root); \
    ADDSHORT(x); \
    ADDSHORT(y); \
    ADDSHORT(z); \
    ADDSHORT(w); \
    ENDCOMPILE; \
}

#define ROOT_4I(root) \
void root (x,y,z,w) \
Icoord x,y,z,w; \
{ \
    extern int i_/**/root (); \
    if (gl_openobjhdr == 0) { \
	im_setup; \
	im_/**/root(x,y,z,w); \
	im_cleanup; \
	return; \
    } \
    if ((gl_checkspace)(10) == 0) return; \
    BEGINCOMPILE(10); \
    ADDADDR(i_/**/root); \
    ADDICOORD(x); \
    ADDICOORD(y); \
    ADDICOORD(z); \
    ADDICOORD(w); \
    ENDCOMPILE; \
}

#define ROOT_4F(root) \
void root (x,y,z,w) \
float x,y,z,w; \
{ \
    extern int i_/**/root (); \
    if (gl_openobjhdr == 0) { \
	im_setup; \
	im_/**/root(x,y,z,w); \
	im_cleanup; \
	return; \
    } \
    if ((gl_checkspace)(10) == 0) return; \
    BEGINCOMPILE(10); \
    ADDADDR(i_/**/root); \
    ADDFLOAT(x); \
    ADDFLOAT(y); \
    ADDFLOAT(z); \
    ADDFLOAT(w); \
    ENDCOMPILE; \
}

#define ROOT_MATRIX(root) \
void root (m)	\
Matrix m;	\
{ \
    extern int i_/**/root (); \
    if (gl_openobjhdr == 0) {	\
	im_setup;	\
	im_/**/root(m);	\
	im_cleanup;	\
	return;		\
    } \
    else {	\
	register short	i;	\
	register float	*farray;	\
	if ((gl_checkspace)(34) == 0) return; \
	BEGINCOMPILE(34);		\
	ADDADDR(i_/**/root);	\
	farray = &(m[0][0]);	\
	for (i = 16; --i != -1;) {	\
	    ADDFLOAT(*farray++);	\
	}	\
	ENDCOMPILE; \
    }	\
}

#define INTERP_NAMES(rootname) \
	INTERP_NAME(rootname/**/2);	INTERP_NAME(rootname);	\
	INTERP_NAME(rootname/**/2i);	INTERP_NAME(rootname/**/i);	\
	INTERP_NAME(rootname/**/2s);	INTERP_NAME(rootname/**/s)
#define INTERP_NAME(name) static char name/**/_n[] = "name"

#define MAXSTANDARDLENGTH 1000000
#define POLYLENGTH	1000001
#define POLY2LENGTH	1000002
#define CALLFUNCLENGTH  1000003
#define POLYLENGTH_S	1000004
#define POLY2LENGTH_S	1000005

#define SPOLYLENGTH	1000006
#define SPOLY2LENGTH	1000007
#define SPOLYLENGTH_S	1000008
#define SPOLY2LENGTH_S	1000009

#endif THEMACRO
