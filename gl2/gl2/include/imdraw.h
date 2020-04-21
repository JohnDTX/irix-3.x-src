#ifndef  IMDRAWDEF
#define  IMDRAWDEF
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

/* macros to output 2d, 3d, 4d points of type S,I,F	*/
#define im_out2S(x,y)		im_outscoord(x);	\
				im_last_outscoord(y)
#define im_out2I(x,y)		im_outicoord(x);	\
				im_last_outicoord(y)
#define im_out2F(x,y)		im_outfloat(x);		\
				im_last_outfloat(y)
#define im_out3S(x,y,z)		im_outscoord(x);	\
				im_outscoord(y);	\
				im_last_outscoord(z)
#define im_out3I(x,y,z)		im_outicoord(x);	\
				im_outicoord(y);	\
				im_last_outicoord(z)
#define im_out3F(x,y,z)		im_outfloat(x); 	\
				im_outfloat(y); 	\
				im_last_outfloat(z)
#define im_out4S(x,y,z,w)	im_outscoord(x);	\
				im_outscoord(y);	\
				im_outscoord(z);	\
				im_last_outscoord(w)
#define im_out4I(x,y,z,w)	im_outicoord(x);	\
				im_outicoord(y);	\
				im_outicoord(z);	\
				im_last_outicoord(w)
#define im_out4F(x,y,z,w)	im_outfloat(x); 	\
				im_outfloat(y); 	\
				im_outfloat(z); 	\
				im_last_outfloat(w)

/* these macros are for Poly move/draw, they do not output to the LASTGE
port and thus will not be interrupted until someone outputs to LASTGE	*/

#define im_Pout2S(x,y)		im_outscoord(x);	\
				im_outscoord(y)
#define im_Pout2I(x,y)		im_outicoord(x);	\
				im_outicoord(y)
#define im_Pout2F(x,y)		im_outfloat(x);		\
				im_outfloat(y)
#define im_Pout3S(x,y,z)	im_outscoord(x);	\
				im_outscoord(y);	\
				im_outscoord(z)
#define im_Pout3I(x,y,z)	im_outicoord(x);	\
				im_outicoord(y);	\
				im_outicoord(z)
#define im_Pout3F(x,y,z)	im_outfloat(x); 	\
				im_outfloat(y); 	\
				im_outfloat(z)
#define im_Pout4S(x,y,z,w)	im_outscoord(x);	\
				im_outscoord(y);	\
				im_outscoord(z);	\
				im_outscoord(w)
#define im_Pout4I(x,y,z,w)	im_outicoord(x);	\
				im_outicoord(y);	\
				im_outicoord(z);	\
				im_outicoord(w)
#define im_Pout4F(x,y,z,w)	im_outfloat(x); 	\
				im_outfloat(y); 	\
				im_outfloat(z); 	\
				im_outfloat(w)

/* note that we count on the outzero macro to output 2 short 0's */
#define im_movezero()		{GEWAIT;im_outshort(GEmove | GEPA_2S);	\
					im_last_outzero;}
#define im_pmovezero()		{GEWAIT;im_outshort(GEmovepoly | GEPA_2S);\
					im_outzero;}

#define im_move(x,y,z)		{GEWAIT;im_outshort(GEmove | GEPA_3F); \
					im_out3F(x,y,z);}
#define im_move2(x,y)		{GEWAIT;im_outshort(GEmove | GEPA_2F); \
					im_out2F(x,y);}
#define im_movei(x,y,z)		{GEWAIT;im_outshort(GEmove | GEPA_3I); \
					im_out3I(x,y,z);}
#define im_move2i(x,y)		{GEWAIT;im_outshort(GEmove | GEPA_2I); \
					im_out2I(x,y);}
#define im_moves(x,y,z)		{GEWAIT;im_outshort(GEmove | GEPA_3S); \
					im_out3S(x,y,z);}
#define im_move2s(x,y)		{GEWAIT;im_outshort(GEmove | GEPA_2S); \
					im_out2S(x,y);}

#define im_draw(x,y,z)		{GEWAIT;im_outshort(GEdraw | GEPA_3F); \
					im_out3F(x,y,z);}
#define im_draw2(x,y)		{GEWAIT;im_outshort(GEdraw | GEPA_2F); \
					im_out2F(x,y);}
#define im_drawi(x,y,z)		{GEWAIT;im_outshort(GEdraw | GEPA_3I); \
					im_out3I(x,y,z);}
#define im_draw2i(x,y)		{GEWAIT;im_outshort(GEdraw | GEPA_2I); \
					im_out2I(x,y);}
