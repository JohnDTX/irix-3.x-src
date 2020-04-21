#include "gl.h"
#include "device.h"

#define PLANET 1

main()
{
	short type, val;
	register short buffer[50], cnt, i;
	float shipx, shipy, shipz;

	ginit();
	for (i = 0; i < 50; i++)
		buffer[i] = 0;
	qdevice(MOUSE3);
	cursoff();
	color(BLACK);
	clear();
	color(RED);
	/* create the planet object */
	createplanet(PLANET);
	callobj(PLANET);

	while (1) {
		type = qread(&val);
		if (val==0)
			continue;
		switch (type){
		case MOUSE3:
			/* set ship location to cursor location */
			shipz=0;
			shipx=getvaluator(MOUSEX);
			shipy=getvaluator(MOUSEY);
			/* draw the ship */
			color(BLUE);
			rect(shipx, shipy, shipx+20, shipy+10);

			/* specify the selecting region to be a box 
			surrounding the rocket ship */

			pushmatrix();
			ortho(shipx,shipx+.05,shipy,shipy+.05,
							shipz-0.5,shipz+.05);
			/* clear the name stack */
			initnames();

			select(buffer, 50);  /* enter selecting mode */

			/* put "1" on the name stack to be saved if PLANET
			draws into the selecting region */
			loadname(1);
			pushname(2);

			/* draw the planet */
			callobj(PLANET);

			/* exit selecting mode */
			cnt = endselect(buffer);
			popmatrix();

			/* check to see if PLANET was selected */
			if (buffer[1]==1) {
				printf("CRASH\n");
				curson();
				gexit();
			}
		}
	}
}

createplanet(x)
{
	makeobj(x);
	circfi(200, 200, 20);
	closeobj();
}
