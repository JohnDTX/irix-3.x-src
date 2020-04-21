#include "reentrant.h"
#include "chars.h"
#include "nec7201.h"
#include "necuart.h"
#include "iriskeybd.h"
#include "device.h"
#include "devdata.h"


#define ENTER_ALT_MODE 	1
#define EXIT_ALT_MODE 	2
#define FUNNYESC	(0x80|ESC)

#define MIN(a,b)	((a)<(b) ? (a) : (b))

typedef int (*PFI)();

static int 	doaltmode;
static int 	clickstate = ClickDisable;
static int 	lightstate = 0;
static short 	capslockstate = 0;	/* 0 = up; 1 = down */
static short 	ctrlkey = 0;
static short 	shiftkey = 0;
static short	softkeyboard = 0;
static int 	serialinited;
static PFI	shandler[2];
static int 	awaiting[2];
static int 	readchar[2];
short 	s_interrupt;

extern int serialinter();
extern buttondata kbuttons[];

serialhandler( line, h )
int line;
PFI h;
{
    if(!serialinited)
       k_initserial();
    if(line<0 || line >1)
	return;
    shandler[line] = h;
}

havesoftkbd()
{
    if(!serialinited)
       k_initserial();
    return softkeyboard;
}

startbreak()
{
    char *cp;

    cp = &(ContReg(B));
    *cp = (char)5;
    *cp = (char)(NECINR5 | NECsndbrk);
}

endbreak()
{
    char *cp;

    cp = &(ContReg(B));
    *cp = (char)5;
    *cp = (char)(NECINR5);
}

startaltmode()
{
    if(!serialinited)
       k_initserial();
    doaltmode = 1; 
}

endaltmode()
{
    if(!serialinited)
       k_initserial();
    doaltmode = 0; 
}

ringbell()
{
    if(!serialinited)
       k_initserial();
    if( softkeyboard ) {
	lineputc(0,ShortBeep|clickstate);
    } else
	lineputc(0,0x7);
}

clickon()
{
    if(!serialinited)
       k_initserial();
    clickstate = ClickEnable;
    if( softkeyboard ) {
	lineputc(0,clickstate);
    }
}

clickoff()
{
    if(!serialinited)
       k_initserial();
    clickstate = ClickDisable;
    if( softkeyboard ) {
	lineputc(0,clickstate);
    }
}

lighton(lamps)
int lamps;
{
    if(!serialinited)
       k_initserial();
    lightstate |= lamps;
    if( softkeyboard ) {
	lineputc(0,lightstate | LEDCmd);
    }
}

lightoff(lamps)
int lamps;
{
    if(!serialinited)
       k_initserial();
    lightstate &= ~lamps;
    if( softkeyboard ) {
	lineputc(0,lightstate | LEDCmd);
    }
}


putchar( c )
int c;
{
    if(c=='\n') 
	lineputc(0,'\r');
    lineputc(0,c);
}

getchar()
{
    return linegetc(0);
}


linegetc(line)
register int line;
{
    unsigned char c;
    register short pri;

    if(!serialinited)
       k_initserial();
    if(s_interrupt) {
	while( !linereadyrx(line) ) 
	    ;
	return lineget(line);
    }
    else {
	awaiting[line] = 1;
	while( awaiting[line] )
	    ;
	return readchar[line];
    }
}

lineputc(line,c)
int line;
char c;
{
    if(!serialinited)
       k_initserial();
    while(!linereadytx(line)) 
	;
    lineput(line, c);
}

static k_initserial()
{
    register long i;

    for(i=0; i<20000; i++)
	;
    if( (*(unsigned long *)0x400008 < 0x40000000) 
				&& (*(unsigned long *)0x400008 >= 0x90008)) 
	softkeyboard = *(long *)0x4 & 1;
    else 
        softkeyboard = ! ((*(unsigned short *)0x114) & 0x40);
    doaltmode = 0;
    lineservice(serialinter);
    linereset(0);
    linereset(1);
    linearmrx(0);
    linearmrx(1);
    serialinited = 1;
    spl0();
}

