/* gethostident -- report value of gethostident call
 *
 * gethostident makes the system call and prints the value; the "-a" option
 * will cause the value to be printed in escaped octal when the character is
 * not printable or is an escape, and adds a newline.
 */

#include <ctype.h>
#include <stdio.h>

main (argc, argv)
	int		argc;
	char		**argv;
{
	char		*p, addr [64];
	int		c, ascii, octal;

	if (gethostident (addr) < 0) {
		perror (argv [0]);
		exit (1);
	}
	while ((c = getopt (argc, argv, "bc")) != EOF) {
		switch (c) {
		case 'b': ++octal; break;
		case 'c': ++ascii; break;
		default:
			fprintf (stderr, "usage: %s [ -ao ]\n", argv [0]);
			exit (1);
		}
	}
	for (p = addr, c = 0; p < addr + 64; ++p, ++c) {
		if (ascii && (!isprint (*p) || *p == '\\') || octal) {
			if (ascii) putchar ('\\');
			else if (c % 16) putchar (' ');
			putchar ((*p >> 6 & 03) + '0');
			putchar ((*p >> 3 & 07) + '0');
			putchar ((*p & 07) + '0');
		} else {
			putchar (*p);
		}
		if (octal && c % 16 == 15) putchar ('\n');
	}
	if (ascii) putchar ('\n');
	exit (0);
}
