/**************************************************************************
 *									  *
 * 		 Copyright (C) 1983, Silicon Graphics, Inc.		  *
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

/* individual hit bits returned by gethitcode() */

#define FARPLANE	0x0001
#define NEARPLANE	0x0002
#define TOPPLANE	0x0004
#define BOTTOMPLANE	0x0008
#define RIGHTPLANE	0x0010
#define LEFTPLANE	0x0020
