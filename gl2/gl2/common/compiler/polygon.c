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
#include "imdraw.h"
#include "glerror.h"

INTERP_NAMES(polf);
INTERP_NAMES(poly);

#define POLY_ROOT(rootname,coord_type,move_macro,draw_macro,close_macro,add_macro) \
void rootname(n, pol) \
long n; \
register coord_type pol[][3]; \
{ \
    register coord_type	*fptr; \
    register short	j; \
    extern int i_/**/rootname(); \
\
    if (gl_openobjhdr == 0) { \
	im_setup; \
	if (n <= 0) return; \
	fptr = &pol[0][0]; \
	move_macro(*fptr++, *fptr++, *fptr++); \
	for (j = n-1; --j != -1;) { \
	    /* the next line works because it is a macro */ \
	    draw_macro(*fptr++, *fptr++, *fptr++); \
	} \
	close_macro(pol[0][0], pol[0][1], pol[0][2]); \
	im_cleanup; \
	return; \
    } \
    if (n <= 0) { \
	gl_ErrorHandler(ERR_NEGSIDES, WARNING, rootname/**/_n); \
	return; \
    } \
    j = 3 + (n*sizeof(coord_type)>>1) + n*sizeof(coord_type); \
    if (!gl_checkspace(j)) return; \
    BEGINCOMPILE(j); \
    ADDADDR(i_/**/rootname); \
    ADDSHORT(n-1); \
    fptr = &(pol[0][0]); \
    for (j = n; --j != -1;) { \
	add_macro(*fptr++); \
	add_macro(*fptr++); \
	add_macro(*fptr++); \
    } \
    ENDCOMPILE; \
}

#define POLY2_ROOT(rootname,coord_type,move_macro,draw_macro,close_macro,add_macro) \
void rootname(n, pol) \
long n; \
register coord_type pol[][2]; \
{ \
    register coord_type	*fptr; \
    register short	j; \
    extern int i_/**/rootname(); \
\
    if (gl_openobjhdr == 0) { \
	im_setup; \
	if (n <= 0) return; \
	fptr = &pol[0][0]; \
	move_macro(*fptr++, *fptr++); \
	for (j = n-1; --j != -1;) { \
	    /* the next line works because it is a macro */ \
	    draw_macro(*fptr++, *fptr++); \
	} \
	close_macro(pol[0][0], pol[0][1]); \
	im_cleanup; \
	return; \
    } \
    if (n <= 0) { \
	gl_ErrorHandler(ERR_NEGSIDES, WARNING, rootname/**/_n); \
	return; \
    } \
    j = 3 + n*sizeof(coord_type); \
    if (!gl_checkspace(j)) return; \
    BEGINCOMPILE(j); \
    ADDADDR(i_/**/rootname); \
    ADDSHORT(n-1); \
    fptr = &(pol[0][0]); \
    for (j = n; --j != -1;) { \
	add_macro(*fptr++); \
	add_macro(*fptr++); \
    } \
    ENDCOMPILE; \
}

#define POLY_CLOSE(x,y,z) im_pclos()
#define POLY2_CLOSE(x,y) im_pclos()

POLY_ROOT (polf,Coord,im_pmv,im_pdr,POLY_CLOSE,ADDFLOAT)
POLY_ROOT (poly,Coord,im_move,im_draw,im_draw,ADDFLOAT)
POLY_ROOT (polfi,Icoord,im_pmvi,im_pdri,POLY_CLOSE,ADDICOORD)
POLY_ROOT (polyi,Icoord,im_movei,im_drawi,im_drawi,ADDICOORD)
POLY_ROOT (polfs,Scoord,im_pmvs,im_pdrs,POLY_CLOSE,ADDSCOORD)
POLY_ROOT (polys,Scoord,im_moves,im_draws,im_draws,ADDSCOORD)

POLY2_ROOT (polf2,Coord,im_pmv2,im_pdr2,POLY2_CLOSE,ADDFLOAT)
POLY2_ROOT (poly2,Coord,im_move2,im_draw2,im_draw2,ADDFLOAT)
POLY2_ROOT (polf2i,Icoord,im_pmv2i,im_pdr2i,POLY2_CLOSE,ADDICOORD)
POLY2_ROOT (poly2i,Icoord,im_move2i,im_draw2i,im_draw2i,ADDICOORD)
POLY2_ROOT (polf2s,Scoord,im_pmv2s,im_pdr2s,POLY2_CLOSE,ADDSCOORD)
POLY2_ROOT (poly2s,Scoord,im_move2s,im_draw2s,im_draw2s,ADDSCOORD)

