/*
 *	Kurt Akeley			5/3/82
 *
 *	Interactive debugging tool for use with the BPC version 1.
 *
 *	Updates:
 *		9/11/82	 KBA	Modified for the bpc2
 *		9/17/82	 KBA	Returned to bpc1 specs
 *		9/18/82	 KBA	Subroutines moved to separate files
 *				Main changed to console
 *		12/8/82  KBA	RELOCATE macro included to allow PROM use
 *		12/8/83	 KBA	New revision banner
 *		12/13/83 KBA	RELOCATE macro removed
 *		4/4/84   KBA	Added paul's character handler stuff to
 *				support interrupt and restart.
 *		4/25/84  KBA	Removed paul's character handler stuff.
 *				Added nwgetchar(), setjmp(), and longjmp()
 *				to provide pseudo-interrupt capability.
 *		8/6/84   KBA	Put Paul's stuff back.  Added getcmnd().
 */

#include "console.h"
#include "getcmnd.h"
#include "commands.h"

extern Help helplist[];	/* list of help strings in help.c */

short	colorindex;	/* number 0..2**n, where n is the number*/
			/*   of significant planes		*/
short	weindex;	/* as above, but for the wecode		*/
long	colorcode;	/* mapping of colorindex consistent with*/
			/*   sigplanes				*/
long	wecode;		/* mapping of weindex consistent with	*/
			/*   sigplanes				*/
long	sigplanes;	/* planes whose output is to be		*/
			/*   considered significant		*/
long	installedplanes;/* planes which are actually installed	*/

short	onestipple[16] =      { 0xffff, 0xffff, 0xffff, 0xffff,
				0xffff, 0xffff, 0xffff, 0xffff,
				0xffff, 0xffff, 0xffff, 0xffff,
				0xffff, 0xffff, 0xffff, 0xffff
				};
short	checkstipple[16] =    { 0xaaaa, 0x5555, 0xaaaa, 0x5555,
				0xaaaa, 0x5555, 0xaaaa, 0x5555,
				0xaaaa, 0x5555, 0xaaaa, 0x5555,
				0xaaaa, 0x5555, 0xaaaa, 0x5555
				};
long	env[16];		/* for setjmp and longjmp stuff */

#ifdef PM1
static int myhandler ();
static int pm1break;
#endif PM1

#ifdef PM3
static long vectors[256];
#endif PM3

