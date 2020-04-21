# define PROMSTATIC

PROMSTATIC char *numptr;

/*
** 	printf - a scaled down version of C Library printf.
** 		 only %s %c %u %d %o %x are recognized.
*/
printf(fmt, x1)
register char *fmt;
unsigned x1;
{
    register c;
    register unsigned int *adx;
    register base;
    register width;
    register fillc, left;
    register char *s;
    char numbuf[32];
    
    adx = &x1;

    for(;;) {
	while((c = *fmt++) != '%') {
		if(c == '\0')
			return;
		putchar(c);
		if(c == '\n')
		    putchar('\r');
	}
	width = 0;
	if(*fmt == '-') {
		left = 1;
		fmt++;
	} else
		left = 0;
	if(*fmt == '0') {
		fillc = '0';
		left = 0;
	} else
		fillc = ' ';
	while(*fmt >= '0' && *fmt <= '9')
		width = width * 10 + (*fmt++ - '0');
	c = *fmt++;
	switch(c) {
	case 'o': case 'O':
		base = 8; goto ppp;
	case 'u': case 'U':
		base = -10; goto ppp;
	case 'd': case 'D':
		base = 10; goto ppp;
	case 'x': case 'X':
		base = 16;
	ppp:	numptr = numbuf;
		printn((long)*adx, base);
		*numptr = 0;
		c = numptr - numbuf;
		s = numbuf;
	oput:	if(left == 0 && c < width)
			do putchar(fillc);
			while(++c < width);
		while(*s)
			putchar(*s++);
		if(left && c < width)
			do putchar(fillc);
			while(++c < width);
		break;
	case 's':
		for(s = (char *)*adx; *s; s++) ;
		c = s - (char *)*adx;
		s = (char *)*adx;
		goto oput;
	case 'c':
		putchar(*adx);
		break;
	case '%':
		putchar(c);
		continue;
	default:
		putchar('%'); putchar('?');
		break;
	}
	adx++;
    }
}

/*
** 	printn - print an integer in base b.
**
*/
static
printn(n, b)
unsigned long n;
{
	register long a;

	if (b == 10 && (int)n < 0) {
		*numptr++ = '-';
		n = -(int)n;
	}
	if(b < 0) b = -b;
	if(a = n/b)
		printn(a, b);
	*numptr++ = "0123456789ABCDEF"[(int)(n%b)];
	*numptr = 0;
}
