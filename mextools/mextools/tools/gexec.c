/*
 *	gexec -
 *		Exec unix commands from a menu.
 *
 *	Here are some typical lines from the gexec description.
 *
	texback:texback 12
 	mag:mag
	cat picture:ipaste /usr/people/gifts/mextools/images/cat.bw
 	cedit:cedit
	mousemon:mousemon
	melt:melt
 *
 *	This file consists of strings for menu entries and a unix
 *	command line to execute.  The gexec description should be installed
 *	in a directory ~/.gexec/desc there should be an image file installed
 *	in ~/.images/name.icon.
 *
 *	In this case the program would be invoked by
 *	
 *	% gexec name desc
 *
 *				Paul Haeberli - 1985
 *
 */
#include "device.h"
#include "port.h"
#include "image.h"
#include "stdio.h"

typedef struct cmd {
   	struct cmd *next;
	char *cmdstr;
} cmd;

cmd *cmdlist = 0;
FILE *inf;
MEMIMAGE *image;
int menu;
int ncmds = 0;

FILE *configopen();

main(argc,argv)
int argc;
char **argv;
{
    short dev, val;
    int old;
    int i;
    char tempstr[256];

    if (argc<3) {
	fprintf(stderr,"usage: gexec name desc\n");
	exit();
    }
    strcpy(tempstr,argv[1]);
    strcat(tempstr,".icon");
    image = readimage(tempstr);

    strcpy(tempstr,".gexec/");
    strcat(tempstr,argv[2]);
    if ((inf = configopen(tempstr,"r")) == 0) {
	fprintf(stderr,"gexec: can't open input file ~/%s\n",tempstr);
	exit(1);
    }
    prefsize(image->xsize,image->ysize);
    winopen(argv[1]);
    drawimage(0,0,image);
    
    strcpy(tempstr,argv[1]);
    strcat(tempstr," %t");
    menu = defpup(tempstr);
    readcmds(menu,inf);
    fclose(inf);
    qdevice(MENUBUTTON);
    while (1) {
	switch (qread(&val)) {
	    case REDRAW:
		drawimage(0,0,image);
		break;
	    case MENUBUTTON:
		if (val) 
		    doexec(dopup(menu));
		break;
	}
    }
}

readcmds(menu,inf)
int menu;
FILE *inf;
{
    char tempstr[256];
    register char *cptr;
    register cmd *c;

    while (fgets(tempstr,256,inf)) {
	cptr = tempstr;
	while (*cptr) {
	    if (*cptr == ':')
		break;
	    cptr++;
	}
	if (*cptr) {
	   c = (cmd *)malloc(sizeof(cmd));
	   c->next = cmdlist;
	   cmdlist = c;
	   *cptr++ = 0;
	   addtopup(menu,tempstr);
	   c->cmdstr = (char *)malloc(strlen(cptr)+1);
	   strcpy(c->cmdstr,cptr);
	   ncmds++;
	}
    }
}

doexec(n)
int n;
{
    register cmd *c;
    register int i;

    if (n <= 0)
	return;
    c = cmdlist;
    i = ncmds;
    while (c) {
	if (i == n) {
	    dosystem(c->cmdstr);
	    return;
	}
	c = c->next;
	i--;
    }
}