Cmnddef cmndlist[] = {
    {C_BALL,"ball","ba",0,6,6,"x pos",0,"y pos",0,"x vel",0,"y vel",0,"x acc",5,"y acc",8},
    {C_BPCTEST,"bpctest","bpc",0,3,3,"iterations",1,"verbose",1,"tests",0x3F},
    {C_BPLAMPTEST,"bplamptest","bpl",0,1,1,"iterations",1},
    {C_BPTEST,"bptest","b",0,5,5,"iterations",1,"verbose",1,"tests",0x3F,"chip report",0,"diag",0},
    {C_CHARTEST,"chartest","ch",0,2,2,"iterations",1,"verbose",1},
    {C_CLEAR,"clear","cl",0,2,2,"x stipple",0xFFFF,"y stipple",0},
    {C_CMNDLIST,"cmndlist","cm",0,0,0},
    {C_COLORCODE,"colorcode","colorc",0,1,1,"code",0},
    {C_COLORWETEST,"colorwetest","colorw",0,2,2,"iterations",1,"verbose",1},
    {C_COLOR,"color","c",0,1,0,"index",0},
    {C_DCLAMPTEST,"dclamptest","dcl",0,1,1,"iterations",1},
    {C_DCPAL,"dcpal","dcp",0,1,0,"value",0},
    {C_DCRTEST,"dcrtest","dcrt",0,2,2,"iterations",1,"verbose",1},
    {C_DCR,"dcr","dcr",0,1,0,"dcr",0},
#ifdef UC4
    {C_DDATEST,"ddatest","ddat",0,3,3,"iterations",1,"verbose",1,"tests",0x1FF},
#endif UC4
    {C_DEFMACRO,"defmacro","def",0,1,1,"number",0},
#ifdef UC4
    {C_DEPTHCUE,"depthcue","dep",0,7,7,"x1",0,"y1",0,"x2",0x3FF,"y2",0x2FF,"stipple",0xFFFF,"color1",0,"color2",0xF},
#endif UC4
    {C_DISPLAY,"display","dis",0,2,2,"x word address",0,"y address",0},
    {C_DRAWCHAR,"drawchar","drawc",0,2,2,"x address",0,"y address",0},
    {C_DRAWLINE,"drawline","dr",0,5,5,"x1",0,"y1",0,"x2",0x3FF,"y2",0x2FF,"stipple",0xFFFF},
    {C_ENDMACRO,"endmacro","e",0,0,0},
    {C_FILLRECT,"fillrect","fillr",0,6,6,"x",0,"y",0,"width",0x100,"height",0x100,"x stipple",0xFFFF,"y stipple",0},
#ifdef UC4
    {C_FILLTRAP,"filltrap","fillt",0,8,8,"x",0,"y",0,"width",0x100,"height",0x100,"topx",0x10,"topwidth",0x80,"x stipple",0xFFFF,"y stipple",0},
#endif UC4
    {C_FMTEST,"fmtest","fm",0,4,4,"iterations",1,"verbose",1,"tests",0x1F,"diag",0},
#ifdef PM3
    {C_FPTEST,"fptest","fp",0,3,3,"iterations",1,"verbose",1,"optreg",0x01},
#endif PM3
    {C_GFTEST,"gftest","gf",0,3,3,"iterations",1,"verbose",1,"tests",-1},
    {C_HELP,"help","he",0,0,0},
    {C_HISTORY,"history","h",0,1,0,"command number",0},
    {C_INIT,"init","i",0,1,1,"mode",0},
    {C_JOESCALE,"joescale","j",0,0,0},
    {C_LDCONFIG,"ldconfig","ldc",0,1,0,"value",0xDF},
#ifdef UC4
    {C_LDDDASAF,"ldddasaf","ldddasaf",0,1,1,"12-bit value",0},
    {C_LDDDASAI,"ldddasai","ldddasai",0,1,1,"12-bit value",0},
    {C_LDDDASDF,"ldddasdf","ldddasdf",0,1,1,"12-bit value",0},
    {C_LDDDASDI,"ldddasdi","ldddasdi",0,1,1,"12-bit value",0},
    {C_LDDDAEAF,"ldddaeaf","ldddaeaf",0,1,1,"12-bit value",0},
    {C_LDDDAEAI,"ldddaeai","ldddaeai",0,1,1,"12-bit value",0},
    {C_LDDDAEDF,"ldddaedf","ldddaedf",0,1,1,"12-bit value",0},
    {C_LDDDAEDI,"ldddaedi","ldddaedi",0,1,1,"12-bit value",0},
#endif UC4
    {C_LDEC,"ldec","ldec",0,1,1,"correction",0},
    {C_LDED,"lded","lded",0,1,1,"delta",0},
    {C_LDFMADDR,"ldfmaddr","ldf",0,1,1,"font address",0},
#ifdef UC4
    {C_LDMODE,"ldmode","ldm",0,1,0,"value",0},
    {C_LDREPEAT,"ldrepeat","ldr",0,1,1,"stipple repeat",0},
#endif UC4
    {C_LDXE,"ldxe","ldxe",0,1,1,"x end address",0},
    {C_LDXS,"ldxs","ldxs",0,1,1,"x start address",0},
    {C_LDYE,"ldye","ldye",0,1,1,"y end address",0},
    {C_LDYS,"ldys","ldys",0,1,1,"y start address",0},
    {C_LINETEST,"linetest","linet",0,3,3,"iterations",1,"verbose",1,"tests",0x7F},
    {C_LOOPCLEARWORD,"loopclearword","loopc",0,3,3,"x word address",0,"y address",0,"x stipple",0x5555},
    {C_LOOP,"loop","lo",0,2,2,"command",0,"data",0},
    {C_MACRO,"macro","mac",0,2,2,"number",0,"iterations",1},
    {C_MAPCOLOR,"mapcolor","mapc",0,4,4,"index",0,"red",0,"green",0,"blue",0},
    {C_MAPTEST,"maptest","mapt",0,4,4,"iterations",1,"verbose",1,"tests",0x1F,"diag",0},
#ifdef PM2
    {C_MEMTEST,"memtest","me",0,1,1,"verbose",1},
#endif PM2
#ifdef PM3
    {C_MEMTEST,"memtest","me",0,3,3,"iterations",1,"verbose",1,"tests",0xFF},
#endif PM3
#ifdef UC4
    {C_PIXELTEST,"pixeltest","pi",0,3,3,"iterations",1,"verbose",1,"tests",0xF},
#endif UC4
#ifdef PM2
    {C_PRINT,"print","pr",0,0,0},
#endif PM2
    {C_QUIT,"quit","q",0,0,0},
    {C_RAMPTEST,"ramptest","ram",0,4,4,"red",1,"green",1,"blue",1,"rgb",0},
    {C_RANDLINES,"randlines","randl",0,3,3,"seed",0,"number",0x200,"stipple",0xFFFF},
    {C_RANDRECT,"randrect","randr",0,1,1,"log2 size",8},
    {C_READDDA,"readdda","readd",0,0,0},
    {C_READFM,"readfm","readf",0,2,2,"address",0,"locations",1},
    {C_READLOOP,"readloop","readl",0,1,1,"i/o address",0},
    {C_READMAP,"readmap","readm",0,2,2,"address",0,"locations",1},
    {C_READWORD,"readword","readw",0,2,2,"x word address",0,"y address",0},
    {C_READ,"read","rea",0,1,1,"i/o address",0},
    {C_RECTTEST,"recttest","rec",0,2,2,"iterations",1,"verbose",1},
    {C_REQUEST,"request","req",0,2,2,"command",0,"data",0},
    {C_RESTORE,"restore","res",0,1,0,"reg num", 0},
    {C_REVISION,"revision","rev",0,0,0},
    {C_RGBCOLOR,"rgbcolor","rg",0,3,3,"red",0,"green",0,"blue",0},
    {C_ROTATELOOP,"rotateloop","ro",0,2,2,"command",0,"data",0},
    {C_SAVE,"save","sa",0,1,1,"reg num",0},
    {C_SCREENMASK,"screenmask","scre",0,4,4,"left",0,"bottom",0,"width",0x400,"height",0x300},
    {C_SCRMSKTEST,"scrmsktest","scrm",0,2,2,"iterations",1,"verbose",1},
    {C_SEEMACRO,"seemacro","se",0,1,1,"number",0},
    {C_SIGPLANES,"sigplanes","si",0,1,0,"planes",0xFFFFFFFF},
    {C_STEPTEST,"steptest","ste",0,4,4,"red",1,"green",1,"blue",1,"rgb",0},
    {C_STRIPETEST,"stripetest","stripet",0,4,4,"iterations",1,"verbose",1,"fields",0x40,"singlebuf",0},
    {C_STRIPE,"stripe","str",0,2,2,"width",3,"step",6},
    {C_SWAP,"swap","sw",0,1,1,"delay",0x10000},
    {C_TIME,"time","ti",0,1,1,"test number",0},
#ifdef UC4
    {C_TRAPTEST,"traptest","tr",0,3,3,"iterations",1,"verbose",1,"tests",0x7},
#endif UC4
    {C_UCLAMPTEST,"uclamptest","ucl",0,1,1,"iterations",1},
#ifdef UC4
    {C_UCR,"ucr","ucr",0,1,1,"value",0x200},
#endif UC4
    {C_WECODE,"wecode","wec",0,1,1,"code",0xFFFFFFFF},
    {C_WE,"we","we",0,1,0,"index",0xFFF},
    {C_WRITEFM,"writefm","writef",0,2,2,"address",0,"value",0},
    {C_WRITEMAP,"writemap","writem",0,4,4,"address",0,"red",0,"green",0,"blue",0},
    {C_WRITELOOP,"writeloop","writel",0,2,2,"i/o address",0,"data",0},
    {C_WRITEWORD,"writeword","writew",0,3,3,"x word address",0,"y address",0,"data word",0},
    {C_WRITE,"write","wr",0,2,2,"i/o address",0,"data",0},
    {C_NEWARG,"$","$",2,2,2,"arg",1,"newvalue",0},
    {C_ENDMACRO,".",".",0,0,0},
    {C_CMNDLIST,"?","?",0,0,0},
    {C_HISTORY,"!","!",0,1,0,"command number",0},
    {C_NOTACOMMAND,"","",0,0,0}};


