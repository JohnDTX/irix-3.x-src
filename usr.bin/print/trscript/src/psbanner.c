#ifndef lint
static char Notice[]="Copyright (C) 1985 Adobe Systems Incorporated";
static char *RCSID="$Header: /d2/3.7/src/usr.bin/print/trscript/src/RCS/psbanner.c,v 1.1 89/03/27 18:20:46 root Exp $";
#endif
/* psbanner.c
 *
 * Copyright (c) 1985 Adobe Systems Incorporated
 *
 * System V banner/breakpage program 
 *
 * RCSLOG:
 * $Log:	psbanner.c,v $
 * Revision 1.1  89/03/27  18:20:46  root
 * Initial check-in for 3.7
 * 
 * Revision 1.1  86/11/17  20:22:25  root
 * Initial revision
 * 
 * Revision 2.1  85/11/24  11:49:54  shore
 * Product Release 2.0
 * 
 * Revision 1.1  85/11/20  00:25:33  shore
 * Initial revision
 * 
 *
 */

#include <stdio.h>
#include <pwd.h>
#include <string.h>
#include <ctype.h>
#include "transcript.h"

struct passwd *getpwnam();
VOID quote();

/* psbanner
 * 	gets called with argv:
 *	printer seqid user title date
 */
main(argc, argv)
int argc;
char **argv;
{
    struct passwd *pswd;
    char *program, *bannerpro, *fulluser;
    char *printer, *seqid, *user, *title, *date;
    char host[100];

    program = strrchr(*argv,'/');
    if (program) program++;
    else program = *argv;
    argv++;

    printer = *argv++;
    seqid = *argv++;
    user = *argv++;
    title = *argv++;
    date = *argv++;

    if ((pswd = getpwnam(user)) == NULL) fulluser = "";
    else fulluser = pswd->pw_gecos;
    gethostname(host,100);

    bannerpro = envget("BANNERPRO");
    copyfile(bannerpro,stdout);

    quote(user);
    quote(fulluser);
    quote(host);
    quote(seqid);
    quote(title);
    quote(printer);
    quote(date);
    printf("Banner\n");
    return(0);

}

/* send PostScript delimited/quoted string to stdout */
VOID quote(str)
char *str;
{
    int c;
    putchar('(');
    while ((c = ((*str++) & 0377)) != 0) {
	if (isascii(c) && (isprint(c) || isspace(c))) {
	    if ((c == '(') || (c == ')') || (c =='\\')) {
		putchar('\\');
	    }
	    putchar(c);
	}
	else {
	    putchar('\\');
	    putchar(((c>>6)&03)+'0');
	    putchar(((c>>3)&07)+'0');
	    putchar((c&07)+'0');
	}
    }
    putchar(')');
    putchar('\n');
}