static interpretsoftkeyboard(ch,str)
unsigned char ch;
unsigned char *str;
{
    register long key, stroke;
    register unsigned short ascii;

    stroke = (ch & 0x80) ? 0 : 1;	/* high order bit => upstroke */
    key = (ch & 0x7f) + BUTOFFSET;

   /* Check if a shift, control or caps lock. */
    if( key == CTRLKEY ) {
	ctrlkey = stroke;
	return 0;
    }
    if( (key == RIGHTSHIFTKEY) || (key == LEFTSHIFTKEY) ) {
	shiftkey = stroke;
	return 0;
    }
    if( key == CAPSLOCKKEY ) {
	if(stroke == 0 ) capslockstate = 1 - capslockstate;
	return 0;
    }
    if( stroke == 0 ) 		/* key just went up. */
	return 0; 

    /* Down stroke of a key other than control, shift or shift lock. */

    switch( 4*capslockstate + 2*ctrlkey + shiftkey ) {
	case 0:	/* neither button down */
		ascii = kbuttons[key].ascii;
		break;
	case 1:	/* shift only */
	case 5:	/* caps lock + shift */
		ascii = kbuttons[key].shiftascii;
		break;
	case 2:	/* control only */
	case 6:	/* caps lock + control */
		ascii = kbuttons[key].ctrlascii;
		break;
	case 3:	/* control + shift */
	case 7:	/* caps lock + control + shift */
		ascii = kbuttons[key].ctrlshiftascii;
		break;
	case 4:	/* caps lock */
		ascii = kbuttons[key].ascii;
		if ('a' <= ascii && ascii <= 'z')
		    ascii = ascii + 'A' - 'a';
		break;
    }

    /* Return it if a standard ascii code. */
    if ((ascii & 0x80) == 0) {
	str[0] = ascii;
	return 1;
    }
    /* Else some weirdo character, or illegal */
    str[0] = FUNNYESC;
    switch (ascii) {
	case 0x80:	/* BREAK */
		str[0] = 0x80;
		return 1;
	case 0x81:	/* BUTTON71 - PADPF1KEY */
		str[1] = 'P';
		return 2;
	case 0x82: 	/* BUTTON70 - PADPF2KEY */
		str[1] = 'Q';
		return 2;
	case 0x83: 	/* BUTTON78 - PADPF3KEY */
		str[1] = 'R';
		return 2;
	case 0x84: 	/* BUTTON77 - PADPF4KEY */
		str[1] = 'S';
		return 2;
	case 0x85: 	/* BUTTON80 - UPARROWKEY */
		str[1] = 'A';
		return 2;
	case 0x86: 	/* BUTTON73 - DOWNARROWKEY */
		str[1] = 'B';
		return 2;
	case 0x87: 	/* BUTTON79 - RIGHTARROWKEY */
		str[1] = 'C';
		return 2;
	case 0x88: 	/* BUTTON72 - LEFTARROWKEY */
		str[1] = 'D';
		return 2;
    }
    if( (doaltmode) && (ascii >= 0x80) ) {
	str[0] = FUNNYESC;
	str[1] = '?';
	str[2] = ascii-0x80 + 0x40;
	return 3;
    }
    if( (!doaltmode) && (ascii >= 0x80) ) {
	str[0] = ascii - 0x80;
	return 1;
    }
    return 0; 
}

reentrant(serialinter)
{
    register struct ring *r;	
    register unsigned char c;
    register char *in;
    unsigned char keystr[10];
    int keystrlen, i;

    s_interrupt = 1;
    if( linereadyrx(0) ) {
	c = lineget(0);
	if( softkeyboard ) 
	    keystrlen = interpretsoftkeyboard(c,keystr);
	else {
	    keystrlen = 1;
	    keystr[0] = c;
	}
	if(awaiting[0]) {
	    readchar[0] = c;
	    awaiting[0] = 0;
	} else if(shandler[0])
	    for(i=0; i<keystrlen; i++)
		(shandler[0])(keystr[i]);
    }
    if( linereadyrx(1) ) {
	c = lineget(1);
	if(awaiting[1]) {
	    readchar[1] = c;
	    awaiting[1] = 0;
	} else if(shandler[1])
	    (shandler[1])(c);
    }
    s_interrupt = 0;
}