#define MAXHISTORY	10

static char name[] = "BPC Diagnostic";
extern char date[];				/* declared in date.c */
Command macrolist[MAXMACRO][MACROSIZE+1];
Command historylist[MAXHISTORY];

main () {
    Command cmnd;
    boolean defmacro;
    short macrocmnd;
    short macronum;
    short cmndnum;
    short i, j, k;

    cmndnum = 1;
    cmnd.number = C_NOTACOMMAND;
    cmnd.args = 0;
    defmacro = FALSE;
    for (i=0; i<MAXMACRO; i++)
	macrolist[i][0].number = C_NOTACOMMAND;
    for (i=0; i<MAXHISTORY; i++)
	historylist[i].number = C_NOTACOMMAND;
#ifdef PM2
    initcircbuf ();
#endif PM2
#ifdef PM3
    movevectors ();
#endif PM3
    printrevision ();
    printf ("  Type <space> to abort the current command\n");
#ifdef PM1
    serialhandler (0, myhandler);
    pm1break = FALSE;
#endif PM1
    setjmp (env);
    while (TRUE) {
	if (defmacro)
	    printf ("[defmacro-%x,%x] -> ", macronum, macrocmnd+1);
	else
	    printf ("[%x] -> ", cmndnum);
	getcmnd (&cmnd, cmndlist, helplist);
	switch (cmnd.number) {
	    case C_REVISION:
		printrevision ();
		break;
	    case C_HELP:
		printf ("  type <help ?> for program information\n");
		break;
	    case C_DEFMACRO:
		printf ("  type <end> or <.> to complete macro definition\n");
		defmacro = TRUE;
		macronum = cmnd.arg[0];
		macrocmnd = 0;
		break;
	    case C_ENDMACRO:
		defmacro = FALSE;
		break;
	    case C_HISTORY:
		if (cmnd.args == 0) {
		    copycmnd (&cmnd, &historylist[hindex(cmndnum)]);
		    printhistory (cmndnum);
		    break;
		    }
		else {
		    short hp, minhp;
		    hp = cmnd.arg[0];
		    minhp = (MAXHISTORY < (cmndnum-1)) ? -MAXHISTORY
						       : 1-cmndnum;
		    if (hp > 0 && hp < cmndnum)
			copycmnd (&historylist[hindex(hp)], &cmnd);
		    else if (hp < 0 && hp >= minhp)
			copycmnd (&historylist[hindex(cmndnum+hp)], &cmnd);
		    else {
			printf ("  argument %x out of range\n",hp);
			cmnd.number = C_NOTACOMMAND;
			cmnd.args = 0;
			break;
			}
		    printcmnd (&cmnd, cmndlist);
		    printf ("\n");
		    }
		/* no break - go directly to default !!! */
	    default:
		if (doline (&cmnd, TRUE, cmndlist, macrolist)) {
		    if (setjmp (env))
			printf ("\n");
		    if (defmacro) {
			if (macrocmnd<MACROSIZE) {
			    copycmnd(&cmnd,&macrolist[macronum][macrocmnd++]);
			    macrolist[macronum][macrocmnd].number =
				C_NOTACOMMAND;
			    }
			else {
			    printf ("  macro full\n\007");
			    defmacro = FALSE;
			    }
			}
		    }
		else {
		    putchar (007);
		    }
		break;
	    }
	copycmnd (&cmnd, &historylist[hindex(cmndnum)]);
	cmndnum += 1;
	}
    }

