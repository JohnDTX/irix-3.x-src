#ifndef  IMMATRIXDEF
#define  IMMATRIXDEF
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
#include "imsetup.h"

/* if top of stack is clean, then flag the top make it dirty	*/
#define im_dirty_matrixstack				\
	im_lockpipe;					\
	if (WS->softstacktop == WS->matrixlevel) {	\
	    WS->softstacktop--;				\
	    WS->matrixstatep++;				\
	}

#define im_scale(x,y,z)		{im_dirty_matrixstack; im_do_scale(x,y,z);}
#define im_do_scale(x,y,z)	{GEWAIT; im_outshort(GEfirstmm0 | GEPA_2F); \
				im_outfloat(x); 			\
				im_outzero; 				\
				im_outshort(GEmidmm1 | GEPA_4F);	\
				im_outzero; 				\
				im_outfloat(y); 			\
				im_outzero; 				\
				im_outzero; 				\
				im_outshort(GElastmm2 | GEPA_4F);	\
				im_outzero; 				\
				im_outzero; 				\
				im_outfloat(z); 			\
				im_last_outzero;}

#define im_scalei(x,y,z)	{im_dirty_matrixstack; im_do_scalei(x,y,z);}
#define im_do_scalei(x,y,z)	{GEWAIT; im_outshort(GEfirstmm0 | GEPA_2I); \
				im_outicoord(x); 			\
				im_outzero; 				\
				im_outshort(GEmidmm1 | GEPA_4I);	\
				im_outzero; 				\
				im_outicoord(y); 			\
				im_outzero; 				\
				im_outzero; 				\
				im_outshort(GElastmm2 | GEPA_4I);	\
				im_outzero; 				\
				im_outzero; 				\
				im_outicoord(z); 			\
				im_last_outzero;}

#define im_scales(x,y,z)	{im_dirty_matrixstack; im_do_scales(x,y,z);}
#define im_do_scales(x,y,z)	{GEWAIT; im_outshort(GEfirstmm0 | GEPA_2S); \
				im_outscoord(x); 			\
				im_outshortzero; 			\
				im_outshort(GEmidmm1 | GEPA_4S);	\
				im_outshortzero;			\
				im_outicoord(y); 			\
				im_outzero; 				\
				im_outshort(GElastmm2 | GEPA_4I);	\
				im_outzero; 				\
				im_outscoord(z); 			\
				im_last_outshortzero;}

#define im_scale2(x,y)		{im_dirty_matrixstack; im_do_scale2(x,y);}
#define im_do_scale2(x,y)	{GEWAIT; im_outshort(GEfirstmm0 | GEPA_2F); \
				im_outfloat(x); 			\
				im_outzero; 				\
				im_outshort(GElastmm1 | GEPA_4F); 	\
				im_outzero; 				\
				im_outfloat(y); 			\
				im_outzero; 				\
				im_last_outzero;}

#define im_scale2i(x,y)		{im_dirty_matrixstack; im_do_scale2i(x,y);}
#define im_do_scale2i(x,y)	{GEWAIT; im_outshort(GEfirstmm0 | GEPA_2I); \
				im_outicoord(x); 			\
				im_outzero; 				\
				im_outshort(GElastmm1 | GEPA_4I);	\
				im_outzero; 				\
				im_outicoord(y);			\
				im_outzero;				\
				im_last_outzero;}

#define im_scale2s(x,y)		{im_dirty_matrixstack; im_do_scale2s(x,y);}
#define im_do_scale2s(x,y)	{GEWAIT; im_outshort(GEfirstmm0 | GEPA_2S); \
				im_outscoord(x); 			\
				im_outshortzero;			\
				im_outshort(GElastmm1 | GEPA_4S);	\
				im_outshortzero;			\
				im_outscoord(y);			\
				im_last_outzero;}

#define im_translate(x,y,z)	{im_dirty_matrixstack;		\
				im_do_translate(x,y,z);}
#define im_translatei(x,y,z)	{im_dirty_matrixstack;		\
				im_do_translatei(x,y,z);}
#define im_translates(x,y,z)	{im_dirty_matrixstack;		\
				im_do_translates(x,y,z);}
#define im_do_translate(x,y,z)	{GEWAIT;im_outshort(GEcompletemm3 | GEPA_4F);\
				im_outfloat(x);				\
				im_outfloat(y);				\
				im_outfloat(z);				\
				im_last_outone;}
#define im_do_translatei(x,y,z)	{GEWAIT;im_outshort(GEcompletemm3 | GEPA_4I);\
				im_outicoord(x);	\
				im_outicoord(y);	\
				im_outicoord(z);	\
				im_last_outicoord(1);}
#define im_do_translates(x,y,z)	{GEWAIT;im_outshort(GEcompletemm3 | GEPA_4S);\
				im_outscoord(x);	\
				im_outscoord(y);	\
				im_outscoord(z);	\
				im_last_outscoord(1);}