#define im_draws(x,y,z)		{GEWAIT;im_outshort(GEdraw | GEPA_3S); \
					im_out3S(x,y,z);}
#define im_draw2s(x,y)		{GEWAIT;im_outshort(GEdraw | GEPA_2S); \
					im_out2S(x,y);}

#define im_pnt(x,y,z)		{GEWAIT;im_outshort(GEpoint | GEPA_3F); \
					im_out3F(x,y,z);}
#define im_pnt2(x,y)		{GEWAIT;im_outshort(GEpoint | GEPA_2F); \
					im_out2F(x,y);}
#define im_pnti(x,y,z)		{GEWAIT;im_outshort(GEpoint | GEPA_3I); \
					im_out3I(x,y,z);}
#define im_pnt2i(x,y)		{GEWAIT;im_outshort(GEpoint | GEPA_2I); \
					im_out2I(x,y);}
#define im_pnts(x,y,z)		{GEWAIT;im_outshort(GEpoint | GEPA_3S); \
					im_out3S(x,y,z);}
#define im_pnt2s(x,y)		{GEWAIT;im_outshort(GEpoint | GEPA_2S); \
					im_out2S(x,y);}

#define GExfpoint GEtransformpoint
#define im_xfpt(x,y,z)		{GEWAIT;im_outshort(GExfpoint | GEPA_3F); \
					im_out3F(x,y,z);}
#define im_xfpt2(x,y)		{GEWAIT;im_outshort(GExfpoint | GEPA_2F); \
					im_out2F(x,y);}
#define im_xfpti(x,y,z)		{GEWAIT;im_outshort(GExfpoint | GEPA_3I); \
					im_out3I(x,y,z);}
#define im_xfpt2i(x,y)		{GEWAIT;im_outshort(GExfpoint | GEPA_2I); \
					im_out2I(x,y);}
#define im_xfpts(x,y,z)		{GEWAIT;im_outshort(GExfpoint | GEPA_3S); \
					im_out3S(x,y,z);}
#define im_xfpt2s(x,y)		{GEWAIT;im_outshort(GExfpoint | GEPA_2S); \
					im_out2S(x,y);}
#define im_xfpt4(x,y,z,w)	{GEWAIT;im_outshort(GExfpoint | GEPA_4F); \
					im_out4F(x,y,z,w);}
#define im_xfpt4i(x,y,z,w)	{GEWAIT;im_outshort(GExfpoint | GEPA_4I); \
					im_out4I(x,y,z,w);}
#define im_xfpt4s(x,y,z,w)	{GEWAIT;im_outshort(GExfpoint | GEPA_4S); \
					im_out4S(x,y,z,w);}

#define im_pmv(x,y,z)		{GEWAIT;im_outshort(GEmovepoly | GEPA_3F); \
					im_Pout3F(x,y,z);}
#define im_pmv2(x,y)		{GEWAIT;im_outshort(GEmovepoly | GEPA_2F); \
					im_Pout2F(x,y);}
#define im_pmvi(x,y,z)		{GEWAIT;im_outshort(GEmovepoly | GEPA_3I); \
					im_Pout3I(x,y,z);}
#define im_pmv2i(x,y)		{GEWAIT;im_outshort(GEmovepoly | GEPA_2I); \
					im_Pout2I(x,y);}
#define im_pmvs(x,y,z)		{GEWAIT;im_outshort(GEmovepoly | GEPA_3S); \
					im_Pout3S(x,y,z);}
#define im_pmv2s(x,y)		{GEWAIT;im_outshort(GEmovepoly | GEPA_2S); \
					im_Pout2S(x,y);}

#define im_pdr(x,y,z)		{GEWAIT;im_outshort(GEdrawpoly | GEPA_3F); \
					im_Pout3F(x,y,z);}
#define im_pdr2(x,y)		{GEWAIT;im_outshort(GEdrawpoly | GEPA_2F); \
					im_Pout2F(x,y);}
#define im_pdri(x,y,z)		{GEWAIT;im_outshort(GEdrawpoly | GEPA_3I); \
					im_Pout3I(x,y,z);}
#define im_pdr2i(x,y)		{GEWAIT;im_outshort(GEdrawpoly | GEPA_2I); \
					im_Pout2I(x,y);}
#define im_pdrs(x,y,z)		{GEWAIT;im_outshort(GEdrawpoly | GEPA_3S); \
					im_Pout3S(x,y,z);}
#define im_pdr2s(x,y)		{GEWAIT;im_outshort(GEdrawpoly | GEPA_2S); \
					im_Pout2S(x,y);}

