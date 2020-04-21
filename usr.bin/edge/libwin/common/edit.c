#include <gl.h>
#include <device.h>
#include <fcntl.h>
#include <stdio.h>
#include "tf.h"

textframe	*tf;
textview	*tv;
int		toprow;

void		readtext();

main(argc,argv)
int argc;
char **argv;
{
	short val;
	int xsize, ysize;

	if (argc < 2) {
		fprintf(stderr,"usage: edit file\n");
		exit(1);
	}

	if (getenv("DEBUG"))
		foreground();
	winopen("tv");
	tf = tfnew();
	tfsetlooks(tf, (0 << LOOKS_FONTSHIFT) |
		       (0 << LOOKS_FGSHIFT) |
		       (7 << LOOKS_BGSHIFT));
	readtext(tf, argv[1]);

	tv = tvnew(tf);
	getsize(&xsize, &ysize);
	tvviewsize(tv, xsize, ysize);
	tvsetbg(tv, 2);
	tvdraw(tv,1);
	tvtoprow(tv,0);
	toprow = 0;

	qdevice(LEFTMOUSE);
	qdevice(MIDDLEMOUSE);
	qdevice(MENUBUTTON);
	for (;;) {
		switch(qread(&val)) {
		  case REDRAW:
			getsize(&xsize, &ysize);
			viewport(0, xsize - 1, 0, ysize - 1);
			ortho2(-0.5, xsize - 1 + 0.5, -0.5, ysize + 0.5);
			tvviewsize(tv, xsize, ysize);
			tvdraw(tv,1);
			break;

		  case LEFTMOUSE:
			if (val)
				start_setpoint();
			break;
		  case MIDDLEMOUSE:
			break;
		  case MENUBUTTON:
			tfdumptext(tf);
			exit();
			break;
		}
	}
}

int	startx, starty;
int	finishx, finishy;

start_setpoint()
{
	startx = getvaluator(MOUSEX);
	starty = getvaluator(MOUSEY):
	if (setpoint())
		return;

	finishx = startx;
	finishy = starty;
	while (getbutton(LEFTMOUSE)) {
		setmark();
	}
}

finish_setpoint()
{
	finishx = getvaluator(MOUSEX);
	finishx = getvaluator(MOUSEY):
	setmark();
}

/*
 * Figure out where the point belongs.
 */
setpoint()
{
	long row, col;
	long x, y;

	tvpixtopos(tv, startx, starty, &row, &col);
	if (row < 0) {
		write(1, "\007", 1);
		while (getbutton(LEFTMOUSE))
			;
		return (1);
	}
	tfsetpoint(tf, row, col);
	tvpostopix(tv, row, col, &x, &y);
	rectfi(x, y, x + 5, y + 5);
	return (0);
}

void
readtext(tf, fn)
	textframe *tf;
	char *fn;
{
	int fd;
	char buf[4096];
	int nb;

	/* open file */
	if ((fd = open(fn, O_RDONLY)) < 0) {
		perror("edit");
		exit(-1);
	}

	/* copy file to textframe */
	for (;;) {
		nb = read(fd, buf, sizeof(buf));
		if (nb <= 0)
			break;
		if (tfputascii(tf, buf, nb))
			abort();
	}
	close(fd);
}
