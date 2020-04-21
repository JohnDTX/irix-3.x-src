#include "gl.h"
#include "tf.h"
#include "fcntl.h"
#include "stdio.h"

textframe *tf;
textview *tv;
char asciibuf[20000];
long textbuf[20000];

long point_row, point_col;
long mark_row, mark_col;
int cutfd;

/* handles */
#define	FG_RED		10
#define	FG_BLUE		11
#define	BG_WHITE	20
#define	BG_GREEN	21
#define	BG_BLACK	22
#define	FNT_ONE		1
#define	FNT_TWO		2

main()
{
	register int i;
	char buf[1000];
	int xsize, ysize;
	int nb_ascii, nb_text, nb_select;

	if ((cutfd = open("/tmp/cut", O_RDWR|O_CREAT, 0644)) < 0) {
		printf("getit: couldn't create cut file\n");
		exit(-1);
	}

	foreground();
	minsize(150, 150);
	winopen("getit");

	tf = tfnew();
	tv = tvnew(tf);
	if (!tf || !tv) {
		printf("getit: couldn't get textframe/textview\n");
		exit(-1);
	}

	/* map color handles to indexes */
	tvmapindex(tv, FG_RED, RED);
	tvmapindex(tv, FG_BLUE, BLUE);
	tvmapindex(tv, BG_WHITE, WHITE);
	tvmapindex(tv, BG_GREEN, GREEN);
	tvmapindex(tv, BG_BLACK, BLACK);

	/* map font handles to given file names */
	tvmapfont(tv, FNT_ONE, "mel.fnt");
	tvmapfont(tv, FNT_TWO, "smel.fnt");

	/*
	 * Build a 24 line frame with predictable text.
	 */
	tvsetbg(tv, BG_GREEN);
	for (i = 0; i < 6; i++) {
		sprintf(buf, "this!is#line@%d\n", i);
		if (i & 1)
			tfsetlooks(tf, (FNT_ONE << LOOKS_FONTSHIFT) |
				       (FG_RED << LOOKS_FGSHIFT) |
				       (BG_WHITE << LOOKS_BGSHIFT));
		else
			tfsetlooks(tf, (FNT_TWO << LOOKS_FONTSHIFT) |
				       (FG_BLUE << LOOKS_FGSHIFT) |
				       (BG_WHITE << LOOKS_BGSHIFT));
		tfputascii(tf, buf, strlen(buf));
		tfsplit(tf);
	}

	getsize(&xsize, &ysize);
	tvviewsize(tv, xsize, ysize);
	tvtoprow(tv, 0);

	for (;;) {
		tvdraw(tv, 1);

		getrowcol("Point", &point_row, &point_col);
		getrowcol("Mark", &mark_row, &mark_col);

		tfsetpoint(tf, point_row, point_col);
		tfsetmark(tf, mark_row, mark_col);
		nb_ascii = tfgetascii(tf, asciibuf, sizeof(asciibuf));
		nb_select = tfselcount(tf);
		nb_text = tfgettext(tf, textbuf, sizeof(textbuf));
	    printf("getascii=%d gettext=%d selcount=%d *ascii='%c' *text=%x\n",
				nb_ascii, nb_text, nb_select,
				asciibuf[0], textbuf[0]);

		/* copy cut data to file */
		lseek(cutfd, 0L, 0);
		ftruncate(cutfd, 0L);
		write(cutfd, asciibuf, nb_ascii);

		/*
		 * Now delete the region we just selected, and then put
		 * it back in.
		 */
		printf("Ready to delete?");
		gets(buf);
		tfsetpoint(tf, point_row, point_col);
		tfsetmark(tf, mark_row, mark_col);
		tfdelete(tf);
		tvdraw(tv, 1);

		printf("Ready to see it put back in?");
		gets(buf);
		tfsetpoint(tf, point_row, point_col);
		stufftext(tf, textbuf, nb_text, 0);
		tvdraw(tv, 1);

		printf("Ready to see it changed?");
		gets(buf);
		tfsetpoint(tf, point_row, point_col);
		tfsetmark(tf, mark_row, mark_col);
		tfsetwritemask(tf, LOOKS_BG);
		tfsetlooks(tf, (BG_BLACK << LOOKS_BGSHIFT));
		tfchangelooks(tf);
		tvdraw(tv, 1);

		printf("Ready to undo that?");
		gets(buf);
		tfsetpoint(tf, point_row, point_col);
		stufftext(tf, textbuf, nb_text, 1);
		tvdraw(tv, 1);

		printf("Ready to see the text highlighted?");
		gets(buf);
		tfsetpoint(tf, point_row, point_col);
		tfsetmark(tf, mark_row, mark_col);
		tvhighlight(tv, 9, 1);
		tfsetwritemask(tf, LOOKS_SELECT);
		tfsetlooks(tf, LOOKS_SELECT);
		tfchangelooks(tf);
		tvdraw(tv, 1);

		printf("Ready to see the text un-highlighted?");
		gets(buf);
		tfsetpoint(tf, point_row, point_col);
		tfsetmark(tf, mark_row, mark_col);
		tfsetwritemask(tf, LOOKS_SELECT);
		tfsetlooks(tf, 0);
		tfchangelooks(tf);
		tvdraw(tv, 1);

		color(7);
		clear();
		for (i = -52; i < 52; i++) {
			tvleftpix(tv, -i);
			tvdraw(tv, 1);
			sginap(5);
		}

		printf("Ready to center text?");
		gets(buf);
		tvleftpix(tv, 0);
	}
}

    /*
     * stuff long text data, line by line, into a textframe,
     * without performing wrapping.
     */
stufftext(tf, buffer, count, how)
    textframe *tf;
    long *buffer;
    long count;
    int how;
{
    long *linebuf = buffer;
    long linecount = 0;
    long l;
    textcoord begin;
    long row, col;

    tfgetpoint(tf, &begin.tc_row, &begin.tc_col);
    row = begin.tc_row;
    col = begin.tc_col;
    while (count--) {
	l = *buffer++;
	linecount++;
	if ((l & LOOKS_INDEX) == '\n') {
	    if (how == 0) {
		tfputtext(tf, linebuf, linecount);
		tfsplit(tf);
	    } else {
		tfsetpoint(tf, row, col);
		tfsetmark(tf, row, col + linecount);
		tfdelete(tf);
		tfputtext(tf, linebuf, linecount);
		row++;
		col = 0;
	    }
	    linebuf = buffer;
	    linecount = 0;
	}
    }
    if (linecount) {
	if (how == 0)
	    tfputtext(tf, linebuf, linecount);
	else {
	    tfsetpoint(tf, row, col);
	    tfsetmark(tf, row, col + linecount);
	    tfdelete(tf);
	    tfputtext(tf, linebuf, linecount);
	}
    }
    tfsetmark(tf, begin.tc_row, begin.tc_col);
}

getrowcol(msg, rowp, colp)
	char *msg;
	int *rowp, *colp;
{
	char buf[1000];

	printf("%s Row: ", msg);
	gets(buf);
	*rowp = atoi(buf);
	printf("%s Col: ", msg);
	gets(buf);
	*colp = atoi(buf);
}
