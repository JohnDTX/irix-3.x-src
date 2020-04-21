# include "glx.h"
# include "ctype.h"

/*
S5|wsiris|iris40|iris emulating a 40 line visual 50 (approximately):\
	:am:al=\EL:\
	:bs:cd=\EJ:ce=\EK:cl=\EH\EJ:ho=\EH:cm=\EY%+ %+ :co#80:li#40:nd=\EC:\
	:sr=\EI:up=\EA:ku=\EA:kd=\EB:kr=\EC:kl=\ED:\
	:ho=\EH:dl=\EM:
 */

# define PROMSTATIC

struct GLX GLX;
# define WP	(&GLX.win)



PROMSTATIC	short EscState;

ScreenChar(ccc)
    register unsigned int ccc;
{
    register int xcol;

    ccc = toascii(ccc);

    if( EscState != 0 ) { ScreenEsc(ccc); return; }

    xcol = GLX.curr.x;

    if( isprint(ccc) )
    {
	if( xcol >= WP->siz.x )
	{
	    xcol = 0;
	    TermWrap(1);
	    glmovcursor(xcol,GLX.curr.y);
	}

	glchar(ccc);
	xcol++;
    } 
    else 
    {
	switch(ccc) 
	{
	case '\007':
	    beep();
	    break;

	case '\033':
	    EscState++;
	    break;

	case '\014':
	    ScreenClear();
	    xcol = 0;
	    break;

	case '\r':
	    xcol = 0;
	    break;

	case '\n':
	    TermWrap(1);
	    break;

	case '\b': /* backspace */
	    if( --xcol < 0 )
	    {
		xcol = WP->siz.x-1;
		TermWrap(-1);
	    }
	    break;

	case '\t': /* tab */
	    xcol += 8;
	    xcol -= xcol%8;
	    break;

	default:
	    break;
	}
    }

    while( xcol >= WP->siz.x )
    {
	xcol -= WP->siz.x;
	TermWrap(1);
    }

    glmovcursor(xcol,GLX.curr.y);
}

static
ScreenEsc(ccc)
    register unsigned int ccc;
{
    register short x,y;
    register short estate;

    x = GLX.curr.x;
    y = GLX.curr.y;

    estate = EscState;
    EscState = 0;

    switch(estate)
    {
    case 0:
	return;

    case 1:
	switch(ccc)
	{
	case 'A':
	    if( --y < 0 )
		y = WP->siz.y-1;
	    break;

	case 'B':
	    if( ++y >= WP->siz.y )
		y = 0;
	    break;

	case 'C':
	    if( ++x >= WP->siz.x )
		x = 0;
	    break;

	case 'D':
	    if( --x < 0 )
		x = WP->siz.x-1;
	    break;

	case 'H':
	    x = y = 0;
	    break;

	case 'I':
	    TermWrap(-1);
	    return;

	case 'J':
	    glclearEOL();
	    if( y < WP->siz.y-1 )
		glclearbox(0,y+1
			,WP->siz.x-1,WP->siz.y-1);
	    return;

	case 'K':
	    glclearEOL();
	    return;

	case 'L':
	    TermInsert();
	    TermRepaint();
	    return;

	case 'M':
	    TermDelete();
	    TermRepaint();
	    return;

	case 'N':
	    TermTransmit();
	    return;

	case 'Y':
	    EscState = 'Y';
	    return;

	case '7':
	    EscState = '7';
	    return;

	case '0':
	case '9':
	    EscState = '7'+1;
	    return;

	case 'r':
	    ungetchar(GLX.curr.y+' ');
	    ungetchar(GLX.curr.x+' ');
	    return;
	}
	break;

    case 'Y'+0:
	EscState = 'Y'+1;
	y = ccc - ' ';
	if( (unsigned)y >= WP->siz.y )
	    return;
	glmovcursor(x,y);
	return;
    case 'Y'+1:
	x = ccc - ' ';
	if( (unsigned)x >= WP->siz.x )
	    return;
	break;

    case '7'+0:
	EscState = estate + 1;
	return;
    case '7'+1:
	return;
    }

    glmovcursor(x,y);
}

ScreenClear()
{
    glclearbox(0,0,WP->siz.x-1,WP->siz.y-1);
    glmovcursor(0,0);
}
