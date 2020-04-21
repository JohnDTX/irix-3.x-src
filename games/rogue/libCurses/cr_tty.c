static	char	*Cr_tty_c	= "@(#)cr_tty.c	1.9";
/*
 * Terminal initialization routines.
 *
 * 5/15/81 (Berkeley) @(#)cr_tty.c	1.3
 */

# undef	DEBUG

# include	"curses.ext"
# include	"cr_ex.h"

static bool	*sflags[]	= {
			&AM, &BS, &EO, &HZ, &IN, &MI, &MS, &NC, &OS, &UL, &XN
		};

static char	*xPC,
		**sstrs[]	= {
			&AL, &BC,  &BT, &CD, &CE, &CL, &CM, &DC, &DL,
			&DM, &DO,  &ED, &EI, &HO, &IC, &IM, &IP, &LL,
			&MA, &ND, &xPC, &SE, &SF, &SO, &SR, &TA, &TE,
			&TI, &UC,  &UE, &UP, &US, &VB, &VS, &VE,
			&CZ, &CP,		/* color extensions */
			&AS, &AE, &GS, &GE,	/* vt100 alt char sets ext. */
			&Al, &Nb, &Nm,		/* Cobol extensions */
			&CF, &CN		/* Cursor on/off */
		},
		*tgoto();
int	CT;				/* Color Terminal type */

/* tspace boosted to 200 from 128 - 03/20/83 */
static char	tspace[200],		/* Space for capability strings */
		*aoftspace;		/* Address of tspace for relocation */

static int	destcol, destline;

/*
 *	This routine does terminal type initialization routines, and
 * calculation of flags at entry.  It is almost entirely stolen from
 * Bill Joy's ex version 2.6.
 */

gettmode() {

	savetty();
#ifndef	V7
#ifdef DEBUG
	UPPERCASE = ( (_tty.c_iflag & IUCLC) && (_tty.c_lflag & XCASE)
		      && (_tty.c_oflag & OLCUC) ) != 0;
	GT = ((_tty.c_oflag & TAB3) == 0);
	NONL = ((_tty.c_iflag & ICRNL) && (_tty.c_oflag & OCRNL) ) == 0;
	fprintf(outf, "GETTMODE: UPPERCASE = %s\n", UPPERCASE ? "TRUE":"FALSE");
	fprintf(outf, "GETTMODE: GT = %s\n", GT ? "TRUE" : "FALSE");
	fprintf(outf, "GETTMODE: NONL = %s\n", NONL ? "TRUE" : "FALSE");
	fprintf(outf, "GETTMODE: ospeed = %d\n", ospeed);
#endif	DEBUG
#endif	V7
}

setterm(type)
reg char	*type; {

	reg int	unknown;
	char	genbuf[1024];

# ifdef DEBUG
	fprintf(outf, "SETTERM(\"%s\")\n", type);
	fprintf(outf, "SETTERM: LINES = %d, COLS = %d\n", LINES, COLS);
# endif
	if (type[0] == '\0')
		type = "xx";
	unknown = FALSE;
	if (tgetent(genbuf, type) != 1) {
		unknown++;
		strcpy(genbuf, "xx|dumb:");
	}
# ifdef DEBUG
	fprintf(outf, "SETTERM: tty = %s\n", type);
# endif
	if (LINES == 0)
		LINES = tgetnum("li");
	if (LINES <= 5)
		LINES = 24;
	else if (LINES > 48)
		LINES = 48;

	if (COLS == 0)
		COLS = tgetnum("co");
	if (COLS <= 4)
		COLS = 80;
	else if (COLS > 1000)
		COLS = 1000;
	SG = tgetnum("sg");
	CT = tgetnum("CT");
	if (CT < 0)
		CT = 0;
	if (CT == 0)
		CT = tgetflag("CT");
# ifdef DEBUG
	fprintf(outf, "SETTERM: LINES = %d, COLS = %d\n", LINES, COLS);
# endif
	aoftspace = tspace;
	zap();			/* get terminal description		*/
	if (tgoto(CM, destcol, destline)[0] == 'O')
		CA = FALSE, CM = 0;
	else
		CA = TRUE;
	PC = xPC ? xPC[0] : FALSE;
	aoftspace = tspace;
	strcpy(ttytype, longname(genbuf, type));
	if (unknown)
		return ERR;
	return OK;
}
/*
 *	This routine gets all the terminal flags from the termcap database
 */
zap() {

	reg bool	**fp;
	reg char	*namp, ***sp;
	int		i;	/* for colors */
	char		chr;
	extern char	*tgetstr();

	/*
	 * get boolean flags
	 */
 	namp = "ambseohzinmimsncosulxn\0\0";
# ifdef FULLDEBUG
	fprintf(outf, "ZAP: namp = \"%s\"\n", namp);
# endif
	fp = sflags;
	do {
		*(*fp++) = tgetflag(namp);
# ifdef FULLDEBUG
		fprintf(outf, "ZAP: %.2s = %d", namp, *(*(fp - 1)));
# endif
		namp += 2;
	} while (*namp);
			/* parse Map Chars before we use the string space */
	namp = getcap("MC");
			/* clear _mapchar for restartability */
	for (i=0; i < 128; )
		_mapchr[i++] = 0;
	if (namp)
		while (namp[0] && namp[1])
			if (namp[1] == '*' && namp[2]) {
				_mapchr[namp[0]] = namp[2] | 0200;
				namp += 3;
			} else {
				_mapchr[namp[0]] = namp[1];
				namp += 2;
			}

	/*
	 * get string values
	 */
	namp = "albcbtcdceclcmdcdldmdoedeihoicimipllmandpcsesfsosrtatetiucueupusvbvsveCZCPASAEGSGEALNBNMCFCN";
# ifdef FULLDEBUG
	fprintf(outf, "ZAP: namp = \"%s\"\n", namp);
# endif
	sp = sstrs;
	do {
		*(*sp++) = tgetstr(namp, &aoftspace);
# ifdef FULLDEBUG
		fprintf(outf, "ZAP: %.2s = \"%s\"\n", namp, *(*(sp-1)));
# endif
		namp += 2;
	} while (*namp);
	if (!SO && US) {
		SO = US;
		SE = UE;
	}
			/* handle colors */
	namp = CZ;	/* list of color names */
	Dforeground = 0;
	Dbackground = 0;
	Iforeground = -1;
	Ibackground = -1;
	if (namp) {
		Dforeground = -1;
		Dbackground = -1;
		for (i=0; i<_MAXCOLORS && *namp; i++) {
			if (*namp == '*') {	/* fore/background color spec */
				switch (*++namp) {
				  case 'B':
				  case 'b':
					Dbackground = i;
						/*
						 * check for spec that fore-
						 * ground can't be default
						 * background color
						 */
					if (namp[1] == '-') {
						Iforeground = i;
						namp++;
					}
					break;
				  case 'F':
				  case 'f':
					Dforeground = i;
					if (namp[1] == '-') {
						Ibackground = i;
						namp++;
					}
					break;
				  case 'I':
				  case 'i':
					Dforeground = i;
					break;
				}
				namp++;
			}
			Colors[i] = namp;	/* have color # point to name */
			while (*++namp)
				if (*namp == ',') {
					*namp = '\0';
					namp++;
					Ncolors = i + 2;
					break;
				}
		}
	}
}

/*
 * get a string capability from the entry
 */
char *
getcap(name)
char *name;
{
	return tgetstr(name, &aoftspace);
}
