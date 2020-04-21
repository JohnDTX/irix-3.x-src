/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* typewriter driving table structure */

#define	NROFFCHARS	NCHARS	/* ought to be dynamic */

extern struct t {
	int	bset;		/* these bits have to be on */
	int	breset;		/* these bits have to be off */
	int	Hor;		/* #units in minimum horiz motion */
	int	Vert;		/* #units in minimum vert motion */
	int	Newline;	/* #units in single line space */
	int	Char;		/* #units in character width */
	int	Em;		/* ditto */
	int	Halfline;	/* half line units */
	int	Adj;		/* minimum units for horizontal adjustment */
	char	*twinit;	/* initialize terminal */
	char	*twrest;	/* reinitialize terminal */
	char	*twnl;		/* terminal sequence for newline */
	char	*hlr;		/* half-line reverse */
	char	*hlf;		/* half-line forward */
	char	*flr;		/* full-line reverse */
	char	*bdon;		/* turn bold mode on */
	char	*bdoff;		/* turn bold mode off */
	char	*iton;		/* turn italic mode on */
	char	*itoff;		/* turn italic mode off */
	char	*ploton;	/* turn plot mode on */
	char	*plotoff;	/* turn plot mode off */
	char	*up;		/* sequence to move up in plot mode */
	char	*down;		/* ditto */
	char	*right;		/* ditto */
	char	*left;		/* ditto */

	char	*codetab[NROFFCHARS-128];
	char	width[NROFFCHARS];
} t;
