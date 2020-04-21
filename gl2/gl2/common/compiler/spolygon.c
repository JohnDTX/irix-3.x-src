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
#include "globals.h"
#include "TheMacro.h"
#include "immed.h"
#include "glerror.h"

INTERP_NAME(setshade);
INTERP_NAMES(splf);

#define POLY_ROOT(rootname,coord_type,move_macro,draw_macro,add_macro) \
void rootname(n, pol, shades) \
long n; \
coord_type pol[][3]; \
register Colorindex shades[]; \
{ \
    register coord_type	*fptr; \
    register short	j; \
    extern int i_/**/rootname(); \
\
    if (gl_openobjhdr == 0) { \
	im_setup; \
	if (n <= 0) return; \
	fptr = &pol[0][0]; \
	im_setshade(*shades++); \
	move_macro(*fptr++, *fptr++, *fptr++); \
	for (j = n-1; --j != -1;) { \
	    /* the next line works because it is a macro */ \
	    im_setshade(*shades++); \
	    draw_macro(*fptr++, *fptr++, *fptr++); \
	} \
	im_spclos();	\
	im_cleanup; \
	return; \
    } \
    if (n <= 0) { \
	gl_ErrorHandler(ERR_NEGSIDES, WARNING, rootname/**/_n); \
	return; \
    } \
    j = 3 + (n*sizeof(coord_type)>>1) + n*sizeof(coord_type) + n; \
    if (!gl_checkspace(j)) return; \
    BEGINCOMPILE(j); \
    ADDADDR(i_/**/rootname); \
    ADDSHORT(n-1); \
    fptr = &(pol[0][0]); \
    for (j = n; --j != -1;) { \
	ADDSHORT(*shades++); \
	add_macro(*fptr++); \
	add_macro(*fptr++); \
	add_macro(*fptr++); \
    } \
    ENDCOMPILE; \
}

#define POLY2_ROOT(rootname,coord_type,move_macro,draw_macro,add_macro) \
void rootname(n, pol, shades) \
long n; \
coord_type pol[][2]; \
register Colorindex shades[]; \
{ \
    register coord_type	*fptr; \
    register short	j; \
    extern int i_/**/rootname(); \
\
    if (gl_openobjhdr == 0) { \
	im_setup; \
	if (n <= 0) return; \
	fptr = &pol[0][0]; \
	im_setshade(*shades++); \
	move_macro(*fptr++, *fptr++); \
	for (j = n-1; --j != -1;) { \
	    /* the next line works because it is a macro */ \
	    im_setshade(*shades++); \
	    draw_macro(*fptr++, *fptr++); \
	} \
	im_spclos();	\
	im_cleanup; \
	return; \
    } \
    if (n <= 0) { \
	gl_ErrorHandler(ERR_NEGSIDES, WARNING, rootname/**/_n); \
	return; \
    } \
    j = 3 + n*sizeof(coord_type) + n; \
    if (!gl_checkspace(j)) return; \
    BEGINCOMPILE(j); \
    ADDADDR(i_/**/rootname); \
    ADDSHORT(n-1); \
    fptr = &(pol[0][0]); \
    for (j = n; --j != -1;) { \
	ADDSHORT(*shades++); \
	add_macro(*fptr++); \
	add_macro(*fptr++); \
    } \
    ENDCOMPILE; \
}

ROOT_1S(setshade)

POLY_ROOT (splf,Coord,im_pmv,im_pdr,ADDFLOAT)
POLY_ROOT (splfi,Icoord,im_pmvi,im_pdri,ADDICOORD)
POLY_ROOT (splfs,Scoord,im_pmvs,im_pdrs,ADDSCOORD)

POLY2_ROOT (splf2,Coord,im_pmv2,im_pdr2,ADDFLOAT)
POLY2_ROOT (splf2i,Icoord,im_pmv2i,im_pdr2i,ADDICOORD)
POLY2_ROOT (splf2s,Scoord,im_pmv2s,im_pdr2s,ADDSCOORD)

#include "interp.h"

static bogus ()
{
    DECLARE_INTERP_REGS;
    register short i;
    register long *first_point;

INTERP_ROOT_1S(setshade);

INTERP_LABEL (splf,1000006); /* SPOLYLENGTH == 1000006 */
    i = *(short *)PC++;
    im_setshade(*(short *)PC++);
    im_pmv(*(float *)PC++, *(float *)PC++, *(float *)PC++);
    while (--i != -1) {
	im_setshade(*(short *)PC++);
	im_pdr(*(float *)PC++, *(float *)PC++, *(float *)PC++);
    }
    jra(_i_splf_close);

INTERP_LABEL (splfi,1000006); /* SPOLYLENGTH == 1000006 */
    i = *(short *)PC++;
    im_setshade(*(short *)PC++);
    im_pmvi(*PC++, *PC++, *PC++);
    while (--i != -1) {
	im_setshade(*(short *)PC++);
	im_pdri(*PC++, *PC++, *PC++);
    }
    jra(_i_splf_close);

INTERP_LABEL (splfs,1000008); /* SPOLYLENGTH_S == 1000008 */
    i = *(short *)PC++;
    im_setshade(*(short *)PC++);
    im_pmvs(*(short *)PC++, *(short *)PC++, *(short *)PC++);
    while (--i != -1) {
	im_setshade(*(short *)PC++);
	im_pdrs(*(short *)PC++, *(short *)PC++, *(short *)PC++);
    }
    jra(_i_splf_close);

INTERP_LABEL (splf2,1000007); /* SPOLY2LENGTH == 1000007 */
    i = *(short *)PC++;
    im_setshade(*(short *)PC++);
    im_pmv2(*(float *)PC++, *(float *)PC++);
    while (--i != -1) {
	im_setshade(*(short *)PC++);
	im_pdr2(*(float *)PC++, *(float *)PC++);
    }
    jra(_i_splf_close);

INTERP_LABEL (splf2i,1000007); /* SPOLY2LENGTH == 1000007 */
    i = *(short *)PC++;
    im_setshade(*(short *)PC++);
    im_pmv2i(*PC++, *PC++);
    while (--i != -1) {
	im_setshade(*(short *)PC++);
	im_pdr2i(*PC++, *PC++);
    }
    jra(_i_splf_close);

INTERP_LABEL (splf2s,1000009); /* SPOLY2LENGTH_S == 1000009 */
    i = *(short *)PC++;
    im_setshade(*(short *)PC++);
    im_pmv2s(*(short *)PC++, *(short *)PC++);
    while (--i != -1) {
	im_setshade(*(short *)PC++);
	im_pdr2s(*(short *)PC++, *(short *)PC++);
    }
asm("_i_splf_close:");
    im_spclos();
    thread;
}
