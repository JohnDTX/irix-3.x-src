#include "gl.h"
#include "device.h"

#define CUBE_SIZE 200

main ()
{
	int foo;

	ginit();
	doublebuffer();
	gconfig();

	viewport(0, YMAXSCREEN, 0, YMAXSCREEN);
	ortho(-YMAXSCREEN.0, YMAXSCREEN.0, 
	    -YMAXSCREEN.0, YMAXSCREEN.0, 
	    -YMAXSCREEN.0, YMAXSCREEN.0);

	qdevice(KEYBD);
	makeobj(1);
	rectfi(-CUBE_SIZE,-CUBE_SIZE,CUBE_SIZE,CUBE_SIZE);
	closeobj();

	/* define a cube */
	makeobj(2);
	/* front face */
	pushmatrix();
	translate(0.0,0.0,CUBE_SIZE.0);
	color(RED);
	callobj(1);
	popmatrix();

	/* right face */
	pushmatrix();
	translate(CUBE_SIZE.0, 0.0, 0.0);
	rotate(900, 'y');
	color(GREEN);
	callobj(1);
	popmatrix();

	/* back face */
	pushmatrix();
	translate(0.0, 0.0, -CUBE_SIZE.0);
	rotate(1800, 'y');
	color(BLUE);
	callobj(1);
	popmatrix();

	/* left face */
	pushmatrix();
	translate(-CUBE_SIZE.0, 0.0, 0.0);
	rotate(-900, 'y');
	color(CYAN);
	callobj(1);
	popmatrix();

	/* top face */
	pushmatrix();
	translate(0.0, CUBE_SIZE.0, 0.0);
	rotate(-900, 'x');
	color(MAGENTA);
	callobj(1);
	popmatrix();

	/* bottom face */
	pushmatrix();
	translate(0.0, -CUBE_SIZE.0, 0.0);
	rotate(900, 'x');
	color(YELLOW);
	callobj(1);
	popmatrix();

	closeobj();

	/* turn on back facing polygon removal */
	backface(1);

	while ((foo=qtest()) != KEYBD) {
		if(foo) qreset();
		pushmatrix();
		rotate(2*getvaluator(MOUSEX), 'x');
		rotate(2*getvaluator(MOUSEY), 'y');
		color(BLACK);
		clear();
		callobj(2);
		popmatrix();
		swapbuffers();
	}

	backface(0);
	gexit();
}