#include "interp.h"

static bogus ()
{
    DECLARE_INTERP_REGS;
    register short i;
    register long *first_point;

INTERP_LABEL (polf,1000001); /* POLYLENGTH == 1000001 */
    i = *(short *)PC++;
    im_pmv(*(float *)PC++, *(float *)PC++, *(float *)PC++);
    while (--i != -1) {
	im_pdr(*(float *)PC++, *(float *)PC++, *(float *)PC++);
    }
    im_pclos();
    thread;
INTERP_LABEL (polfi,1000001); /* POLYLENGTH == 1000001 */
    i = *(short *)PC++;
    im_pmvi(*PC++, *PC++, *PC++);
    while (--i != -1) {
	im_pdri(*PC++, *PC++, *PC++);
    }
    im_pclos();
    thread;
INTERP_LABEL (polfs,1000004); /* POLYLENGTH_S == 1000004 */
    i = *(short *)PC++;
    im_pmvs(*(short *)PC++, *(short *)PC++, *(short *)PC++);
    while (--i != -1) {
	im_pdrs(*(short *)PC++, *(short *)PC++, *(short *)PC++);
    }
    im_pclos();
    thread;

INTERP_LABEL (polf2,1000002); /* POLY2LENGTH == 1000002 */
    i = *(short *)PC++;
    im_pmv2(*(float *)PC++, *(float *)PC++);
    while (--i != -1) {
	im_pdr2(*(float *)PC++, *(float *)PC++);
    }
    im_pclos();
    thread;
INTERP_LABEL (polf2i,1000002); /* POLY2LENGTH == 1000002 */
    i = *(short *)PC++;
    im_pmv2i(*PC++, *PC++);
    while (--i != -1) {
	im_pdr2i(*PC++, *PC++);
    }
    im_pclos();
    thread;
INTERP_LABEL (polf2s,1000005); /* POLY2LENGTH_S == 1000005 */
    i = *(short *)PC++;
    im_pmv2s(*(short *)PC++, *(short *)PC++);
    while (--i != -1) {
	im_pdr2s(*(short *)PC++, *(short *)PC++);
    }
    im_pclos();
    thread;

INTERP_LABEL (poly,1000001); /* POLYLENGTH == 1000001 */
    i = *(short *)PC++;
    first_point = PC;
    im_move(*(float *)PC++, *(float *)PC++, *(float *)PC++);
    while (--i != -1) {
	im_draw(*(float *)PC++, *(float *)PC++, *(float *)PC++);
    }
    im_draw(*(float *)first_point++, *(float *)first_point++,
		*(float *)first_point++);
    thread;
INTERP_LABEL (polyi,1000001); /* POLYLENGTH == 1000001 */
    i = *(short *)PC++;
    first_point = PC;
    im_movei(*PC++, *PC++, *PC++);
    while (--i != -1) {
	im_drawi(*PC++, *PC++, *PC++);
    }
    im_drawi(*first_point++, *first_point++, *first_point++);
    thread;
INTERP_LABEL (polys,1000004); /* POLYLENGTH_S == 1000004 */
    i = *(short *)PC++;
    first_point = PC;
    im_moves(*(short *)PC++, *(short *)PC++, *(short *)PC++);
    while (--i != -1) {
	im_draws(*(short *)PC++, *(short *)PC++, *(short *)PC++);
    }
    im_draws(*(short *)first_point++, *(short *)first_point++,
    		*(short *)first_point++);
    thread;

INTERP_LABEL (poly2,1000002); /* POLY2LENGTH == 1000002 */
    i = *(short *)PC++;
    first_point = PC;
    im_move2(*(float *)PC++, *(float *)PC++);
    while (--i != -1) {
	im_draw2(*(float *)PC++, *(float *)PC++);
    }
    im_draw2(*(float *)first_point++, *(float *)first_point++);
    thread;
INTERP_LABEL (poly2i,1000002); /* POLY2LENGTH == 1000002 */
    i = *(short *)PC++;
    first_point = PC;
    im_move2i(*PC++, *PC++);
    while (--i != -1) {
	im_draw2i(*PC++, *PC++);
    }
    im_draw2i(*first_point++, *first_point++);
    thread;
INTERP_LABEL (poly2s,1000005); /* POLY2LENGTH_S == 1000005 */
    i = *(short *)PC++;
    first_point = PC;
    im_move2s(*(short *)PC++, *(short *)PC++);
    while (--i != -1) {
	im_draw2s(*(short *)PC++, *(short *)PC++);
    }
    im_draw2s(*(short *)first_point++, *(short *)first_point++);
    thread;
}
