#ifndef  CURVMACDEF
#define  CURVMACDEF
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

/* NON-GL macros, no GEPA optimizations performed	*/

#define max(x,y)	((x) > (y) ? (x) : (y))

#ifdef UNDEF
#define im_loadgeommatrix(m)	{ register float *_f=(float *)(m); 	\
				im_dirty_matrixstack;		\
				GEWAIT; im_outshort(GEloadmm); 		\
				im_outfloat(_f[0]); 		\
				im_outfloat(_f[3]); 		\
				im_outfloat(_f[6]); 		\
				im_outfloat(_f[9]); 		\
				GEWAIT; im_outfloat(_f[1]); 		\
				im_outfloat(_f[4]); 		\
				im_outfloat(_f[7]); 		\
				im_outfloat(_f[10]); 		\
				GEWAIT; im_outfloat(_f[2]); 		\
				im_outfloat(_f[5]); 		\
				im_outfloat(_f[8]); 		\
				im_outfloat(_f[11]); 		\
				GEWAIT; im_outfloat(1.0);		\
				im_outfloat(1.0); 			\
				im_outfloat(1.0); 			\
				im_outfloat(1.0); }
#endif

#define im_loadunit()	{ 	im_dirty_matrixstack;		\
				GEWAIT; im_outshort(GEloadmm); 		\
				im_outfloat(1.0); 		\
				im_outzero; 		\
				im_outzero; 		\
				im_outzero; 		\
				GEWAIT; im_outzero; 		\
				im_outfloat(1.0); 		\
				im_outzero; 		\
				im_outzero; 		\
				GEWAIT; im_outzero; 		\
				im_outzero; 		\
				im_outfloat(1.0); 		\
				im_outzero; 		\
				GEWAIT; im_outzero;		\
				im_outzero; 		\
				im_outzero; 		\
				im_outfloat(1.0); }

#ifdef UNDEF
#define im_restoregeommatrix(m) { register float *_f=(float *)(m); \
				im_dirty_matrixstack;		\
				GEWAIT; im_outshort(GEloadmm); 		\
				im_outfloat(*_f++);		\
				im_outfloat(*_f++);		\
				im_outfloat(*_f++);		\
				im_outfloat(*_f++);		\
				GEWAIT; im_outfloat(*_f++);		\
				im_outfloat(*_f++);		\
				im_outfloat(*_f++);		\
				im_outfloat(*_f++);		\
				GEWAIT; im_outfloat(*_f++);		\
				im_outfloat(*_f++);		\
				im_outfloat(*_f++);		\
				im_outfloat(*_f);		\
				GEWAIT; im_outfloat(1.0);		\
				im_outfloat(1.0);			\
				im_outfloat(1.0);			\
				im_outfloat(1.0);}
#endif

#define im_restorematrix(m) { register float *_f = (float *)(m); 	\
				im_dirty_matrixstack;		\
				GEWAIT; im_outshort(GEloadmm); 		\
				im_outfloat(*_f++);		\
				im_outfloat(*_f++);		\
				im_outfloat(*_f++);		\
				im_outfloat(*_f++);		\
				GEWAIT; im_outfloat(*_f++);		\
				im_outfloat(*_f++);		\
				im_outfloat(*_f++);		\
				im_outfloat(*_f++);		\
				GEWAIT; im_outfloat(*_f++);		\
				im_outfloat(*_f++);		\
				im_outfloat(*_f++);		\
				im_outfloat(*_f++);		\
				GEWAIT; im_outfloat(*_f++);		\
				im_outfloat(*_f++);		\
				im_outfloat(*_f++);		\
				im_outfloat(*_f);}

#ifdef UNDEF

#define im_geommultmatrix(x)	{ register float *_f=(float *)(x);  \
				im_dirty_matrixstack;		\
				GEWAIT; im_outshort(GEfirstmm0);        \
				im_outfloat(*_f++);		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outone; 			\
				GEWAIT; im_outshort(GEmidmm1); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outone; 			\
				GEWAIT; im_outshort(GEmidmm2); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outone; 			\
				GEWAIT; im_outshort(GElastmm3); 	\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f); 		\
				im_outone; }
#endif


#define im_itermultmatrix(x,y,z,w)  { 				\
				im_dirty_matrixstack;		\
				GEWAIT; im_outshort(GEfirstmm0); 	\
				im_outfloat(*x++); 		\
				im_outfloat(*y++); 		\
				im_outfloat(*z++); 		\
				im_outfloat(*w++);		\
				GEWAIT; im_outshort(GEmidmm1); 		\
				im_outfloat(*x++); 		\
				im_outfloat(*y++); 		\
				im_outfloat(*z++); 		\
				im_outfloat(*w++);		\
				GEWAIT; im_outshort(GEmidmm2); 		\
				im_outfloat(*x++); 		\
				im_outfloat(*y++); 		\
				im_outfloat(*z++); 		\
				im_outfloat(*w++);		\
				GEWAIT; im_outshort(GElastmm3); 	\
				im_outfloat(*x++); 		\
				im_outfloat(*y++); 		\
				im_outfloat(*z++); 		\
				im_outfloat(*w++); }

#define im_multprecmatrix(x)	{ register float *_f=(float *)(x); 	\
				im_dirty_matrixstack;		\
				GEWAIT; im_outshort(GEfirstmm0); 	\
				im_outfloat(*_f++); 		\
				im_outzero; 			\
				im_outzero; 			\
				im_outzero; 			\
				GEWAIT; im_outshort(GEmidmm1); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outzero; 			\
				im_outzero; 			\
				GEWAIT; im_outshort(GElastmm2); 	\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f); 		\
				im_outzero;}
#endif CURVMACDEF
