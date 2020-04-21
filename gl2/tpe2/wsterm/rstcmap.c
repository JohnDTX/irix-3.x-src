/**************************************************************************
 *									  *
 * 		 Copyright (C) 1985, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

#ifndef GL1

#include <stdio.h>

static char s1[80], s2[80], s3[80], s4[80], s5[80], buf[100];

static FILE *configopen();

extern char *concat();

/*
**	rstcmap - restore the color map from .mexrc.  This code was made
**		  from that in scanner.c in the mex sources, and track it.
**
*/ 
rstcmap()
{
    FILE *desktop;
    int n;

    if ((desktop = configopen(".mexrc","r")) == 0) {
	return;
    }
    while (!feof(desktop)) {
	fgets(buf, sizeof buf, desktop);
	n = sscanf(buf,"%s %s %s %s %s\n",s1,s2,s3,s4,s5);
	if (n > 0 && s1[0] != '#') {
	    if (strcmp(s1,"mapcolor") == 0) {
	        mapcolor(atoi(s2),atoi(s3),atoi(s4),atoi(s5));
	    }
	}
    }
    fclose(desktop);
}

static FILE *configopen(name, mode)
char name[];
char mode[];
{
    char *home;
    char *configfile;
    FILE *fd = NULL;

    if ((home = concat(getenv("HOME"), "/"))
		&& (configfile = concat(home, name))) {
	fd = fopen(configfile,mode);
	free(home);
	free(configfile);
    }
    return fd;
}

#endif not GL1
