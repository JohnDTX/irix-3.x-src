/*
 * Etch-a-sketch simulator used to test the dial box for GL2
 *
 * Written by: Tom Davis
 * Updated by: Rick McLeod
 *             December 14, 1985
 */

#include "gl.h"
#include "device.h"

Icoord xval, yval, pair, pairled, colorled, counter, dialnumber, newpair, 
	newcolor, result, hits;
Colorindex line_dr_color;
Device	val, type, i;
char buf[100];

/* FUNCTIONS DEFINED */

choosepair() {    /*  Determine which pair of dials will be used for X and Y
		      And set value of pairled to turn on that button's led.
	              Switches 4, 10, 16, 22 chose dial pairs */	

	newpair =0;

	if (type == SW4) {  /* choose top pair */
		pair = 4;
		pairled = 0x00000010;
		newpair = 1;
	}
	else
	if (type == SW10) { /* choose pair next to top pair */
		pair = 3;
		pairled = 0x00000400;
		newpair = 1;
	}
	else
	if (type == SW16) { /* choose pair next to bottom pair  */
		pair = 2;
		pairled = 0x00010000;
		newpair = 1;
	}
	else
	if (type == SW22) { /* choose bottom pair  */
		pair = 1;
		pairled = 0x00400000;
		newpair = 1;
	}
	if (newpair == 1) { /* set dials if new pair */
		setdials();
	}
}

setdials()  {  /* set new dial pair to current X and Y screen positions   */

	if (pair == 4) {
		setvaluator(DIAL6,xval,100,924);
		setvaluator(DIAL7,yval,100,678);
	}
	else

	if (pair == 3) {
		setvaluator(DIAL4,xval,100,924);
		setvaluator(DIAL5,yval,100,678);
	}
	else

	if (pair == 2) {
		setvaluator(DIAL2,xval,100,924);
		setvaluator(DIAL3,yval,100,678);
	}
	else

	if (pair == 1) {
		setvaluator(DIAL0,xval,100,924);
		setvaluator(DIAL1,yval,100,678);
	}
}

choosecolor()   {  /* Determine the line drawing color
		      Use switches 0 - 3 & 28 - 30 to select color  */

	newcolor = 0;

	if (type == SW0) {  /* black */
		line_dr_color = 1;
		newcolor =1;
	}
	else
	if (type == SW1) {  /* red */
		line_dr_color = 2;
		newcolor =1;
	}
	else
	if (type == SW2) {  /* green */
		line_dr_color = 3;
		newcolor =1;
	}
	else
	if (type == SW3) {  /* blue */
		line_dr_color = 4;
		newcolor =1;
	}
	else
	if (type == SW28) { /* yellow */
		line_dr_color = 5;
		newcolor =1;
	}
	else
	if (type == SW29) { /* magenta */
		line_dr_color = 6;
		newcolor =1;
	}
	else
	if (type == SW30) { /* light blue */
		line_dr_color = 7;
		newcolor =1;
	}

	if (newcolor == 1)  { /* set color if new */
		setcolor();
	}
}

setcolor()  { /* turn on appropriate button's led */

	if (line_dr_color == 1)  {
		colorled = 0x00000001;
	}

	else
	if (line_dr_color == 2)  {
		colorled = 0x00000002;
	}

	else
	if (line_dr_color == 3)  {
		colorled = 0x00000004;
	}

	else
	if (line_dr_color == 4)  {
		colorled = 0x00000008;
	}

	else
	if (line_dr_color == 5)  {
		colorled = 0x10000000;
	}

	else
	if (line_dr_color == 6)  {
		colorled = 0x20000000;
	}

	else
	if (line_dr_color == 7)  {
		colorled = 0x40000000;
	}

}

checkxy ()  { /* check x and y values and make sure they stay within:
		  	X:  100 to 924
		  	Y:  100 to 678   */

	if (xval < 100) {
		xval = 100;
	}
	if (xval > 924) {
		xval = 924;
	}
	if (yval < 100) {
		yval = 100;
	}
	if (yval > 678) {
		yval = 678;
	}
}


/*  MAIN PROGRAM LOOP  */

