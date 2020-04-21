/*
 * 4.2 compatability system calls for System V
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/sys/sys/RCS/compat.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:07 $
 */
#include "../h/param.h"
#include "../h/user.h"

getpagesize()
{
	u.u_rval1 = NBPG;
}

getdtablesize()
{
	u.u_rval1 = NOFILE;
}
