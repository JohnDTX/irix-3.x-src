/*
 * Bind a function key with a given ascii string.  The "echo" style of
 * escape sequences are supported for newlines and such
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/gsh/RCS/bindkey.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:46:25 $
 */
#include "stdio.h"
#include "signal.h"
#include "termio.h"

extern	int errno;

char	*progname;

#define	PF1	'1'
#define	PF2	'2'
#define	PF3	'3'
#define	PF4	'4'

struct	keyname {
	char	*username;
	int	keycode;
};

struct	keyname keynames[] = {
	{ "pf1", PF1 },
	{ "pf2", PF2 },
	{ "pf3", PF3 },
	{ "pf4", PF4 },
};
#define	NKEYNAMES	(sizeof(keynames) / sizeof(struct keyname))

/*
 * Print out the legal key names
 */
void
printkeys()
{
	register int i;

	fprintf(stderr, "Legal key names are:\n");
	for (i = 0; i < NKEYNAMES; i++) {
		if (i) {
			if ((i % 6) == 0)
				fprintf(stderr, "\n");
			else
				fprintf(stderr, "\t");
		}
		fprintf(stderr, "%s", keynames[i].username);
	}
	fprintf(stderr, "\n");
}

/*
 * See if the given key name is a known key name
 */
int
check_key(name)
	char *name;
{
	register int i;

	for (i = 0; i < NKEYNAMES; i++)
		if (strcmp(keynames[i].username, name) == 0)
			return (keynames[i].keycode);

	/* oops, unknown key name */
	fprintf(stderr, "%s: unknown key name\n", progname);
	printkeys();
	exit(-1);
}

/*
 * Convert argument string into ascii binary, filling in supplied buffer.
 * Return the new length.
 */
int
convert(to, string)
	register char *to;
	register char *string;
{
	register int newlen;
	register char c;
	register int num;

	newlen = 0;
	while (c = *string++) {
		if (c == '\\') {
			c = *string++;
			switch (c) {
			  case '\000':			/* oops */
				*to++ = '\\';
				newlen++;
				return (newlen);
			  case 'b':			/* backspace */
				*to++ = '\b';
				newlen++;
				break;
			  case 't':			/* tab */
				*to++ = '\t';
				newlen++;
				break;
			  case 'n':			/* new line */
				*to++ = '\n';
				newlen++;
				break;
			  case 'r':			/* carriage return */
				*to++ = '\r';
				newlen++;
				break;
			  case '\\':			/* backslash */
				*to++ = '\\';
				newlen++;
			  case '0':			/* number to convert */
				num = 0;
				for (;;) {
					c = *string++;
					if ((c < '0') || (c > '7')) {
						string--;
						break;
					}
					num = num * 8 + (c - '0');
				}
				*to++ = num;
				newlen++;
				break;
			}
		} else {
			*to++ = c;
			newlen++;
		}
	}
	return (newlen);
}

/*
 * Bind key
 */
void
bind_key(keycode, string)
	int keycode;
	char *string;
{
	char buf[80];
	char used[128];
	register int len;
	register int i;
	struct termio save_tcb, new_tcb;

	if (strlen(string) > sizeof(buf)) {
		fprintf(stderr, "%s: string is too long\n", progname);
		exit(-1);
	}

	/*
	 * Convert string into binary ascii, converting \X into appropriate
	 * ascii code.
	 */
	len = convert(&buf[4], string);

	/*
	 * Make a quick tag in the used map of all the ascii characters
	 * that the key binding uses.  Then pick a code that isn't
	 * being used for the terminator character to send.
	 */
	bzero(used, sizeof(used));
	used['\n'] = 1;
	used['\r'] = 1;
	for (i = 0; i < len; i++)
		used[buf[i+4]] = 1;
	for (i = 1; i < sizeof(used); i++) {
		if (used[i] == 0) {
			buf[0] = '\033';
			buf[1] = 'k';
			buf[2] = keycode;
			buf[3] = i;
			buf[4 + len] = i;
			/*
			 * Now setup terminal to be in raw mode, then
			 * send the escape sequence
			 */
			signal(SIGINT, SIG_IGN);
			signal(SIGQUIT, SIG_IGN);
			ioctl(0, TCGETA, &save_tcb);
			ioctl(0, TCGETA, &new_tcb);
			new_tcb.c_iflag = 0;
			new_tcb.c_oflag = 0;
			new_tcb.c_lflag = 0;
			new_tcb.c_cc[VEOF] = 0;
			new_tcb.c_cc[VEOL] = 0;
			ioctl(0, TCSETA, &new_tcb);
			write(1, buf, len + 5);
			ioctl(0, TCSETA, &save_tcb);
			return;
		}
	}
	fprintf(stderr, "%s: can't bind key, too complex\n", progname);
	exit(-1);
}

/*
 * Print all the current key bindings
 */
printkeybindings()
{
	register int i;

	for (i = 0; i < NKEYNAMES; i++)
		printkeybinding(keynames[i].username);
}

/*
 * Print a given keys binding
 */
printkeybinding(name)
	char *name;
{
	register int i;
	register int key, nb;
	int (*ov1)(), (*ov2)();
	struct termio save_tcb, new_tcb;
	char buf[100];

	key = check_key(name);
	buf[0] = '\033';
	buf[1] = 'r';
	buf[2] = key;

	ov1 = signal(SIGINT, SIG_IGN);
	ov2 = signal(SIGQUIT, SIG_IGN);

	ioctl(0, TCGETA, &save_tcb);
	ioctl(0, TCGETA, &new_tcb);
	new_tcb.c_iflag = 0;
	new_tcb.c_lflag = 0;
	new_tcb.c_cc[VEOF] = 0;
	new_tcb.c_cc[VEOL] = 0;
	ioctl(0, TCSETA, &new_tcb);
	write(1, buf, 3);

	/*
	 * Now read value from terminal emulator
	 */
	while ((nb = read(0, buf, sizeof(buf))) == 0)
		;
	ioctl(0, TCSETA, &save_tcb);
	(void) signal(SIGINT, ov1);
	(void) signal(SIGQUIT, ov2);

	/*
	 * Check result of read now that tty state is restored
	 */
	if (nb <= 0) {
		int save_errno;

		save_errno = errno;
		fprintf(stderr, "%s: read of function key failed\n", progname);
		errno = save_errno;
		perror(progname);
		exit(-1);
	}

	/*
	 * Display key binding in a nice way, so that it could be
	 * fed back into this program.
	 * Skip over leader (<escape>k<key><char>) and trailer (<char>).
	 */
	printf("%s ", name);
	for (i = 4; i < nb - 1; i++) {
		if ((buf[i] < ' ') || (buf[i] >= 127)) {
			switch (buf[i]) {
			  case '\b':
				printf("%cb", '\\');
				break;
			  case '\t':
				printf("%ct", '\\');
				break;
			  case '\r':
				printf("%cr", '\\');
				break;
			  case '\n':
				printf("%cn", '\\');
				break;
			  case '\\':
				printf("%c%c", '\\', '\\');
				break;
			  default:
				printf("\\%03o", buf[i]);
				break;
			}
		} else
			printf("%c", buf[i]);
	}
	printf("\n");
}

/*
 * Main
 */
main(argc, argv)
	int argc;
	char *argv[];
{
	progname = argv[0];
	if (argc == 1)
		printkeybindings();
	else
	if (argc == 2)
		printkeybinding(argv[1]);
	else
	if (argc == 3)
		bind_key(check_key(argv[1]), argv[2]);
	else {
		fprintf(stderr, "usage: %s key string\n", progname);
		exit(-1);
	}
}
