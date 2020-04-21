/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/*
 * Scaled down version of C Library printf:
 *	- only %s %u %d (==%u) %o %x %D are recognized
 *	- used to print diagnostic information directly on console tty.
 *	  Since it is not interrupt driven, all system activities are
 *	  pretty much suspended
 *
 * $Source: /d2/3.7/src/gl2/gl2/kgl/RCS/printf.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 15:57:50 $
 */

#include "sys/types.h"
#include "printf.h"
#include "shmem.h"
#include "window.h"

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
 *	- print something on the console, callable at interrupt time
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
				if (putc == grputchar)
					tx_repaint(1);
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
			return;
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
	if (con_putchar == grputchar)
		tx_repaint(1);
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

/*
 * grputchar:
 *	- output a character to the screen at the current location
 *	- if the hardware is in use, save the data in the kgr_stash buffer,
 *	  which will be looked at later for potential delayed output
 */
grputchar(c)
	int c;
{
	char b[1];

	b[0] = c;
	if (tx_lock(0)) {
		tx_addchars(0, b, 1);
		tx_unlock(0);
	} else
		tx_stash(b[0]);
}
