#include "stdio.h"
#include "chars.h"

/*
**	ttygetline - get a line of text from the keyboard with a little
**		  editting
**
*/
ttygetline(buf)
register char *buf;
{
	register c, n = 0;

	while( n < 80 ) {
		flushscreen();
		c = getc(stdin);
		if(c == '\r' || c == '/n') {
			putscrchar('\r');
			putscrchar('\n');
			flushscreen();
			break;
		}
		else if(c == BS || c == DEL) {
			if(n) {
				putscrchar(BS);
				putscrchar(' ');
				putscrchar(BS);
				--n; --buf;
			}
			continue;
		}
		else if(c == NAK) {		/* ^U */
			while(n) {
				putscrchar('\b');
				putscrchar(' ');
				putscrchar('\b');
				--n; --buf;
			}
			continue;
		}
		else { 
			putscrchar(c);
			*buf++ = c;
			n++;
		}
	}
	*buf = 0;
	flushscreen();	
}

#ifdef NOTDEF
gl_IdleForRetrace()
{
}

gl_DisplayListRetrace()
{
}
#endif
