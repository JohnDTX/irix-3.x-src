#ifndef  IMSETUPDEF
#define  IMSETUPDEF
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
#include	"gf2.h"
#include	"gl2cmds.h"

extern long gl_currentshade;

#define GEONE		0x3f800000
#define GEN_ONE		GEONE

#define	GEPA_2D		0x100
#define	GEPA_3D		0x200
#define	GEPA_4D		0x000
#define	GEPA_I		0x400
#define	GEPA_S		0x800
#define	GEPA_F		0x000
#define GEPA_2I		(GEPA_2D | GEPA_I)
#define GEPA_2S		(GEPA_2D | GEPA_S)
#define GEPA_2F		(GEPA_2D | GEPA_F)
#define GEPA_3I		(GEPA_3D | GEPA_I)
#define GEPA_3S		(GEPA_3D | GEPA_S)
#define GEPA_3F		(GEPA_3D | GEPA_F)
#define GEPA_4I		(GEPA_4D | GEPA_I)
#define GEPA_4S		(GEPA_4D | GEPA_S)
#define GEPA_4F		(GEPA_4D | GEPA_F)

#ifdef KERNEL
#define	im_setup	register unsigned short *GE = &GEPORT
#define im_cleanup
#define GEWAIT
#else
#ifdef CLOVER
#define	im_setup	register windowstate *WS = gl_wstatep
#else
#define	im_setup	register unsigned short *GE = &GEPORT;	\
			register windowstate *WS = gl_wstatep
#endif
#define im_cleanup
#define GEWAIT
#endif KERNEL

#ifdef CLOVER
#define im_outicoord(x)	im_outlong(x)
#define im_outscoord(x)	im_outshort(x)
#define im_outone	im_outlong(GEONE)
#define im_outn_one	im_outlong(GEN_ONE)
#define im_outzero	im_outlong(0)
#define im_outshortzero	im_outshort(0)
#define im_passthru(n)	im_outshort(GEpassthru|(((n)-1)<<8))
#define im_passcmd(n,cmd) im_outlong(((GEpassthru|(((n)-1)<<8))<<16)|(cmd))
#define im_last_outicoord(x)	im_last_outlong(x)
#define im_last_outscoord(x)	im_last_outshort(x)
#define im_last_outone		im_last_outlong(GEONE)
#define im_last_outn_one	im_last_outlong(GEN_ONE)
#define im_last_outzero		im_last_outlong(0)
#define im_last_outshortzero	im_last_outshort(0)
#define im_last_passthru(n)	im_last_outshort(GEpassthru|(((n)-1)<<8))
#define im_last_passcmd(n,cmd) im_last_outlong(((GEpassthru|(((n)-1)<<8))<<16)|(cmd))
#else
#define im_outshort(x)	*(short *)GE = (x)
#define im_outlong(x)	*(long *)GE = (x)
#define im_outicoord(x)	*(long *)GE = (x)
#define im_outscoord(x)	*(short *)GE = (x)
#define im_outfloat(x)	*(float *)GE = (x)
#define im_outone	*(long *)GE = GEONE
#define im_outn_one	*(long *)GE = GEN_ONE
#define im_outzero	*(long *)GE = 0
#define im_outshortzero	*(short *)GE = 0
#define im_passthru(n)	*(short *)GE = GEpassthru|(((n)-1)<<8)
#define	im_passcmd(n,cmd) *(long *)GE = (((GEpassthru|(((n)-1)<<8))<<16)|(cmd))

#define LASTGE		(GE-0x800)

#define im_last_outshort(x)	*(short *)LASTGE = (x)
#define im_last_outlong(x)	*(long *)LASTGE = (x)
#define im_last_outscoord(x)	*(short *)LASTGE = (x)
#define im_last_outicoord(x)	*(long *)LASTGE = (x)
#define im_last_outfloat(x)	*(float *)LASTGE = (x)
#define im_last_outone		*(long *)LASTGE = GEONE
#define im_last_outn_one	*(long *)LASTGE = GEN_ONE
#define im_last_outzero		*(long *)LASTGE = 0
#define im_last_outshortzero	*(short *)LASTGE = 0
#define im_last_passthru(n)	*(short *)LASTGE = GEpassthru|(((n)-1)<<8)
#define	im_last_passcmd(n,cmd) \
		*(long *)LASTGE = (((GEpassthru|(((n)-1)<<8))<<16) | (cmd))
#endif

#define	im_freepipe		im_last_passthru(0)
#define	im_lockpipe		im_passthru(0)
#define im_passthrough(s) {im_passthru(1); im_outshort(s);}

#define im_shade(b)	{gl_currentshade = (b);		\
			im_passcmd(3,FBCdrawmode);	\
			im_outshort(b);			\
			im_outshort(WS -> myzbuffer);}

#define im_zshade(b,m)	{im_passcmd(3,FBCdrawmode);	\
			im_outshort(b);			\
			im_outshort(m);}

#endif IMSETUPDEF
