/*
**	getline - get a line of text from the keyboard with a little
**		  editting
**
*/
getline(buf)
register char *buf;
{
	register c, n = 0;

	while( n < 80 ) {
		c = linegetc(0);
		if(c == '\b') {
			if(n) {
				lineputc(0,'\b');
				lineputc(0,' ');
				lineputc(0,'\b');
				--n; --buf;
			}
			continue;
		}
		if(c == 21) {		/* ^U */
			while(n) {
				lineputc(0,'\b');
				lineputc(0,' ');
				lineputc(0,'\b');
				--n; --buf;
			}
			continue;
		}
		lineputc(0,c);		/* Echo the character */
		if(c == '\r') {
			lineputc(0,'\n');
			break;
		}

		*buf++ = c;
		n++;
	}
	*buf = 0;
}
