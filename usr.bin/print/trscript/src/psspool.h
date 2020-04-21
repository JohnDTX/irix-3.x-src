/* psspool.h
 *
 * Copyright (c) 1985 Adobe Systems Incorporated
 *
 * nice constants for printcap spooler filters
 *
 * RCSID: $Header: /d2/3.7/src/usr.bin/print/trscript/src/RCS/psspool.h,v 1.1 89/03/27 18:20:56 root Exp $
 *
 * RCSLOG:
 * $Log:	psspool.h,v $
 * Revision 1.1  89/03/27  18:20:56  root
 * Initial check-in for 3.7
 * 
 * Revision 1.1  86/11/17  20:24:27  root
 * Initial revision
 * 
 * Revision 2.1  85/11/24  11:51:12  shore
 * Product Release 2.0
 * 
 * Revision 1.2  85/05/14  11:26:54  shore
 * 
 * 
 *
 */

#define PS_INT	'\003'
#define PS_EOF	'\004'
#define PS_STATUS '\024'

/* error exit codes for lpd-invoked processes */
#define TRY_AGAIN 1
#define THROW_AWAY 2

