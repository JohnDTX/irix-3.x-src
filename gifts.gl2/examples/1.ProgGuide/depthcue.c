#include "gl.h"
#include "device.h"
#include "math.h"

float hrand();

main ()
{
	int val;
	int i;

	ginit();
	doublebuffer();
	gconfig();

	ortho(-350.0, 350.0, -350.0, 350.0, -350.0, 350.0);
	viewport(0, YMAXSCREEN, 0, YMAXSCREEN);
	qdevice(KEYBD);

	makeobj(1);

/* a bunch of random points */
	for (i = 0; i < 100; i++)
		pnt(hrand(-200.0,200.0), 
				hrand(-200.0,200.0), hrand(-200.0,200.0));

/* and a cube */
	movei(-200, -200, -200);
	drawi(200, -200, -200);
	drawi(200, 200, -200);
	drawi(-200, 200, -200);
	drawi(-200, -200, -200);
	drawi(-200, -200, 200);
	drawi(-200, 200, 200);
	drawi(-200, 200, -200);
	movei(-200, 200, 200);
	drawi(200, 200, 200);
	drawi(200, -200, 200);
	drawi(-200, -200, 200);
	movei(200, 200, 200);
	drawi(200, 200, -200);
	movei(200, -200, -200);
	drawi(200, -200, 200);
	closeobj();

/* load the color map with a cyan ramp */
	for (i = 0; i < 128; i++)
		mapcolor(128+i, 0, 2*i, 2*i);
/* set the range of z values that will be stored
	in the bitplanes */
	setdepth(0, 0x7fff);
/* set up the mapping of z values to color map indices:
	z value 0 is mapped to index 128 and z value 0x7fff is
	mapped to index 255 */
	shaderange(128,255,0,0x7fff);

/* turn on depthcue mode:  the color index of each pixel in points
	and lines is determined from the z value of the pixel */
	depthcue(1);
/* until a key is pressed, rotate the cube according to the
	movement of the mouse */
	while ((val=qtest()) != KEYBD) {
		pushmatrix();
		rotate(3*getvaluator(MOUSEY), 'x');
		rotate(3*getvaluator(MOUSEX), 'y');
		color(BLACK);
		clear();
		callobj(1);
		popmatrix();
		swapbuffers();
	}
	gexit();
}

/* this routine returns random numbers in the specified range */
float hrand(low,high)
float low,high;
{
	float val;

	val = ((float)( (short)rand(0) & 0xffff)) / ((float)0xffff);
	return( (2.0 * val * (high-low)) + low);
}