#ifdef PM1
static myhandler (onechar)
int onechar;
{
    if(onechar == ' ')
	pm1break = TRUE;
    }
#endif PM1

long	breakcheck () {
#ifdef PM1
    if (pm1break) {
	pm1break = FALSE;
	dobreak ();
	}
#endif PM1	
#ifdef PM2
    if (nwgetchar () == ' ') {
	dobreak ();
	}
#endif PM2
#ifdef PM3
    if (nwgetchar () == ' ') {
	dobreak ();
	}
#endif PM3
    }

dobreak () {
    /* unconditional branch to start of program */
    putchar (007);
    longjmp (env, 1);
    }

printrevision () {
    printf ("  %s: %s\n", name, date);
#ifdef UC3
    printf ("    UC3\n");
#endif UC3
#ifdef UC4
    printf ("    UC4\n");
#endif UC4
#ifdef DC3
    printf ("    DC3\n");
#endif DC3
#ifdef DC4
    printf ("    DC4\n");
#endif DC4
#ifdef INTER2
    printf ("    Interface 2 (GF1 board)\n");
#endif INTER2
#ifdef INTER3
    printf ("    Interface 3 (UC4 Multibus Interface)\n");
#endif INTER3
#ifdef PM1
    printf ("    PM1\n");
#endif PM1
#ifdef PM2
    printf ("    PM2\n");
#endif PM2
#ifdef PM3
    printf ("    IP2\n");
#endif PM3
    }

hindex (cmndnum)
short cmndnum;
{
    /* returns the history array index that corresponds to the command num */
    return cmndnum % MAXHISTORY;
    }

printhistory (cmndnum)
short cmndnum;
{
    short i, mini;
    mini = cmndnum - (MAXHISTORY - 1);
    if (mini < 1)
	mini = 1;
    for (i=mini; i<=cmndnum; i++) {
	printf ("  %3x  ", i);
	printcmnd (&historylist[hindex(i)], cmndlist);
	printf ("\n");
	}
    }

#ifdef PM3
movevectors () {
    register i;
    register long *lp;
    /* move the exception vectors from the current location (PROM)
     * to a static array so that they can be changed.
     */

    lp = (long*)getvbr ();
    for (i=0; i<256; i++)
	vectors[i] = lp[i];
    setvbr (vectors);
    *(short*)0x38000000 |= 0x8000;	/* fast timeout */
    }
#endif PM3
