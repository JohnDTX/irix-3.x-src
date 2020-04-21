#include "gl.h"

main ()
{
	int i;

	ginit();
	color(BLACK);
	clear();

	/* create a magenta color ramp */
	for (i = 0; i < 128; i++)
		mapcolor(i, 2*i, 0, 2*i);

	setshade(0);  		/* set the intensity for the first vertex */
	pmv(100.0,100.0,0.0);  	/* specify the first vertex */
	setshade(127); 		/* set the intensity for the second vertex */
	pdr(200.0,200.0,0.0);  	/* specify the second vertex */
	setshade(64);  		/* set the intensity for the third vertex */
	pdr(200.0,100.0,0.0);  	/* specify the third vertex */
	spclos();  		/* close the shaded polygon */

	sleep(5);
	greset();
	gexit();
}
