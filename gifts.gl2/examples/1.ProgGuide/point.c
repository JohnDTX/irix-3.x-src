#include "gl.h"

main()
{
	int i,j;


	ginit();
	cursoff();  /* turn off cursor so it doesn't interfere with drawing */
	color(BLACK); 	/* make BLACK the current drawing color */
	clear(); 	/* clear the screen (to black) */
	color(BLUE);  	/* make BLUE the current drawing color */

	for (i=0; i<10; i=i+1) {
		for (j=0; j<10; j=j+1)
			pnti(i*5,j*5,0);
	}

	sleep(5);
	gexit();
}
