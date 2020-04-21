static char *sccsid = "@(#)strings.c	4.1 (Berkeley) 10/1/80";
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/bin/RCS/strings.c,v 1.1 89/03/27 14:51:09 root Exp $";
/*
 * $Log:	strings.c,v $
 * Revision 1.1  89/03/27  14:51:09  root
 * Initial check-in for 3.7
 * 
 * Revision 1.3  85/01/10  18:38:37  bruce
 * Added mips specific stuff for searching through a.out files for data.
 * 
 * Revision 1.2  85/03/09  17:04:38  bob
 * Put in new strings that uses new symbol table format.
 * Changed %7D to %7ld.
 * 
 */
#include <stdio.h>
#include <a.out.h>
#include <ctype.h>

long	ftell();

/*
 * strings
 */

#ifdef SVR3
struct	aouthdr header;
struct	filehdr fileheader;
#else
struct	exec header;
#endif SVR3

char	*infile = "Standard input";
int	oflg;
int	asdata;
long	offset;
int	minlength = 4;

main(argc, argv)
	int argc;
	char *argv[];
{

	argc--, argv++;
	while (argc > 0 && argv[0][0] == '-') {
		register int i;
		if (argv[0][1] == 0)
			asdata++;
		else for (i = 1; argv[0][i] != 0; i++) switch (argv[0][i]) {

		case 'o':
			oflg++;
			break;

		case 'a':
			asdata++;
			break;

		default:
			if (!isdigit(argv[0][i])) {
				fprintf(stderr, "Usage: strings [ -a ] [ -o ] [ -# ] [ file ... ]\n");
				exit(1);
			}
			minlength = argv[0][i] - '0';
			for (i++; isdigit(argv[0][i]); i++)
				minlength = minlength * 10 + argv[0][i] - '0';
			i--;
			break;
		}
		argc--, argv++;
	}
	do {
		if (argc > 0) {
			if (freopen(argv[0], "r", stdin) == NULL) {
				perror(argv[0]);
				exit(1);
			}
			infile = argv[0];
			argc--, argv++;
		}
		fseek(stdin, (long) 0, 0);
#ifdef SVR3
		if (asdata ||
		    fread((char *)&fileheader, sizeof fileheader, 1, stdin) != 1 || 
		    fread((char *)&header, sizeof header, 1, stdin) != 1 || 
		    N_BADMAG(header)) {
			fseek(stdin, (long) 0, 0);
			find((long) 100000000L);
			continue;
		}
		fseek(stdin, (long) N_TXTOFF(fileheader, header)+header.tsize, 1);
		find((long) header.dsize);
#else
		if (asdata ||
		    fread((char *)&header, sizeof header, 1, stdin) != 1 || 
		    N_BADMAG(header)) {
			fseek(stdin, (long) 0, 0);
			find((long) 100000000L);
			continue;
		}
		fseek(stdin, (long) N_TXTOFF(header)+header.a_text, 1);
		find((long) header.a_data);
#endif SVR3
	} while (argc > 0);
}

find(cnt)
	long cnt;
{
	static char buf[BUFSIZ];
	register char *cp;
	register int c, cc;

	cp = buf, cc = 0;
	for (; cnt != 0; cnt--) {
		c = getc(stdin);
		if (c == '\n' || dirt(c) || cnt == 0) {
			if (cp > buf && cp[-1] == '\n')
				--cp;
			*cp++ = 0;
			if (cp > &buf[minlength]) {
				if (oflg)
					printf("%7ld ", ftell(stdin) - cc - 1);
				printf("%s\n", buf);
			}
			cp = buf, cc = 0;
		} else {
			if (cp < &buf[sizeof buf - 2])
				*cp++ = c;
			cc++;
		}
		if (ferror(stdin) || feof(stdin))
			break;
	}
}

dirt(c)
	int c;
{

	switch (c) {

	case '\n':
	case '\f':
		return (0);

	case 0177:
		return (1);

	default:
		return (c > 0200 || c < ' ');
	}
}
