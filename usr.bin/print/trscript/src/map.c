#ifndef lint
static char Notice[] = "Copyright (c) 1985 Adobe Systems Incorporated";
static char *RCSID="$Header: /d2/3.7/src/usr.bin/print/trscript/src/RCS/map.c,v 1.1 89/03/27 18:20:40 root Exp $";
#endif
/* map.c
 *
 * Copyright (c) 1985 Adobe Systems Incorporated
 *
 * front end to mapname -- font mapping for users
 *
 * for non-4.2bsd systems (e.g., System V) which do not
 * allow long Unix file names
 *
 * RCSLOG:
 * $Log:	map.c,v $
 * Revision 1.1  89/03/27  18:20:40  root
 * Initial check-in for 3.7
 * 
 * Revision 1.1  86/11/17  20:21:26  root
 * Initial revision
 * 
 * Revision 2.1  85/11/24  11:49:13  shore
 * Product Release 2.0
 * 
 * Revision 1.1  85/11/20  00:14:39  shore
 * Initial revision
 * 
 *
 */

#include <stdio.h>
#include "transcript.h"

main(argc,argv)
int argc;
char **argv;
{
    char result[128];

    if (argc != 2) exit(1);
    if (mapname(argv[1],result) == NULL) exit(1);

    printf("%s\n",result);
    exit(0);
}
