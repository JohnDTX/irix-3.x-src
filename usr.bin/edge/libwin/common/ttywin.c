#include "tf.h"
#include "te.h"
#include "lw.h"
#include "shell.h"
#include "device.h"

textview *tty_view;
textframe *tty_frame;
termulator *tty_emulator;

main()
{
	int xsize, ysize;
	extern char *getenv();

	if (getenv("DEBUG"))
		foreground();
	winopen("tty");
	tty_frame = tfnew();
	tty_view = tvnew(tty_frame);
	tty_emulator = tenew(tty_view);
	if (getenv("HOME"))
		chdir(getenv("HOME"));
	if (!shnew(tty_emulator, 1, "/bin/csh", "-csh", "-i", 0)) {
		printf("can't start shell\n");
		exit(-1);
	}

	getsize(&xsize, &ysize);
	tvviewsize(tty_view, xsize, ysize);
	tvdraw(tty_view, 1);

	kbstart();		/* start keyboard process going */
	lwrun();
}
