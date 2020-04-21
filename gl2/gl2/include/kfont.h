#ifndef KFONTDEF
#define KFONTDEF
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

/*
 * Structure containing font info
 */
struct	fontchar {
	ushort	offset;
	char	w, h;
	char	xoff, yoff;
	short	width;
};

/*
 * this is a "parallel" struct defined to make font loading easier
 * and must be the same size as struct fontchar
 */
struct	ufontchar {
	long l1, l2;
};

extern struct fontchar defont_font[];
extern short defont_bits[];

extern int defont_nr, defont_nc;
extern int max_defont_nr, min_defont_nr;

#define FR_DEFPATTERN	0	/* don't ever ever move this! */
#define FR_DEFCURSOR	16
#define FR_DEFFONT	32
#define FR_MAXKFONTSIZE	15000

#endif KFONTDEF
