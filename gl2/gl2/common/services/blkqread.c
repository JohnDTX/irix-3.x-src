
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
#include "gl.h"

long blkqread(buff,cnt)
register short *buff;
register int cnt;
{
    register int i;

    cnt >>= 1;
    for(i=0; i<cnt; i++) {
	*buff = qread(buff+1);
	buff += 2;
	if(!qtest()) {
	    i++;
	    break;
	}
    }
    return (i<<1);
}
