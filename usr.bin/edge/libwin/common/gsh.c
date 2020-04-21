#include "graphlib.h"

main()
{
	int xsize, ysize;
	int s;

	foreground();
	winopen("tty-frame");
	getsize(&xsize, &ysize);
	s = NewPane(NewShellPane, xsize, ysize);
	lwrun();
	printf("bye bye\n");
}