main()
{

	/* initialize graphics display */
	ginit();
	singlebuffer();
	writemask(0xffff);
	gconfig();

	/* Set color map */
	mapcolor(0, 120, 120, 120);
	mapcolor(1, 0, 0, 0);
	mapcolor(2, 255, 0, 0);
	mapcolor(3, 0, 255, 0);
	mapcolor(4, 0, 0, 255);
	mapcolor(5, 255, 255, 0);
	mapcolor(6, 255, 0, 255);
	mapcolor(7, 0, 255, 255);

	linewidth(2);
	cursoff();
	color(0);
	clear();
	move2i(0, 0);
	gflush();
	curson();

 	/* capture MOUSEX & MOUSEY values when LEFTMOUSE is pressed */
	tie(LEFTMOUSE,MOUSEX,MOUSEY); 

	/* capture MOUSEX & MOUSEY values when MIDDLEMOUSE is pressed */
	tie(MIDDLEMOUSE,MOUSEX,MOUSEY);

	/* Put Mouse buttons & valuators, switches, and valuators in queue */	
	qdevice(RIGHTMOUSE);
	qdevice(MIDDLEMOUSE);
	qdevice(LEFTMOUSE);
	qdevice(MOUSEX);
	qdevice(MOUSEY);
	qdevice(KEYBD);

	for(i=SW0; i<=SW31; i++) {
		qdevice(i);	
	}	
		
	for(i=DIAL0; i<=DIAL7; i++) {
		qdevice(i);
	}		

	/* Dials must move more than two before value is queued */
	for(i=DIAL0; i<=DIAL7; i++) {
		noise(i, 2);
	}

	/* Set initial, max, and min values for dials */
	setvaluator(DIAL0,512,100,924);
	setvaluator(DIAL1,512,100,678);
	setvaluator(DIAL2,512,100,924);
	setvaluator(DIAL3,512,100,678);
	setvaluator(DIAL4,512,100,924);
	setvaluator(DIAL5,512,100,678);
	setvaluator(DIAL6,512,100,924);
	setvaluator(DIAL7,512,100,678);

	xval = getvaluator(DIAL0);
	yval = getvaluator(DIAL1);
	move2i(xval,yval);

	/* Cycle through switch led's until switch 31 is pressed. */
	while (type != SW31) {
			dialnumber = 1;
			setdblights(dialnumber);
			for(counter=0; counter<=31; ++counter) {  
				dialnumber = dialnumber << 1;
				setdblights(dialnumber);
				dbtext("PRESS 31");
			}
			if (qtest()) {
				if (qread(&val) == SW31)
					break;
				}
	}

	/* Select pair 4 and color 1 to start */
	colorled = 0x00000001;
	pairled = 0x00000010;
	line_dr_color = 1;
	pair = 4;
	color(1);

	/* set button leds to show pair and color selected */
	setdblights(pairled | colorled);

	while (1) {
		type = qread(&val);
		dialnumber = 1;
		color(line_dr_color);

		/* if ESC is pressed exit program and clear graphics screen */
		if (type == KEYBD && val == 27) {
			greset();
			doublebuffer();
			gconfig();
			color(BLACK);
			clear();
			swapbuffers();
			clear();
			gflush();
			textinit();
			tpon();
			gexit();
			exit(0);
		}
		else

		choosepair(); /* choose which pair of dials to use */

		/* Take x and y values from one of the pairs
		   of valuators depending on value of 'pair' */
		
		if (type == DIAL0 && pair == 1)
			xval = val;
		else
		if (type == DIAL1 && pair == 1)
			yval = val;
		else
		if (type == DIAL2 && pair == 2)
			xval = val;
		else
		if (type == DIAL3 && pair == 2)
			yval = val;
		else
		if (type == DIAL4 && pair == 3)
			xval = val;
		else
		if (type == DIAL5 && pair == 3)
			yval = val;
		else
		if (type == DIAL6 && pair == 4)
			xval = val;
		else
		if (type == DIAL7 && pair == 4)
			yval = val;
		else

		/* the left mouse button will cause a line to be drawn from
		   the current screen position to the position of the mouse */

		if (type == LEFTMOUSE) {
			xval = getvaluator(MOUSEX);
			yval = getvaluator(MOUSEY);
			checkxy(); /* check values of x and y */
			setdials(); /* update value of dials to new position */
		}
		else

		/* the middle mouse button will cause the current screen 
		   position to change to where the mouse is, but not draw
		   a line between the two points */

		if (type == MIDDLEMOUSE) {
			xval = getvaluator(MOUSEX);
			yval = getvaluator(MOUSEY);
			checkxy(); /* check values of x and y */
			move2i(xval,yval);
			setdials(); /* update value of dials to new position */  
		}
		else

		/* the right mouse button will cause the line drawing color
		   to be incremented by 1.  'hits' prevents button bounce 
		   from incrementing color by more than 1 for each pressing */

		if (type == RIGHTMOUSE) {
			hits = hits + 1;
			if (hits == 1)   {
				line_dr_color = line_dr_color + 1;
				if (line_dr_color == 8)  {
					line_dr_color = 1;
				}
				setcolor(); /* update color button led */
			}
			if (hits >= 2) {
				hits = 0;
			}
		}
		else

		choosecolor(); /* chose line drawing color */

		/* draw line using xval, yval & color  */
		cursoff();
		color(line_dr_color);
		draw2i(xval, yval);
		curson();

		/* set text display and button leds  */
		dbtext("LED TEST");
		setdblights(pairled | colorled);

		/* The following switches perform these functions:

		   switches 5 - 9 ----- draws circles of various radii
		   switches 11 - 15 --- draws solid circles of various radii
		   switches 17 - 21 --- draws rectangles of various sizes
		   switches 23 - 27 --- draws solid rect's of various sizes 
		   
		   Text display is updated to either "CIRCLE" or "SQUARE"
		   All objects are drawn based on current screen position */

		if (type == SW5) {
			cursoff();
			color(2);
			circi(xval,yval, 100);
			curson();
			dbtext("CIRCLE");
		}
		else
		if (type == SW6) {
			cursoff();
			color(3);
			circi(xval,yval, 75);
			curson();
			dbtext("CIRCLE");
		}
		else
		if (type == SW7) {
			cursoff();
			color(4);
			circi(xval,yval, 50);
			curson();
			dbtext("CIRCLE");
		}
		else
		if (type == SW8) {
			cursoff();
			color(5);
			circi(xval,yval, 25);
			curson();
			dbtext("CIRCLE");
		}
		else
		if (type == SW9) {
			cursoff();
			color(6);
			circi(xval,yval, 10);
			curson();
			dbtext("CIRCLE");
		}
		else
		if (type == SW11) {
			cursoff();
			color(4);
			circfi(xval, yval, 100);
			curson();
			dbtext("CIRCLE");
		}
		else
		if (type == SW12) {
			cursoff();
			color(5);
			circfi(xval, yval, 75);
			curson();
			dbtext("CIRCLE");
		}
		else
		if (type == SW13) {
			cursoff();
			color(6);
			circfi(xval, yval, 50);
			curson();
			dbtext("CIRCLE");
		}
		else
		if (type == SW14) {
			cursoff();
			color(7);
			circfi(xval, yval, 25);
			curson();
			dbtext("CIRCLE");
		}
		else
		if (type == SW15) {
			cursoff();
			color(3);
			circfi(xval, yval, 10);
			curson();
			dbtext("CIRCLE");
		}
		else
		if (type == SW17) {
			cursoff();
			color(3);
			recti(xval+100,yval-100,xval-100,yval+100);
			curson();
			dbtext("SQUARE");
		}
		else
		if (type == SW18) {
			cursoff();
			color(4);
			recti(xval+75,yval-75,xval-75,yval+75);
			curson();
			dbtext("SQUARE");
		}
		else
		if (type == SW19) {
			cursoff();
			color(5);
			recti(xval+50,yval-50,xval-50,yval+50);
			curson();
			dbtext("SQUARE");
		}
		else
		if (type == SW20) {
			cursoff();
			color(6);
			recti(xval+25,yval-25,xval-25,yval+25);
			curson();
			dbtext("SQUARE");
		}
		else
		if (type == SW21) {
			cursoff();
			color(7);
			recti(xval+10,yval-10,xval-10,yval+10);
			curson();
			dbtext("SQUARE");
		}
		else
		if (type == SW23) {
			cursoff();
			color(5);
			rectfi(xval+100,yval-100,xval-100,yval+100);
			curson();
			dbtext("SQUARE");
		}
		else
		if (type == SW24) {
			cursoff();
			color(6);
			rectfi(xval+75,yval-75,xval-75,yval+75);
			curson();
			dbtext("SQUARE");
		}
		else
		if (type == SW25) {
			cursoff();
			color(7);
			rectfi(xval+50,yval-50,xval-50,yval+50);
			curson();
			dbtext("SQUARE");
		}
		else
		if (type == SW26) {
			cursoff();
			color(3);
			rectfi(xval+25,yval-25,xval-25,yval+25);
			curson();
			dbtext("SQUARE");
		}
		else
		if (type == SW27) {
			cursoff();
			color(4);
			rectfi(xval+10,yval-10,xval-10,yval+10);
			curson();
			dbtext("SQUARE");
		}
		else

		/* Switch 31 clears the screen   */
		if (type == SW31) {
			dbtext(" CLEAR ");
			cursoff();
			color(0);
			clear();
			color(1);
			curson();
			move2i(xval,yval);
		}
		/* Print X, Y, and Color information in bottom left corner
		   of screen.  */
		color(4);
		rectfi(0, 0, 200, 30);
		color(2);
		sprintf(buf, "X=%d Y=%d Color=%d", xval, yval, line_dr_color);  
		cmov2i(0, 0);
		charstr(buf);
		move2i(xval, yval);
		gflush();
	}
}



