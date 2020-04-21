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

#include "gl.h"
#define even(A) (!(A%2))

main()
{
	int j;
	ginit();
	cursoff();
	color(BLACK);
	clear();
	color(BLUE);

	/* move to center of the screen */
	move2i(XMAXSCREEN/2,YMAXSCREEN/2);
	for (j=1; j<500; j=j+5) {
		if (even(j)) {
		    rdr2i(j,0);
		    rdr2i(0,j);
		}
		else  {
		    rdr2i(-j,0);
		    rdr2i(0,-j);
		}
	}
	gexit();
}