/* The parameters to the next three rotate commands take as parameters
 * the sine and cosine of the angle of rotation.
 */

#define im_rotatex(cos,sin,sin1,cos1){im_dirty_matrixstack;		\
				GEWAIT; im_outshort(GEfirstmm1 | GEPA_3F); \
				im_outzero; 				\
				im_outfloat(cos); 			\
				im_outfloat(sin); 			\
				GEWAIT; im_outshort(GElastmm2 | GEPA_4F); \
				im_outzero; 				\
				im_outfloat(sin1); 			\
				im_outfloat(cos1); 			\
				im_last_outzero;}

#define im_rotatey(cos,sin,sin1,cos1){im_dirty_matrixstack;		\
				GEWAIT; im_outshort(GEfirstmm0 | GEPA_3F); \
				im_outfloat(cos); 			\
				im_outzero; 				\
				im_outfloat(sin); 			\
				GEWAIT; im_outshort(GElastmm2 | GEPA_4F); \
				im_outfloat(sin1); 			\
				im_outzero; 				\
				im_outfloat(cos1); 			\
				im_last_outzero;}

#define im_rotatez(cos,sin,sin1,cos1){im_dirty_matrixstack;		\
				GEWAIT; im_outshort(GEfirstmm0 | GEPA_2F); \
				im_outfloat(cos); 			\
				im_outfloat(sin); 			\
				GEWAIT; im_outshort(GElastmm1 | GEPA_4F); \
				im_outfloat(sin1); 			\
				im_outfloat(cos1); 			\
				im_outzero; 				\
				im_last_outzero;}

#ifdef KERNEL
#define im_pushmatrix()		{GEWAIT; im_last_outshort(GEpushmm);}
#define im_popmatrix()		{GEWAIT; im_last_outshort(GEpopmm);}
#else
#define im_pushmatrix()	{				\
    if (MATRIXSTACKDEPTH-1 <= WS -> matrixlevel) {	\
	gl_ErrorHandler (ERR_PUSHMATRIX,WARNING,NULL);	\
    } else {						\
	im_lockpipe;					\
	if (WS->hdwrstackbottom + 7 == WS->matrixlevel) {\
	    if (WS->softstacktop + 8 == WS->matrixlevel)\
		gl_hardoverflow ();			\
	    else im_outshort (GEpushmm);		\
	    WS->hdwrstackbottom++;			\
	}						\
	else im_outshort (GEpushmm);			\
	WS->matrixlevel++;				\
	im_freepipe;					\
    }							\
}

#define im_popmatrix()	{				\
    if (WS -> matrixlevel == 0) {			\
	gl_ErrorHandler (ERR_POPMATRIX,WARNING,NULL);	\
    } else {						\
	im_lockpipe;					\
	if (WS->softstacktop == WS->matrixlevel) {	\
	    WS->softstacktop--;				\
	    WS->matrixstatep++;				\
	}						\
	if (WS->hdwrstackbottom == WS->matrixlevel)	\
	    gl_hardunderflow ();			\
	else im_outshort(GEpopmm);			\
	WS->matrixlevel--;				\
	im_freepipe;					\
    }							\
}
#endif

#define im_do_loadmatrix(m) {	register float *_f = (float *)(m);\
				GEWAIT; im_outshort(GEloadmm); 	\
				im_outfloat(_f[0]); 		\
				im_outfloat(_f[4]); 		\
				im_outfloat(_f[8]); 		\
				im_outfloat(_f[12]); 		\
				GEWAIT; im_outfloat(_f[1]); 	\
				im_outfloat(_f[5]); 		\
				im_outfloat(_f[9]); 		\
				im_outfloat(_f[13]); 		\
				GEWAIT; im_outfloat(_f[2]); 	\
				im_outfloat(_f[6]); 		\
				im_outfloat(_f[10]); 		\
				im_outfloat(_f[14]); 		\
				GEWAIT; im_outfloat(_f[3]); 	\
				im_outfloat(_f[7]); 		\
				im_outfloat(_f[11]); 		\
				im_last_outfloat(_f[15]);	\
			    }

/* If we are in picking mode, we have to load the `funny matrix'
	first, and then turn the load into a matrix multiply	*/
#define im_loadmatrix(m)  {	im_dirty_matrixstack;		\
				if (gl_fbmode && gl_picking) {		\
				    im_do_loadmatrix (gl_pickmat);\
				    im_multmatrix (m);		\
				}				\
				else im_do_loadmatrix (m);	\
			    }

#define im_multmatrix(x)	{ register float *_f = (float *)(x); \
				im_dirty_matrixstack;		\
				GEWAIT; im_outshort(GEfirstmm0);\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++);  		\
				GEWAIT; im_outshort(GEmidmm1); 	\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				GEWAIT; im_outshort(GEmidmm2);	\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				GEWAIT; im_outshort(GElastmm3);	\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_last_outfloat(*_f); }
#endif  IMMATRIXDEF
