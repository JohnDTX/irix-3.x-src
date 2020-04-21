#ifndef  IMATTRIBDEF
#define  IMATTRIBDEF
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
extern unsigned short gl_findlinestyle(), gl_findpattern();
extern fontrec *gl_findfont();

#define im_outconfig() {		\
	GEWAIT; im_passcmd(3,FBCconfig); \
	if(gl_shmemptr->isblanked)	\
	im_last_outlong(WS->curatrdata.myconfig & ~(DISPLAYA | DISPLAYB)); \
	else				\
	im_last_outlong(WS->curatrdata.myconfig);}

#define im_outfontbase(addr) {				\
	GEWAIT; im_passcmd(2,FBCbaseaddress);		\
	im_outshort(addr);}			/* KEEP IT THIS WAY */

#define im_setshade(i)	{im_passcmd(2,FBCsetintensity);	\
			im_outshort(i);			\
			}

#define im_color(c) {register Colorindex _col = (c);	\
	_col &= WS -> bitplanemask;			\
	WS->curatrdata.mycolor = _col;			\
	im_do_color(_col);				\
    }

#define im_writemask(wtm) {register Colorindex _wtm = (wtm);	\
	_wtm &= WS -> bitplanemask;				\
	WS->curatrdata.mywenable = _wtm;			\
	im_do_writemask(_wtm);					\
    }

#define im_do_color(c) {			\
	GEWAIT; im_passcmd(2,FBCcolor);		\
	im_last_outshort(c);			\
    }

#define im_buffcopy(c) {			\
	GEWAIT; im_passcmd(2,FBCbuffcopy);	\
	im_last_outshort(c);			\
    }

#define im_do_writemask(wtm) {			\
	GEWAIT; im_passcmd(2,FBCwrten);		\
	im_last_outshort(wtm);			\
    }

#define im_RGBcolor(r,g,b) {GEWAIT; im_passcmd(4,FBCrgbcolor);		\
				im_outshort(WS -> curatrdata.myr = (r));\
				im_outshort(WS -> curatrdata.myg = (g));\
				im_last_outshort(WS -> curatrdata.myb = (b));}
#define im_RGBwritemask(r,g,b) {GEWAIT; im_passcmd(4,FBCrgbwrten);	\
				im_outshort(WS -> curatrdata.myrm = (r));\
				im_outshort(WS -> curatrdata.mygm = (g));\
				im_last_outshort(WS -> curatrdata.mybm = (b));}

#define im_font(n)	WS->curatrdata.currentfont = gl_findfont(n)

#define im_lsrepeat(r)		{GEWAIT; im_passcmd(3,FBClinestipple);	\
				WS->curatrdata.mylsrepeat = (r) - 1; \
				im_outshort(WS->curatrdata.mylsrepeat);	\
				im_last_outshort(WS->mylscode);	\
				}

#define im_setlinestyle(lstyle)	{register int _ls = (lstyle);	\
				GEWAIT; im_passcmd(3,FBClinestipple);	\
				WS->curatrdata.mylstyle = _ls;	\
				WS->mylscode = gl_findlinestyle(_ls);	\
				im_outshort(WS->curatrdata.mylsrepeat);	\
				im_last_outshort(WS->mylscode);	\
				}

#define im_linewidth(w)	{register short _wid = (w);		\
			if(_wid <= 0) _wid = 1;			\
			else if(_wid > 1024) _wid = 1024;	\
			GEWAIT; im_passcmd(2,FBClinewidth);	\
			WS->curatrdata.mylwidth = --_wid;	\
			im_last_outshort(_wid);			\
			}

/* set load line stipple bit, then draw a line offscreen just to the left of
	the current char viewport, this forces a load of the line stipple;
	finally, set the load line stipple bit to desired value		*/
#define im_resetls(b)	if(gl_picking) b;			\
			else {					\
			WS->curatrdata.myconfig |= LDLINESTIP;	\
			GEWAIT; im_passcmd(9,FBCconfig);	\
			if(gl_shmemptr->isblanked)	\
			im_outlong(WS->curatrdata.myconfig & ~(DISPLAYA | DISPLAYB)); \
			else				\
			im_outlong(WS->curatrdata.myconfig);	\
			WS->curvpdata.llx -= 0x100;		\
			GEWAIT; im_outshort(FBCmove);		\
			im_outshort(WS->curvpdata.llx>>8);	\
			im_outshort(WS->curvpdata.lly>>8);	\
			GEWAIT; im_outshort(FBCdraw);		\
			im_outshort(WS->curvpdata.llx>>8);	\
			im_outshort(WS->curvpdata.lly>>8);	\
			WS->curvpdata.llx += 0x100;		\
			if (b) WS->curatrdata.myconfig |= LDLINESTIP;	\
			else WS->curatrdata.myconfig &= ~LDLINESTIP;	\
			im_outconfig();				\
			}

#define im_lsbackup(bk)	{if (bk) WS->curatrdata.myconfig |= BACKLINE;	\
			else WS->curatrdata.myconfig &= ~BACKLINE;	\
			im_outconfig();				\
			}

/* NOTE: findpattern modifies myconfg	*/
#define im_setpattern(t) {register int _tex = (t);		\
			GEWAIT; im_passcmd(2,FBCpolystipple);	\
			WS->curatrdata.mytexture = _tex;	\
	    WS->mytexcode = gl_findpattern(_tex,&WS->curatrdata.myconfig);\
			im_last_outshort(WS->mytexcode);	\
			im_outconfig();			\
			}

#define im_frontbuffer(bool)						\
if (WS->curatrdata.myconfig & (UC_DOUBLE<<16)) {			\
    if (bool)								\
	WS->curatrdata.myconfig |= WS->curatrdata.myconfig & DISPLAYA ?	\
					UPDATEA : UPDATEB; 		\
    else WS->curatrdata.myconfig &= ~(WS->curatrdata.myconfig & DISPLAYA ? \
					UPDATEA : UPDATEB);		\
    im_outconfig ();							\
} else									\
    bool;			/* Take that Tarolli!!  CCR */

#define im_backbuffer(bool)						\
if (WS->curatrdata.myconfig & (UC_DOUBLE<<16)) {			\
    if (bool)								\
	WS->curatrdata.myconfig |= WS->curatrdata.myconfig & DISPLAYB ?	\
					UPDATEA : UPDATEB;	 	\
    else WS->curatrdata.myconfig &= ~(WS->curatrdata.myconfig & DISPLAYB ? \
					UPDATEA : UPDATEB);		\
    im_outconfig ();							\
} else									\
    bool;			/* Take that Tarolli!!  CCR */

#define im_backface(b)	{im_passcmd(2,FBCsetbackfacing);	\
			im_last_outshort(WS -> mybackface = (b));	\
			}
#endif  IMATTIBDEF
