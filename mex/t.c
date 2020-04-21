#include "stdio.h"
#include "misc.h"
#include "device.h"

extern	hogwhiledownfunc(), killfunc(), movefunc(), pushfunc(), menufunc(),
	popfunc(), hogmodefunc(), popattachfunc(), attachfunc(),
	movegrowfunc();

struct butfuncs {
    char	name[40];
    int		(*func)();
};

struct butfuncs bf[] = {
    "menu",		menufunc,
    "hogwhiledown",	hogwhiledownfunc,
    "hogmode",		hogmodefunc,
    "kill",		killfunc,
    "move",		movefunc,
    "movegrow",		movegrowfunc,
    "push",		pushfunc,
    "pop",		popfunc,
    "popattach",	popattachfunc,
    "attach",		attachfunc,
    "",			0
};

FILE *popen();
FILE *rcopen();

#define DOTMEXRC ".mexrc"
#define DEFAULTMEXRC "/usr/lib/gl2/mexrc"
#define CPPPROG		"/lib/cpp"
#define CPPSTRING	"/lib/cpp -P "

char s1[80], s2[80], s3[80], s4[80], s5[80], buf[100];


/* 
 * rstcmap.c in the sources for wsiris was built from this and should track
 * any changes to the way .mexrc is read and to the mapcolor command
 */

readinit()
{
    register struct butfuncs *bfptr;
    int i;
    FILE *desktop;
    int index, r, g, b;
    int n;

    if( (desktop = rcopen()) == 0) {
	nofiledefaults();
	return;
    }
    while(!feof(desktop)) {
	fgets(buf, 100, desktop);
	n = sscanf(buf,"%s %s %s %s %s\n",s1,s2,s3,s4,s5);
	if((n>0) && (s1[0] != '#')) {
	    if(strcmp(s1,"bindfunc") == 0) {
		for(bfptr = &bf[0], i = 0; bfptr->name[0]; i++, bfptr++)
		    if(strcmp(bfptr->name, s2) == 0) {
			bindfunc(bfptr->func, atoi(s3));
			break;
		    }
		if(!bfptr->name[0])
		    fprintf(stderr,"unintelligible function name: %s\n",s2);
	    } else if(strcmp(s1,"bindindex") == 0) {
		if(strcmp(s2, "inborder") == 0)
		    stdcolors.binnercolor = atoi(s3);
		else if(strcmp(s2, "outborder") == 0)
		    stdcolors.boutercolor = atoi(s3);
		else if(strcmp(s2, "titletextin") == 0)
		    stdcolors.tinnercolor = atoi(s3);
		else if(strcmp(s2, "titletextout") == 0)
		    stdcolors.toutercolor = atoi(s3);
		else if(strcmp(s2, "hiinborder") == 0)
		    hilightcolors.binnercolor = atoi(s3);
		else if(strcmp(s2, "hioutborder") == 0)
		    hilightcolors.boutercolor = atoi(s3);
		else if(strcmp(s2, "hititletextin") == 0)
		    hilightcolors.tinnercolor = atoi(s3);
		else if(strcmp(s2, "hititletextout") == 0)
		    hilightcolors.toutercolor = atoi(s3);
	    } else if(strcmp(s1,"bindcolor") == 0) {
		if(strcmp(s2, "cursor") == 0) {
		    cursorr = atoi(s3);
		    cursorg = atoi(s4);
		    cursorb = atoi(s5);
		} else if(strcmp(s2, "menu") == 0) {
		    menur = atoi(s3);
		    menug = atoi(s4);
		    menub = atoi(s5);
		} else if(strcmp(s2, "menuback") == 0) {
		    menubr = atoi(s3);
		    menubg = atoi(s4);
		    menubb = atoi(s5);
		}
	    } else if(strcmp(s1,"reservebut") == 0) {
		gl_reservebutton(1, atoi(s2));
	    } else if(strcmp(s1,"imakebackground") == 0) {
		sscanf(buf, "%s %[^\n]\n",s1,s2);
		strcat(s2, "&");
		system(s2);
	    } else if(strcmp(s1,"mapcolor") == 0) {
	        mapcolor(atoi(s2),atoi(s3),atoi(s4),atoi(s5));
	    }
	}
    }
    pclose(desktop);
}

int havecpp = 0;

FILE *rcopen()
{
    char homepath[100];
    char cmd[200];
    FILE *f;
    char *cptr;

    if(!access(CPPPROG,2)) {
   	havecpp = 0;
    } else {
   	havecpp = 0;
    }
    strcpy(cmd,CPPSTRING);
    if(access(DOTMEXRC,0x04)==0) { 	/* try in local directory */
	return popen(strcat(cmd,DOTMEXRC),"r");
    if(cptr = (char *)getenv("HOME")) {		/* try in home directory */
	strcpy(homepath,cptr);
	strcat(homepath,"/");
	strcat(homepath,DOTMEXRC);
	if(access(homepath,0x04)==0) 
	    return popen(strcat(cmd,homepath),"r");
    }
    if(access(DEFAULTMEXRC,0x04)==0) 
	return popen(strcat(cmd,DEFAULTMEXRC),"r");
    else 
	return 0;
}

nofiledefaults()
{
    bindfunc(menufunc, RIGHTMOUSE);
}
