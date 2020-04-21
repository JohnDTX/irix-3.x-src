/*
 * GSH Terminal emulator:
 *
 * S1|wsiris|iris40|iris emulating a 40 line visual 50 (approximately):\
 *	:am:
 *	:al=\EL:
 *	:bs:
 *	:cd=\EJ:
 *	:ce=\EK:
 *	:cl=\Ev:
 *	:cm=\EY%+ %+ :
 *	:co#80:li#40:
 *	:dl=\EM:
 *	:ho=\EH:
 *	:is=\E7B0\E7F7\E7C2\E7R3:
 *	:ku=\EA:kd=\EB:kr=\EC:kl=\ED:
 *	:k0=\E0:k1=\E1:k2=\E2:k3=\E3:k4=\E4:k5=\E5:k6=\E6:k7=\E7:k8=\E8:k9=\E9:
 *	:nd=\EC:
 *	:pt:
 *	:so=\E9P:se=\E0@:
 *	:sr=\EI:
 *	:up=\EA:
 *	:us=\E7R2\E9P:ue=\E7R3\E0@:
 *	:vs=\E;:ve=\E>:
 *
 * XXX finish out all possible termcap/terminfo capabilities and support them
 * XXX all!  A full bodied full figured terminal emulator...
 */

#include "tf.h"
#include "te.h"
#include "message.h"

/* te_mode's */
#define	TEMODE_NORMAL	0
#define	TEMODE_ESCAPE	1
#define	TEMODE_9A	2
#define	TEMODE_0A	4
#define	TEMODE_YA	6
#define	TEMODE_YB	7
#define	TEMODE_7A	8
#define	TEMODE_7B	9
#define	TEMODE_TITLEA	10
#define	TEMODE_TITLEB	11
#define	TEMODE_BINDKEYA	12
#define	TEMODE_BINDKEYB	13
#define	TEMODE_BINDKEYC	14
#define	TEMODE_GETBINDINGA	15
#define	TEMODE_GETBINDINGB	16

/*
 * Set the given keys binding
 */
void
tesetbinding(te, key, string)
	termulator *te;
	char key;
	char *string;
{
	/* sendEvent(KBBINDKEY, something-useful-goes-here); */
}

/*
 * Get the given keys binding
 */
void
tegetbinding(te, key)
	termulator *te;
	char key;
{
}

/*
 * Put a string of data into the termulator.  Overwrite the old contents.
 */
void
te_putdata(te, cp, len)
	register termulator *te;
	char *cp;
	int len;
{
	register textframe *tf;
	int row;
	int numcols;

	tf = te->te_tf;
	row = te->te_toprow + te->te_row;
	if (row >= tfnumrows(tf))
		tfmakeline(tf, row);		/* make line exist */
	numcols = tfnumcols(tf, row);		/* find line length */

	if (numcols < te->te_col) {
		char *spaces;

		/* line is too short.  pad it out */
		spaces = (char *) malloc(te->te_col - numcols);
		memset(spaces, ' ', te->te_col - numcols);
		tfsetpoint(tf, row, numcols);
		tfputascii(tf, spaces, te->te_col - numcols);
	} else {
		/* line is long enough; delete old data, if any */
		tfsetpoint(tf, row, te->te_col);
		tfsetmark(tf, row, te->te_col + len);
		tfdelete(tf);
	}

	tfsetpoint(tf, row, te->te_col);	/* XXX paranoia */
	tfputascii(tf, cp, len);		/* add in new data */
}

/*
 * Given a string of characters, interpret them and cause changes in
 * the underlying textframe.  This procedure returns status indicating if
 * a repaint is needed.
 */
