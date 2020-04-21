/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *	4/3/84 - hpm - changed to start generating ids starting at
 *		       0x7fffffff the largest 32 bit pos integer
 *			it formerly started at 1
 *									  *
 **************************************************************************/

#include "gl.h"

Object	genobj()
{
    static Object nextobj = 0x7fffffff;

    while(isobj(nextobj))
	nextobj--;
    return(nextobj--);
}
