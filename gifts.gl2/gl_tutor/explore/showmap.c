/*
 *	showmap - 
 *		Display the color map.
 *
 *				Paul Haeberli - 1984
 *
 */
#include "gl.h"
#include "device.h"

main(argc, argv)
int argc;
char **argv;
{
    short val;
    int menu;

    keepaspect(1,1);
    winopen("showmap");

    showmap();
    while (1) {
	switch(qread(&val)) {
	    case REDRAW: 
		reshapeviewport(); 
		showmap();
		break;
	}
    }
}

showmap()
{
    int i, j, planes;

    color(BLACK);
    clear();
    planes = getplanes();
    switch (planes) {
    case 2:
	ortho2(-0.1,2.1,-0.1,2.1);
	for (j=0; j<2; j++)
	    for (i=0; i<2; i++) {
		color((j<<1)+i);
		rectf(i+0.1,j+0.1,i+0.9,j+0.9);
	    }
	break;
    case 4:
	ortho2(-0.25,4.25,-0.25,4.25);
	for (j=0; j<4; j++)
	    for (i=0; i<4; i++) {
		color((j<<2)+i);
		rectf(i+0.1,j+0.1,i+0.9,j+0.9);
	    }
	break;
    case 6:
	ortho2(-0.25,8.25,-0.25,8.25);
	for (j=0; j<8; j++)
	    for (i=0; i<8; i++) {
		color((j<<3)+i);
		rectf(i+0.1,j+0.1,i+0.9,j+0.9);
	    }
	break;
    case 8:
	ortho2(-0.25,16.25,-0.25,16.25);
	for (j=0; j<16; j++)
	    for (i=0; i<16; i++) {
		color((j<<4)+i);
		rectfi(i,j,i+1,j+1);
	    }
	color(BLACK);
	for (j=0; j<=16; j++) {
	    move2i(0,j);
	    draw2i(16,j);
	    move2i(j,0);
	    draw2i(j,16);
	}
	break;
    case 10:
	ortho2(-0.25,32.25,-0.25,32.25);
	for (j=0; j<32; j++)
	    for (i=0; i<32; i++) {
		color((j<<5)+i);
		rectfi(i,j,i+1,j+1); 

	    }	
	color(BLACK);
	for (j=0; j<=32; j++) {
	    move2i(0,j);
	    draw2i(32,j);
	    move2i(j,0);
	    draw2i(j,32);
	}
	break;
    }	/*  end switch	*/
}