int
termulate_iris(te, cp, bytes)
	register termulator *te;
	unsigned char *cp;
	int bytes;
{
	register unsigned char c;
	register unsigned char *savecp;
	textframe *tf;
	unsigned char *normalcp;
	int savelen;
	int newcol;
	long l;

/*
printf("row=%d col=%d msg=\"%s\"\n", te->te_row, te->te_col, cp);
*/
	savelen = 0;
	tf = te->te_tf;
	while (--bytes >= 0) {
		c = *cp;
		switch (te->te_mode) {
		  case TEMODE_NORMAL:
			if (c < ' ') {
				if (savelen) {
					te_putdata(te, savecp, savelen);
					temoveto(te, te->te_row,
						     te->te_col + savelen);
					savelen = 0;
				}
				switch (c) {
#ifdef	DEBUG
				  case '\01':
					tedebug(te);
					break;
#endif
				  case '\033':
					te->te_mode = TEMODE_ESCAPE;
					break;
				  case '\b':
					if (te->te_col)
						temoveto(te, te->te_row,
							     te->te_col-1);
					break;
				  case '\007':
					ringbell();
					break;
				  case '\n':
					temoveto(te, te->te_row+1, te->te_col);
					break;
				  case '\r':
					temoveto(te, te->te_row, 0);
					break;
				  case '\t':
					newcol = te->te_col +
							(8 - (te->te_col&7));
					if (newcol >= te->te_cols)
						temoveto(te, te->te_row+1, 0);
					else
						temoveto(te, te->te_row,
							     newcol);
					break;
				}
			} else {
				/*
				 * Just remember normal printing characters,
				 * and display them later in one te_putdata.
				 * This improves performance quite a bit
				 * since most characters are normal printing
				 * ones.  Note that we have to check to see
				 * if the saved characters would cause some
				 * sort of automatic cursor motion, like
				 * the automatic margin.
				 */
				ASSERT(te->te_col < te->te_cols);
				if (savelen == 0)
					savecp = cp;
				savelen++;
				if (te->te_col + savelen >= te->te_cols) {
					/*
					 * Data up to current character must
					 * be output, because the next one
					 * causes the cursor to automatically
					 * scroll down and to the left margin
					 */
					if (savelen) {
						te_putdata(te, savecp, savelen);
						savelen = 0;
					}
					temoveto(te, te->te_row+1, 0);
				}
			}
			break;
		  case TEMODE_ESCAPE:
			switch (c) {
			  case 'J':		/* clear to end-of-frame */
				tecleartoeof(te);
				te->te_mode = TEMODE_NORMAL;
				break;
			  case 'K':		/* clear to end-of-line */
				tecleartoeol(te);
				te->te_mode = TEMODE_NORMAL;
				break;
			  case 'v':		/* home and clear screen */
				temoveto(te, 0, 0);
				tecleartoeof(te);
				te->te_mode = TEMODE_NORMAL;
				break;
			  case 'L':		/* insert line */
				teinsertln(te);
				te->te_mode = TEMODE_NORMAL;
				break;
			  case 'M':		/* delete line */
				tedeleteln(te);
				te->te_mode = TEMODE_NORMAL;
				break;
			  case 'H':		/* home */
				temoveto(te, 0, 0);
				te->te_mode = TEMODE_NORMAL;
				break;
			  case 'I':		/* reverse scroll */
				if (te->te_row == 0)
					teinsertln(te);
				else
					temoveto(te, te->te_row-1, te->te_col);
				te->te_mode = TEMODE_NORMAL;
				break;
			  case 'A':		/* up */
				if (te->te_row)
					temoveto(te, te->te_row-1, te->te_col);
				te->te_mode = TEMODE_NORMAL;
				break;
			  case 'B':		/* down */
				if (te->te_row < te->te_rows)
					temoveto(te, te->te_row+1, te->te_col);
				te->te_mode = TEMODE_NORMAL;
				break;
			  case 'C':		/* right */
				if (te->te_col < te->te_cols)
					temoveto(te, te->te_row, te->te_col+1);
				te->te_mode = TEMODE_NORMAL;
				break;
			  case 'D':		/* left */
				if (te->te_col)
					temoveto(te, te->te_row, te->te_col-1);
				te->te_mode = TEMODE_NORMAL;
				break;
			  case '9':		/* set attribute */
				te->te_mode = TEMODE_9A;
				break;
			  case '0':		/* set attribute */
				te->te_mode = TEMODE_0A;
				break;
			  case 'Y':		/* move cursor */
				te->te_mode = TEMODE_YA;
				break;
			  case '7':		/* set color */
				te->te_mode = TEMODE_7A;
				break;
			  case 'T':		/* set title */
				te->te_mode = TEMODE_TITLEA;
				break;
			  case 'k':		/* set key binding */
				te->te_mode = TEMODE_BINDKEYA;
				break;
			  case 'r':		/* report key binding */
				te->te_mode = TEMODE_GETBINDINGA;
				break;
			  default:
				te->te_mode = TEMODE_NORMAL;
				break;
			}
			break;
		  case TEMODE_9A:
			tftextcolor(tf, 2);
			te->te_mode = TEMODE_NORMAL;
			break;
		  case TEMODE_0A:
			tftextcolor(tf, 0);
			te->te_mode = TEMODE_NORMAL;
			break;
		  case TEMODE_YA:			/* capture row */
			te->te_save[0] = c;
			te->te_mode = TEMODE_YB;
			break;
		  case TEMODE_YB:			/* capture col */
			te->te_mode = TEMODE_NORMAL;
			if ((c < 32) || (c >= (32 + te->te_cols)))
				break;
			if ((te->te_save[0] < 32) ||
			    (te->te_save[0] >= (32 + te->te_rows)))
				break;
			temoveto(te, te->te_save[0] - 32, c - 32);
			break;
		  case TEMODE_7A:
			te->te_save[0] = c;
			te->te_mode = TEMODE_7B;
			break;
		  case TEMODE_7B:
			c -= '0';
			switch (te->te_save[0]) {
			  case 'f': case 'F':		/* set foreground */
				te->te_fgcolor = c;
				tvmapindex(te->te_tv, 0, te->te_fgcolor);
				te->te_flags |= TE_SCROLLED;
				break;
			  case 'b': case 'B':		/* set background */
				te->te_bgcolor = c;
				tvmapindex(te->te_tv, 1, te->te_bgcolor);
				te->te_flags |= TE_SCROLLED;
				break;
			  case 'c': case 'C':		/* set cursor color */
				break;
			  case 'r': case 'R':		/* set reverse color */
				te->te_reversecolor = c;
				tvmapindex(te->te_tv, 2, te->te_reversecolor);
				te->te_flags |= TE_SCROLLED;
				break;
			}
			te->te_mode = TEMODE_NORMAL;
			break;
		  case TEMODE_TITLEA:			/* start title */
			/* remember title terminator character */
			te->te_save[0] = c;
			te->te_mode = TEMODE_TITLEB;
			te->te_savecount = 1;
			break;
		  case TEMODE_TITLEB:			/* finish title */
			/*
			 * If character is terminator character, finish title.
			 * If we have overflowed the save buffer, finish
			 * title.  If we get a new-line, finish the title.
			 */
			if ((c == te->te_save[0]) ||
			    (c == '\n') ||
			    (te->te_savecount >= sizeof(te->te_save))) {
				te->te_save[te->te_savecount] = 0;
				wintitle(&te->te_save[1]);
				te->te_mode = TEMODE_NORMAL;
			} else
				te->te_save[te->te_savecount++] = c;
			break;
		  case TEMODE_BINDKEYA:			/* set key binding */
			/* remember key to bind */
			te->te_save[0] = c;
			te->te_mode = TEMODE_BINDKEYB;
			break;
		  case TEMODE_BINDKEYB:			/* set key binding */
			/* remember delimiter for binding */
			te->te_save[1] = c;
			te->te_mode = TEMODE_BINDKEYC;
			te->te_savecount = 2;
			break;
		  case TEMODE_BINDKEYC:			/* set key binding */
			/*
			 * If character is terminator character, finish title.
			 * If we have overflowed the save buffer, finish
			 * title.  If we get a new-line, finish the title.
			 */
			if ((c == te->te_save[1]) ||
			    (c == '\n') ||
			    (te->te_savecount >= sizeof(te->te_save))) {
				te->te_save[te->te_savecount] = 0;
				tesetbinding(te, te->te_save[0],
						 &te->te_save[2]);
				te->te_mode = TEMODE_NORMAL;
			} else
				te->te_save[te->te_savecount++] = c;
			break;
		  case TEMODE_GETBINDINGA:		/* get key binding */
			te->te_save[0] = c;
			te->te_mode = TEMODE_GETBINDINGB;
			break;
		  case TEMODE_GETBINDINGB:		/* get key binding */
			tegetbinding(te, te->te_save[0]);
			te->te_mode = TEMODE_NORMAL;
			break;
		}
		cp++;
	}

	/* flush any lingering stuff */
	if (savelen) {
		te_putdata(te, savecp, savelen);
		temoveto(te, te->te_row, te->te_col+savelen);
	}
}

#ifdef	DEBUG
/*
 * print info about current line
 */
tedebug(te)
	register termulator *te;
{
	textline *tl;

	tl = tf_findline(te->te_tf, te->te_row, 0);
	if (tl) {
		if (!tl->tl_height)
			tv_findheight(te->te_tv, tl);
		if (!tl->tl_width)
			tv_findwidth(te->te_tv, tl);
		printf("%d: width=%d height=%d len=%d looks=%x\n",
			    te->te_row, tl->tl_width, tl->tl_height,
			    tl->tl_len, tl->tl_looks);
	} else
		printf("%d: no such line\n", te->te_row);
}
#endif
