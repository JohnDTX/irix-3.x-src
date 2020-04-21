/*
 * Scaled down version of C Library printf:
 *	- only %s %u %d (==%u) %o %x %D are recognized
 *	- used to print diagnostic information directly on console tty.
 *	  Since it is not interrupt driven, all system activities are
 *	  pretty much suspended
 *
 * $Source: /d2/3.7/src/sys/gl1/RCS/conio.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:28:11 $
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/printf.h"
#include "../gl1/textport.h"

/*
 * printf:
 *	- print something on the console
 */
/*VARARGS1*/
printf(fmt, x1)
	u_int x1;
{
	doprnt(con_putchar, fmt, &x1);
}

/*
 * iprintf:
 *	- print something on the serial console
 */
/*VARARGS1*/
iprintf(fmt, x1)
	u_int x1;
{
	doprnt(duputchar, fmt, &x1);
}

/*
 * fprintf:
 *	- like printf, except the putchar routine is specified instead
 *	  of being assumed
 */
/*VARARGS1*/
fprintf(putc, fmt, x1)
	int (*putc)();
	char *fmt;
	u_int x1;
{
	doprnt(putc, fmt, &x1);
}

/*
 * sputc:
 *	- used by sprintf as the putchar routine to store characters into
 *	  the users buffer
 */
static	char *sputc_addr;
int
sputc(c)
	char c;
{
	*sputc_addr++ = c;
}

/*
 * sprintf:
 *	- like printf, except print into a buffer
 *	- WARNING: THIS IS NOT RE-ENTRANT
 */
sprintf(buf, fmt, x1)
	char *buf;
	char *fmt;
	int x1;
{
	int s;

	s = spl7();
	sputc_addr = buf;
	doprnt(sputc, fmt, &x1);
	*sputc_addr = 0;
	splx(s);
}

/*
 * doprnt:
 *	- do actual printf processing, using "putc" as the output routine
 */
doprnt(putc, fmt, adx)
	int (*putc)();
	register char *fmt;
	register u_int *adx;
{
	register u_char c;
	register char *s;
	register short width, zero_fill;

	for (;;) {
		while ((c = *fmt++) != '%') {
			if (c == '\0') {
out:
				if (putc == grputchar)
					tx_redisplay(&txport[0]);
				return;
			}
			if (c == '\n')
				(*putc)('\r');
			(*putc)(c);
		}
		c = *fmt++;
		zero_fill = width = 0;
		if (c == '0')
			zero_fill = 1;
		while ((c >= '0') && (c <= '9')) {
			width = width * 10 + (c - '0');
			c = *fmt++;
		}
		if (width > 11)
			width = 11;
		switch (c) {
		  case 0:
			(*putc)('%');
			goto out;
		  case '%':
			(*putc)('%');
			continue;			/* skip adx++ */
		  case 'd':
		  case 'u':
			printn((u_long)*adx, 10, width, zero_fill, putc);
			break;
		  case 'U':
		  case 'D':
			printn(*(u_long *)adx, 10, width, zero_fill, putc);
			break;
		  case 'x':
			printn((u_long)*adx, 16, width, zero_fill, putc);
			break;
		  case 'X':
			printn(*(u_long *)adx, 16, width, zero_fill, putc);
			break;
		  case 'o':
			printn((u_long)*adx, 8, width, zero_fill, putc);
			break;
		  case 'O':
			printn(*(u_long *)adx, 8, width, zero_fill, putc);
			break;
		  case 's':
			s = (char *)*adx;
			while (c = *s++) {
				if (c == '\n')
					(*putc)('\r');
				(*putc)(c);
			}
			break;
		  case 'c':
			(*putc)((u_char)*adx);
			break;
		  default:
			(*putc)('%');
			(*putc)(c);
			break;
		}
		adx++;
	}
}

/*
 * printn:
 *	- print out a number
 */
printn(num, base, width, zero_fill, putc)
	register u_long num;
	register short base, width, zero_fill;
	int (*putc)();
{
	register short digit;
	char digit_buf[20];

	digit = 0;
	if (num) {
		while (num) {
			digit_buf[digit++] = "0123456789abcdef"[num % base];
			num /= base;
		}
	} else
		digit_buf[digit++] = '0';

	if (width) {
		while (digit < width) {
			if (zero_fill)
				digit_buf[digit++] = '0';
			else
				digit_buf[digit++] = ' ';
		}
	}
	while (digit)
		(*putc)(digit_buf[--digit]);
}

/*
 * putchar:
 *	- put a character to the current console
 */
putchar(ch)
	char ch;
{
	(*con_putchar)(ch & 0x7F);
}

/*
 * getchar:
 *	- get a character from the current console
 */
getchar()
{
	char c;

	c = (*con_getchar)();
	c &= 0x7F;
	if (c == '\r')
		c = '\n';
	return c;
}

/*
 * gets:
 *	- read a line of input from the user
 *	- erase input nicely, throw away funny control chars
 */
gets(buf)
	u_char *buf;
{
	register u_char *cp;
	register u_char ch;

	cp = buf;
	for (;;) {
		ch = getchar();
		switch (ch) {
		  case '\b':			/* backspace */
		  case '#':
			if (cp == buf)
				break;
			printf("\b \b");
			cp--;
			break;
		  case 'U'&31:
		  case 'X'&31:
		  case '@':			/* kill line */
			while (cp != buf) {
				printf("\b \b");
				cp--;
			}
			break;
		  case '\n':
			*cp = 0;
			printf("\n");
			return;
		  default:
			if (ch >= 32) {
				*cp++ = ch;
				putchar(ch);
			}
			break;
		}
	}
}

int	(*softintr[4])();

extern	int kb_softintr();
extern	int dial_softintr();

/*
 * gl_portinuse:
 *	- see if port `portno' is in use by the gl
 *	- return 1 if it is, 0 if it isn't
 */
gl_portinuse(portno)
	int portno;
{
	return softintr[portno] ? 1 : 0;
}

/*
 * gl_softintr:
 *	- give `c' to software interrupt routine attached to port `portno'
 *	- return 0 if no reciever
 */
gl_softintr(portno, c)
	int portno, c;
{
	int (*fp)();

	if (fp = softintr[portno]) {
		(*fp)(c);
		return 1;
	}
	return 0;
}

/*
 * kbgetchar:
 *	 - read one character from the 'keyboard' duart
 */
kbgetchar()
{
	char buf[100];

	for (;;) {
		if (kb_translate(duxgetchar(0, (long)0), buf) == 1)
			break;		/* real bona-fide key */
	}
	return buf[0];
}
