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

#include "globals.h"

gl_initallstackstuff()
{
    register windowstate *ws = gl_wstatep;

    ws->attribstatep = ws->attribstack;	/* attribute stack	*/
    ws->vpstatep = ws->vpstack;		/* viewport stack	*/

    /* matrix stack ordered backwards */
    ws->matrixstatep = ws->matrixstack + (MATRIXSTACKDEPTH - 1);
    ws->matrixlevel = 0;
    ws->softstacktop = -1;
    ws->hdwrstackbottom = 0;

    /* This stuff prob'ly belongs elsewhere */
    ws->curvpdata.vcs = ws->curvpdata.vss = 0;
}
