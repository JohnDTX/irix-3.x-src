#ifndef GETDEF	/* Release 2.3 */
#define GETDEF
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


/* include file containing definitions for returned values of get* routines */


/* values returned by getbuffer() */

#define NOBUFFER	0
#define BCKBUFFER	1
#define FRNTBUFFER	2
#define BOTHBUFFERS	3

/* values returned by getcmmode() */

#define CMAPMULTI	0
#define CMAPONE		1

/* values returned by getdisplaymode() */

#define DMRGB		0
#define DMSINGLE	1
#define DMDOUBLE	2

/* values returned by getmonitor() and getothermonitor() */

#define HZ30		0
#define HZ60		1
#define NTSC		2
#define HZ50		3
#define MONA		5
#define MONB		6
#define MONC		7
#define MOND		8
#define PAL		9
#define MONSPECIAL	0x20

/* individual hit bits returned by gethitcode() */

#define LEFTPLANE	0x0001
#define RIGHTPLANE	0x0002
#define BOTTOMPLANE	0x0004
#define TOPPLANE	0x0008
#define NEARPLANE	0x0010
#define FARPLANE	0x0020

#endif GETDEF