#define im_pclos()		{GEWAIT;im_last_outshort(GEclosepoly);}
#define im_spclos()		{GEWAIT; im_shade(1);		\
				im_outshort(GEclosepoly);	\
				im_shade(0);			\
				im_freepipe;}

#define im_cmov(x,y,z)		{GEWAIT; im_passcmd(1,FBCcharposnabs);	\
				im_outshort(GEpoint | GEPA_3F); \
				im_Pout3F(x,y,z);		\
				im_last_passthru(0);}
#define im_cmov2(x,y)		{GEWAIT; im_passcmd(1,FBCcharposnabs);	\
				im_outshort(GEpoint | GEPA_2F); \
				im_Pout2F(x,y);			\
				im_last_passthru(0);}
#define im_cmovi(x,y,z)		{GEWAIT; im_passcmd(1,FBCcharposnabs);	\
				im_outshort(GEpoint | GEPA_3I); \
				im_Pout3I(x,y,z); 		\
				im_last_passthru(0);}
#define im_cmov2i(x,y)		{GEWAIT; im_passcmd(1,FBCcharposnabs);	\
				im_outshort(GEpoint | GEPA_2I); \
				im_Pout2I(x,y);			\
				im_last_passthru(0);}
#define im_cmovs(x,y,z)		{GEWAIT; im_passcmd(1,FBCcharposnabs);	\
				im_outshort(GEpoint | GEPA_3S); \
				im_Pout3S(x,y,z); 		\
				im_last_passthru(0);}
#define im_cmov2s(x,y)		{GEWAIT; im_passcmd(1,FBCcharposnabs);	\
				im_outshort(GEpoint | GEPA_2S); \
				im_Pout2S(x,y);			\
				im_last_passthru(0);}

#define im_rmv(x,y,z)		{GEWAIT;im_outshort(GEmoverel | GEPA_3F); \
					im_out3F(x,y,z);}
#define im_rmv2(x,y)		{GEWAIT;im_outshort(GEmoverel | GEPA_2F); \
					im_out2F(x,y);}
#define im_rmvi(x,y,z)		{GEWAIT;im_outshort(GEmoverel | GEPA_3I); \
					im_out3I(x,y,z);}
#define im_rmv2i(x,y)		{GEWAIT;im_outshort(GEmoverel | GEPA_2I); \
					im_out2I(x,y);}
#define im_rmvs(x,y,z)		{GEWAIT;im_outshort(GEmoverel | GEPA_3S); \
					im_out3S(x,y,z);}
#define im_rmv2s(x,y)		{GEWAIT;im_outshort(GEmoverel | GEPA_2S); \
					im_out2S(x,y);}

#define im_rdr(x,y,z)		{GEWAIT;im_outshort(GEdrawrel | GEPA_3F); \
					im_out3F(x,y,z);}
#define im_rdr2(x,y)		{GEWAIT;im_outshort(GEdrawrel | GEPA_2F); \
					im_out2F(x,y);}
#define im_rdri(x,y,z)		{GEWAIT;im_outshort(GEdrawrel | GEPA_3I); \
					im_out3I(x,y,z);}
#define im_rdr2i(x,y)		{GEWAIT;im_outshort(GEdrawrel | GEPA_2I); \
					im_out2I(x,y);}
#define im_rdrs(x,y,z)		{GEWAIT;im_outshort(GEdrawrel | GEPA_3S); \
					im_out3S(x,y,z);}
#define im_rdr2s(x,y)		{GEWAIT;im_outshort(GEdrawrel | GEPA_2S); \
					im_out2S(x,y);}

#define im_rpmv(x,y,z)		{GEWAIT;im_outshort(GEmovepolyrel | GEPA_3F);\
					im_Pout3F(x,y,z);}
#define im_rpmv2(x,y)		{GEWAIT;im_outshort(GEmovepolyrel | GEPA_2F);\
					im_Pout2F(x,y);}
#define im_rpmvi(x,y,z)		{GEWAIT;im_outshort(GEmovepolyrel | GEPA_3I);\
					im_Pout3I(x,y,z);}
#define im_rpmv2i(x,y)		{GEWAIT;im_outshort(GEmovepolyrel | GEPA_2I);\
					im_Pout2I(x,y);}
#define im_rpmvs(x,y,z)		{GEWAIT;im_outshort(GEmovepolyrel | GEPA_3S);\
					im_Pout3S(x,y,z);}
#define im_rpmv2s(x,y)		{GEWAIT;im_outshort(GEmovepolyrel | GEPA_2S);\
					im_Pout2S(x,y);}

