/*
 *	showmap - 
 *		Display the color map.
 *
 *				Paul Haeberli - 1984
 *
 */
#include "gl.h"
#include "device.h"
#include "port.h"

main()
{
    short val;
    int menu;

    keepaspect(1,1);
    winopen("showmap");

    qdevice(MENUBUTTON);
    menu = defpup("showmap %t|makemap|cedit|interp|palette|savedesktop");
    showmap();
    while (1) {
	switch(qread(&val)) {
	    case REDRAW: 
		reshapeviewport(); 
		showmap();
		break;
	    case MENUBUTTON: 
		if (val) {
		    switch (dopup(menu)) {
			case 1: 
			    dosystem("makemap");
			    break;
			case 2: 
			    dosystem("cedit");
			    break;
			case 3: 
			    dosystem("interp");
			    break;
			case 4: 
			    dosystem("palette");
			    break;
			case 5: 
			    dosystem("savedesktop");
			    break;
		    }
		}
		break;

	}
    }
}

showmap()
{
    int i, j, planes;

    color(GREY(15));
    clear();
    planes = getplanes();
    if (planes <= 8) {
	ortho2(-0.25,8.25,-0.25,8.25);
	for (j=0; j<8; j++)
	    for (i=0; i<8; i++) {
		color((j<<3)+i);
		rectf(i+0.1,j+0.1,i+0.9,j+0.9);
	    }
    } else {
	ortho2(-0.25,32.25,-0.25,32.25);
	for (j=0; j<32; j++)
	    for (i=0; i<32; i++) {
		color((j<<5)+i);
		rectfi(i,j,i+1,j+1); 

	    }	
	color(GREY(8));
	for (j=0; j<=32; j++) {
	    move2i(0,j);
	    draw2i(32,j);
	    move2i(j,0);
	    draw2i(j,32);
	}
    }
}