#define im_rpdr(x,y,z)		{GEWAIT;im_outshort(GEdrawpolyrel | GEPA_3F);\
					im_Pout3F(x,y,z);}
#define im_rpdr2(x,y)		{GEWAIT;im_outshort(GEdrawpolyrel | GEPA_2F);\
					im_Pout2F(x,y);}
#define im_rpdri(x,y,z)		{GEWAIT;im_outshort(GEdrawpolyrel | GEPA_3I);\
					im_Pout3I(x,y,z);}
#define im_rpdr2i(x,y)		{GEWAIT;im_outshort(GEdrawpolyrel | GEPA_2I);\
					im_Pout2I(x,y);}
#define im_rpdrs(x,y,z)		{GEWAIT;im_outshort(GEdrawpolyrel | GEPA_3S);\
					im_Pout3S(x,y,z);}
#define im_rpdr2s(x,y)		{GEWAIT;im_outshort(GEdrawpolyrel | GEPA_2S);\
					im_Pout2S(x,y);}

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* PLEASE leave rects drawn counter clockwise     */
/* 	it makes them visible when backface is on */
/*	hpm - Wed Sep 11 12:59:03 PDT 1985	  */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#define im_rectf(x1,y1,x2,y2)	{GEWAIT; im_pmv2(x1,y1);	\
				im_pdr2(x2,y1);			\
				im_pdr2(x2,y2);			\
				im_pdr2(x1,y2);			\
				im_pclos();}
#define im_rectfi(x1,y1,x2,y2)	{im_pmv2i(x1,y1);		\
				im_pdr2i(x2,y1);		\
				im_pdr2i(x2,y2);		\
				im_pdr2i(x1,y2);		\
				im_pclos();}
#define im_rectfs(x1,y1,x2,y2)	{im_pmv2s(x1,y1);		\
				im_pdr2s(x2,y1);		\
				im_pdr2s(x2,y2);		\
				im_pdr2s(x1,y2);		\
				im_pclos();}
#define im_rect(x1,y1,x2,y2)	{im_move2(x1,y1);		\
				im_draw2(x2,y1);		\
				im_draw2(x2,y2);		\
				im_draw2(x1,y2);		\
				im_draw2(x1,y1);}
#define im_recti(x1,y1,x2,y2)	{im_move2i(x1,y1);		\
				im_draw2i(x2,y1);		\
				im_draw2i(x2,y2);		\
				im_draw2i(x1,y2);		\
				im_draw2i(x1,y1);}
#define im_rects(x1,y1,x2,y2)	{im_move2s(x1,y1);		\
				im_draw2s(x2,y1);		\
				im_draw2s(x2,y2);		\
				im_draw2s(x1,y2);		\
				im_draw2s(x1,y1);}

#define im_curveit(n)	{GEWAIT; im_last_outshort(GEcurve | ((n)-2<<8));}

#define im_screenclear(llx,lly,urx,ury) {im_passcmd(5,FBCblockfill);	\
					im_outshort(llx);		\
					im_outshort(lly);		\
					im_outshort(urx);		\
					im_last_outshort(ury);}

#define im_clear() im_screenclear(WS->curvpdata.llx+WS->xmin,	\
				WS->curvpdata.lly+WS->ymin, 	\
				WS->curvpdata.urx+WS->xmin,	\
				WS->curvpdata.ury+WS->ymin)

#define im_zclear()    if (!((WS->curatrdata.myconfig) & (UC_DOUBLE<<16))) {\
			    im_passcmd(5,FBCwrten);	\
			    im_outshort(0);		\
			    im_outshort(FBCcdcolorwe); \
			    im_outlong(0x7fffffff);	\
			    im_clear(); \
			    im_passcmd(2,FBCwrten);	\
			    im_last_outshort(WS->curatrdata.mywenable); \
			} else { \
			    im_frontbuffer(0);\
			    im_do_color(0x7fff);\
			    im_clear();\
			    im_frontbuffer(1);\
			}

#define im_loadname(name)   if (gl_fbmode) {	\
				im_passcmd(2,FBCloadname);	\
				im_last_outshort(name);	\
			    }	\
			    else name

#define im_pushname(name)   if (gl_fbmode) {	\
				im_passcmd(2,FBCpushname);\
				im_last_outshort(name);	\
			    }	\
			    else name

#define im_popname()	if (gl_fbmode) {		\
				im_last_passcmd(1,FBCpopname);\
			}

#define im_initnames()	if (gl_fbmode) {		\
				im_last_passcmd(1,FBCinitnamestack);\
			}
#endif  IMDRAWDEF
