/*  projection.c						*/
/*  May, 1986 -- Thant Tessman					*/
/*  Learning Environment -- to demonstrate projection and 	*/
/*  viewing transformations.  Particularly:  perspective, 	*/
/*  window, ortho; polarview and lookat.			*/

#include "tutor.h"
#include <stdio.h>
#include <gl.h>
#include <math.h>
#include <device.h>

#define X	0
#define Y	1
#define Z	2

#define ORTHO	1
#define PERSP	2
#define WINDOW	3
#define POLAR	1
#define LOOKAT	2

#define BARX	0
#define BARY	0

#define DOTTED 1

int	    helpw, 
	    statw, 
	    consw, 
	    frontw, 
	    viewzw, 
	    viewxw, 
	    backw;

short oldcolors[16][3];

int killmenu, projmenu, viewmenu, menu, p1(), p2(), p3(), v1(), v2();

Linestyle dotted = 0xaaaa;

/* This stuff is for getting the viewing pyramid object where lookat would. */

float sin_theta, sin_phi, cos_theta, cos_phi;
Matrix m;

Object letter, simple_letter, viewing_pyramid;

short dev, val;
int active;

short mx, my;
long dx;
float wrap;

int holding=FALSE,	/* This is true if the left mouse is down to adjust 
			 * a parameter with the mouse.  (Holding the knob.)
			 */

    projection = PERSP,	/* Projection matrix ORTHO WINDOW PERSP */
    viewing    = POLAR,	/* Viewing matrix POLARVIEW LOOKAT */

    line=0;		/* The first line is the projection transformation
			 * e.g. ortho, window, persp.
			 * The second is the viewing transformation.
			 */

    parameter=0;	/* This is the selected parameter.
			 *
			 * 0 = none selected.
			 *
			 * 1 = dist	\
			 * 2 = azim	 \   polarview
			 * 3 = inc	 /
			 * 4 = twist	/
			 *
			 * 1 = fovy	\
			 * 2 = aspect    \   perspective
			 * 3 = near	 /
			 * 4 = far	/
			 *
			 * et cetera.
			 */

int which_function();	/* This returns a number representing the function
			   selected when the user pressed the left mouse
			   button.
					0 = nothing
					1 = select parameter
					2 = reset parameter values
					3 = adjust parameter values
			*/


Angle fovy;
float aspect;
Coord near, far;
Coord left, right, bottom, top;
Coord vx, vy, vz, px, py, pz;
Coord dist;
Angle azim, inc, twist;
float low_pos, high_pos;

main ()
{
    Coord i,j;
    int really;

    check_for_mex();

    init_windows();
    killmenu = defpup("Exit Confirmation %t|Yes|No");

    setupcolors();
    reset_values();
    makeobjects();
    make_popup();

    qdevice(ESCKEY);
    qdevice(LEFTMOUSE);
    tie(LEFTMOUSE, MOUSEX, MOUSEY);
    qdevice(RIGHTMOUSE);

    while(TRUE) {

	while(qtest()) {
	    dev=qread(&val);
	    switch(dev) {
		case ESCKEY :
		    restore_colors();
		    gexit(); exit(0);
		    break;
		case INPUTCHANGE:
		    active = val;
		    if (val) draw_frame();
		    break;
		case LEFTMOUSE :
		    qread(&mx); qread(&my);
		    if (val) switch (which_function()) {
			case 1:	/* select */
			    select_parameter();
			    break;
			case 2:	/* adjust parameter */
			    holding = val;
			    break;
			default:
			    break;
		    } else {
			if (holding) {
			    holding = FALSE;
			}
		    }
		    break;
		case RIGHTMOUSE :
		    if(val) 
			switch (dopup(menu)) {
			case 1:
			    if(dopup(projmenu)) {
			       draw_state();
			       swapbuffers();
			       draw_state();
			       qreset();
			    }
			    break;
			case 2:
			    if(dopup(viewmenu)) {
			       draw_state();
			       swapbuffers();
			       draw_state();
			       qreset();
			    }
			    break;
			case 4:
			    really = dopup(killmenu);
			    if (really == 1) {
				restore_colors();
				gexit();
				exit(0);
			    }
			    break;
			default:
			    break;
			}
		    break;
		default:
		    break;
	    }
	}

	change_parameter();

	draw_frame();

    }
}

draw_text() {

    color(NORMCOLOR);

    switch (projection) {
    case PERSP:
	cmov2i(0, 50);
#ifdef FORTRAN
	charstr("call perspe (");
#else
	charstr("perspective (");
#endif
	print_int(1, 1, "  fovy", fovy, FALSE);
	print_float(1, 2, "aspect", aspect, FALSE);
	print_float(1, 3, "  near", near, FALSE);
	print_float(1, 4, "   far", far, TRUE);
        break;

    case WINDOW:
	cmov2i(0, 50);
#ifdef FORTRAN
	charstr("call window (");
#else
	charstr("window (");
#endif
	print_float(1, 1, "  left", left, FALSE);
	print_float(1, 2, " right", right, FALSE);
	print_float(1, 3, "bottom", bottom, FALSE);
	print_float(1, 4, "   top", top, FALSE);
	print_float(1, 5, "  near", near, FALSE);
	print_float(1, 6, "   far", far, TRUE);
	break;

    case ORTHO:
	cmov2i(0, 50);
#ifdef FORTRAN
	charstr("call ortho (");
#else
	charstr("ortho (");
#endif
	print_float(1, 1, "  left", left, FALSE);
	print_float(1, 2, " right", right, FALSE);
	print_float(1, 3, "bottom", bottom, FALSE);
	print_float(1, 4, "   top", top, FALSE);
	print_float(1, 5, "  near", near, FALSE);
	print_float(1, 6, "   far", far, TRUE);
        break;

    default:
        break;
    }

    color(NORMCOLOR);

    switch (viewing) {

    case POLAR:
        cmov2i(0, 0);
#ifdef FORTRAN
	charstr("call polarv (");
#else
	charstr("polarview (");
#endif
	print_float(2, 1, "  dist", dist, FALSE);
	print_int(2, 2, "  azim", azim, FALSE);
	print_int(2, 3, "   inc", inc, FALSE);
	print_int(2, 4, " twist", twist, TRUE);
        break;

    case LOOKAT:
        cmov2i(0, 0);
#ifdef FORTRAN
	charstr("call lookat (");
#else
	charstr("lookat (");
#endif
	print_float(2, 1, "    vx", vx, FALSE);
	print_float(2, 2, "    vy", vy, FALSE);
	print_float(2, 3, "    vz", vz, FALSE);
	print_float(2, 4, "    px", px, FALSE);
	print_float(2, 5, "    py", py, FALSE);
	print_float(2, 6, "    pz", pz, FALSE);
	print_int(2, 7, " twist", twist, TRUE);
        break;

    default:
        break;
    }
}

draw_top_view() {

    perspective(450, 1.0, 0.01, 100.0);
    polarview(15.0, 0, 0, 0);

    color(AXESCOLOR);
    cmov(4.0, 0.0, 0.0);
    charstr("x");
    cmov(0.0, 4.0, 0.0);
    charstr("y");

    callobj(simple_letter);

    switch (viewing) {

    case POLAR:
	rotate(azim, 'z');
	rotate(inc, 'x');
	rotate(twist, 'z');
	translate(0.0, 0.0, dist);
	callobj(viewing_pyramid);
	break;

    case LOOKAT:
	color(WHITE);
	move(px, py, pz); draw(vx, vy, vz);
	translate(vx, vy, vz);
	gl_IdentifyMatrix(m);
	m[0][0] = cos_theta;    m[0][2] = -sin_theta;
	m[2][0] = sin_theta;    m[2][2] = cos_theta;
	multmatrix(m);
	gl_IdentifyMatrix(m);
	m[1][1] = cos_phi;	    m[1][2] = sin_phi;
	m[2][1] = -sin_phi;	    m[2][2] = cos_phi;
	multmatrix(m);
	rotate(twist, 'z');
	callobj(viewing_pyramid);
	break;

    default:
	break;
    }
}

draw_side_view() {

    perspective(450, 1.0, 0.01, 100.0);
    polarview(15.0, 900, 900, 0);
    rotate (900, 'x');

    color(AXESCOLOR);

    cmov(0.0, 0.0, 4.0);
    charstr("z");
    cmov(0.0, 4.0, 0.0);
    charstr("y");
    callobj(simple_letter);

    switch (viewing) {

    case POLAR:
	rotate(azim, 'z');
	rotate(inc, 'x');
	rotate(twist, 'z');
	translate(0.0, 0.0, dist);
	callobj(viewing_pyramid);
	break;

    case LOOKAT:
	color(WHITE);
	move(px, py, pz); draw(vx, vy, vz);
	translate(vx, vy, vz);
	gl_IdentifyMatrix(m);
	m[0][0] = cos_theta;    m[0][2] = -sin_theta;
	m[2][0] = sin_theta;    m[2][2] = cos_theta;
	multmatrix(m);
	gl_IdentifyMatrix(m);
	m[1][1] = cos_phi;	    m[1][2] = sin_phi;
	m[2][1] = -sin_phi;	    m[2][2] = cos_phi;
	multmatrix(m);
	rotate(twist, 'z');
	callobj(viewing_pyramid);
	break;

    default:
	break;
    }
}

draw_viewport() {

    if (check_parameters()) {
	switch (projection) {
	case PERSP:
	    perspective(fovy, aspect, near, far);
	    break;
	case ORTHO:
	    ortho(left, right, bottom, top, near, far);
	    break;
	case WINDOW:
	    window(left, right, bottom, top, near, far);
	    break;
	default:
	    break;
	}
	switch (viewing) {
	case POLAR:
	    polarview(dist, azim, inc, twist);
	    break;
	case LOOKAT:
	    lookat(vx, vy, vz, px, py, pz, twist);
	    break;
	default:
	    break;
	}
	callobj(letter);
    }

}

int check_parameters() {

    if (!active) {
	load_help("Press the RIGHT MOUSE BUTTON and select ATTACH.",
	    "", "", "", "", "");
	return(TRUE);
    }

    if (projection==0 || viewing==0) {
	load_help("Press and hold the RIGHT MOUSE BUTTON to ", 
		  "  bring up pop up menu.", "", "", "", "");
	if(projection==0) {
	    load_help("Select a projection transform", 
		      "  (ortho, perspective, window)", "", "", "", "");
	}
	if(viewing==0) {
	    load_help("Select a viewing transform", 
		      "  (polarview, lookat)", "", "", "", "");
	}
	return(FALSE);
    }
    if (projection==PERSP) {
	if ((fovy<2) || (fovy>1799)) {
	    load_help("The field of view must be between 2 and 1799.", "", 
		      "", "", "", "");
	    return(FALSE);
	} else if (near == far) {
	    load_help("The near and far clipping planes can't be the same.",
		      "", "", "", "", "");
	    return(FALSE);
	} else if (aspect == 0.0) {
	    load_help("The aspect ratio can't be zero.", "", "", "", "", "");
	    return(FALSE);
	} else if (parameter == 0) {
	    load_help("Use the LEFT MOUSE button to select a parameter", 
		      "  from the graphics commands below OR use the", 
		      "  RIGHT MOUSE button to bring up the popup menu.", 
		      "", "", "");
	    return(TRUE);
	} else {	/*  a value is highlighted	*/
	    load_help("Use the LEFT MOUSE button to adjust the highlighted", 
		      "  parameter with the controller bar OR to select", 
		      "  another parameter from the graphics commands ", 
		      "  below OR use the RIGHT MOUSE button to bring", 
		      "  up the popup menu.", "");
	    return(TRUE);
	}
    } else if (projection==ORTHO || projection==WINDOW) {
        if (left==right) {
	    load_help("The left edge can't equal the right edge.", "", "", 
			"", "", "");
	    return(FALSE);
	} else if (top==bottom) {
	    load_help("The top edge can't equal the bottom edge.", "", "", 
			"", "", "");
	    return(FALSE);
	} else if (parameter == 0) {
	    load_help("Use the LEFT MOUSE button to select a parameter", 
		      "  from the graphics commands below OR use the ", 
		      "  RIGHT MOUSE button to bring up the popup menu.",
			"", "", "");
	    return(TRUE);
	} else {	/*  a value is highlighted	*/
	    load_help("Use the LEFT MOUSE button to adjust the highlighted", 
		      "  parameter with the controller bar OR to select", 
		      "  another parameter from the graphics commands below", 
		      "  OR use the RIGHT MOUSE button to bring up the", 
		      "  popup menu.", "");
	    return(TRUE);
	}
    }
}

select_parameter() {

    long mx,my;
    long ox, oy;

    while (getbutton(LEFTMOUSE)) {
	winset(statw);
	getorigin(&ox, &oy);
        mx = getvaluator(MOUSEX); my = getvaluator(MOUSEY);
	mx -= ox+10; my -= oy+20; 
	parameter=0;
	if ( (my<35) && (my>-20) ) line = 2;
	else if ( (my<89) && (my>35) ) line = 1;
	if ( (mx>150) && (mx<640) ) parameter = (mx-140) / 70 + 1;
	draw_text();
	swapbuffers();
    }
    draw_text();
    swapbuffers();
}

change_parameter() {

    long mx;
    long iwx;
    long sx, sy;
    long ox, oy;

    wrap = 0.0;

    if (holding) {
	winset(consw);
	getsize(&sx, &sy);
	getorigin(&ox, &oy);
    	mx = getvaluator(MOUSEX);

	iwx = mx - (ox + sx/2);

	if (iwx < -150) {
	    iwx += 300;
	    setvaluator(MOUSEX, iwx+ox+sx/2, 0, 1023);
	    wrap = -1.0;
	}

	else if (iwx > 149) {
	    iwx -= 300;
	    setvaluator(MOUSEX, iwx+ox+sx/2, 0, 1023);
	    wrap = 1.0;
	}

	dx = (iwx+150);

	if (line==1) {
	    switch (projection) {
	    case PERSP:
		modify_persp();
		break;
	    case ORTHO:
		modify_ortho();
		break;
	    case WINDOW:
		modify_window();
		break;
	    default:
		break;
	    }
	    make_viewing_pyramid();
	}
	else if (line==2) {
	    switch (viewing) {
	    case POLAR:
		modify_polar();
		break;
	    case LOOKAT:
		modify_lookat();
		break;
	    default:
		break;
	    }
	    redo_theta_phi();
	}
    }
}

redo_theta_phi() {

    /* the sin components are negative because actually i want -theta and *
     * -phi and sin(-theta) = -sin(theta) and cos(-theta) = cos(theta).   */

    sin_theta = -(px-vx) / sqrt( (px-vx) * (px-vx) + (pz-vz) * (pz-vz) );

    cos_theta = (vz-pz) / sqrt( (px-vx) * (px-vx) + (pz-vz) * (pz-vz) );

    sin_phi = -(vy-py) / 
	sqrt( (px-vx) * (px-vx) + (py-vy) * (py-vy) + (pz-vz) * (pz-vz) );

    cos_phi = sqrt( (px-vx) * (px-vx) + (pz-vz) * (pz-vz) ) / 
	sqrt( (px-vx) * (px-vx) + (py-vy) * (py-vy) + (pz-vz) * (pz-vz) );

}

makeobjects() {

    Coord i, j;

    makeobj(letter=genobj());
	color(RED);
	draw_fflat(.5);
	draw_fflat(.25);
	draw_fflat(0.0);
	draw_fflat(-.25);
	draw_fflat(-.5);
	color(OBJECTCOLOR);
	draw_fedges(.75);
	color(AXESCOLOR);
	move (0.0, 0.0, 0.0); draw(0.0, 0.0, 1.1);
	cmov(0.0, 0.0, 1.15); charstr("Z");
	move (0.0, 0.0, 0.0); draw(0.0, 1.1, 0.0);
	cmov(0.0, 1.15, 0.0); charstr("Y");
	move (0.0, 0.0, 0.0); draw(1.1, 0.0, 0.0);
	cmov(1.15, 0.0, 0.0); charstr("X");
    closeobj();

    makeobj(simple_letter=genobj());

	color(OBJECTCOLOR);
	draw_fedges(.75);

	color(AXESCOLOR);
	move (0.0, 0.0, 0.0); draw(0.0, 0.0, 1.5);
	move (0.0, 0.0, 0.0); draw(0.0, 1.5, 0.0);
	move (0.0, 0.0, 0.0); draw(1.5, 0.0, 0.0);

    closeobj();

    make_viewing_pyramid();
    if (viewing==LOOKAT) redo_theta_phi();

}

draw_fedges(deep) 
float deep;
{

	horizpoly(-.6, -1., -.2, deep);	/*  A to J lower corner		*/
	vertpoly(-.6, -1., 1., deep);	/*  A to B left side		*/
	horizpoly(-.6, 1., .6, deep);	/*  B to C top edge		*/
	vertpoly(.6, 1., .6, deep);	/*  C to D right edge		*/
	horizpoly(.6, .6, -.2, deep);	/*  D to E lower part of top	*/
	vertpoly(-.2, .6, .2, deep);	/*  E to F upright above mid 	*/
	horizpoly(-.2, .2, .2, deep);	/*  F to G upper part of mid 	*/
	vertpoly(.2, .2, -.2, deep);	/*  G to H upright right of mid */
	horizpoly(.2, -.2, -.2, deep);	/*  H to I lower part of mid 	*/
	vertpoly(-.2, -.2, -1., deep);	/*  I to J upright below mid 	*/
}


horizpoly(x, y, nx, deep)
float x, y, nx, deep;
{
	move(x, y, deep);
	draw(x, y, -deep);
	draw(nx, y, -deep);
	draw(nx, y, deep);
	draw(x, y, deep);
}

vertpoly(x, y, ny, deep)
float x, y, ny, deep;
{
	move(x, y, deep);
	draw(x, y, -deep);
	draw(x, ny, -deep);
	draw(x, ny, deep);
	draw(x, y, deep);
}

draw_fflat(deep) 
float deep;
{
/*  draw F hollow outline at z=deep	*/
	move(-.6, -1., deep);	
	draw(-.6, 1., deep);
	draw(.6, 1., deep);
	draw(.6, .6, deep);
	draw(-.2, .6, deep);
	draw(-.2, .2, deep);
	draw(.2, .2, deep);
	draw(.2, -.2, deep);
	draw(-.2, -.2, deep);
	draw(-.2, -1., deep);
	draw(-.6, -1., deep);
}

make_viewing_pyramid() {

    static
    float ful[3],	/* far clipping plane upper left */
	  fur[3],	/* far                      right */
	  fll[3],	/*                    lower left */
	  flr[3],	/* you probably get it by now */
	  nul[3], nur[3], nll[3], nlr[3];   /* near stuff */

    float glsin, glcos;	/* For quick math with the graphics library look-up
			   table. */

    switch (projection) {

    case PERSP:
	if ( fovy % 1800 == 0 || (fovy-1) % 1800 == 0) return;

	gl_sincos(fovy/2, &glsin, &glcos);

	/* The `z' coordinate of the far clipping plane is `far.' */
	ful[2] = -far; fur[2] = -far; fll[2] = -far; flr[2] = -far;

	/* Ditto for near. */
	nul[2] = -near; nur[2] = -near; nll[2] = -near; nlr[2] = -near;

	/* Stuff for `y' coordinate. */

	ful[1] = far * glsin / glcos; fur[1] = ful[1];
	fll[1] = -ful[1]; flr[1] = fll[1];

	nul[1] = near * glsin / glcos; nur[1] = nul[1];
	nll[1] = -nul[1]; nlr[1] = nll[1];

	/* Stuff for `x' coordinate. */

	fur[0] = aspect * fur[1]; ful[0] = -fur[0];
	fll[0] = ful[0]; flr[0] = fur[0];

	nur[0] = aspect * nur[1]; nul[0] = -nur[0];
	nll[0] = nul[0]; nlr[0] = nur[0];

	if (isobj(viewing_pyramid)) delobj(viewing_pyramid);

	makeobj(viewing_pyramid);

	    setlinestyle(DOTTED);

	    color(YELLOW);
	    move(0.0, 0.0, 0.0); draw(nll[0], nll[1], nll[2]);
	    move(0.0, 0.0, 0.0); draw(nlr[0], nlr[1], nlr[2]);
	    move(0.0, 0.0, 0.0); draw(nul[0], nul[1], nul[2]);
	    move(0.0, 0.0, 0.0); draw(nur[0], nur[1], nur[2]);

	    color(RED);
	    move(nll[0], nll[1], nll[2]); draw(fll[0], fll[1], fll[2]);
	    move(nlr[0], nlr[1], nlr[2]); draw(flr[0], flr[1], flr[2]);
	    move(nul[0], nul[1], nul[2]); draw(ful[0], ful[1], ful[2]);
	    move(nur[0], nur[1], nur[2]); draw(fur[0], fur[1], fur[2]);

	    setlinestyle(0);

	    color(YELLOW);
	    move(nur[0], nur[1], nur[2]); draw(nlr[0], nlr[1], nlr[2]);
	    draw(nll[0], nll[1], nll[2]); draw(nul[0], nul[1], nul[2]);
	    draw(nur[0], nur[1], nur[2]);

	    color(RED);
	    move(fur[0], fur[1], fur[2]); draw(flr[0], flr[1], flr[2]);
	    draw(fll[0], fll[1], fll[2]); draw(ful[0], ful[1], ful[2]);
	    draw(fur[0], fur[1], fur[2]);

	closeobj();
	break;

    case WINDOW:
	if (near==0.0) return;

	ful[2] = -far; fur[2] = -far; fll[2] = -far; flr[2] = -far;
	nul[2] = -near; nur[2] = -near; nll[2] = -near; nlr[2] = -near;

	nul[1] = top; nur[1] = top;
	nll[1] = bottom; nlr[1] = bottom;

	ful[1] = top/near * far; fur[1] = ful[1];
	fll[1] = bottom/near * far; flr[1] = fll[1];

	nur[0] = right; nul[0] = left;
	nlr[0] = right; nll[0] = left;

	fur[0] = right/near * far; ful[0] = left/near * far;
	fll[0] = ful[0]; flr[0] = fur[0];

	if (isobj(viewing_pyramid)) delobj(viewing_pyramid);

	makeobj(viewing_pyramid);

	    setlinestyle(DOTTED);

	    color(YELLOW);
	    move(0.0, 0.0, 0.0); draw(nll[0], nll[1], nll[2]);
	    move(0.0, 0.0, 0.0); draw(nlr[0], nlr[1], nlr[2]);
	    move(0.0, 0.0, 0.0); draw(nul[0], nul[1], nul[2]);
	    move(0.0, 0.0, 0.0); draw(nur[0], nur[1], nur[2]);

	    color(RED);
	    move(nll[0], nll[1], nll[2]); draw(fll[0], fll[1], fll[2]);
	    move(nlr[0], nlr[1], nlr[2]); draw(flr[0], flr[1], flr[2]);
	    move(nul[0], nul[1], nul[2]); draw(ful[0], ful[1], ful[2]);
	    move(nur[0], nur[1], nur[2]); draw(fur[0], fur[1], fur[2]);

	    setlinestyle(0);

	    color(YELLOW);
	    move(nur[0], nur[1], nur[2]); draw(nlr[0], nlr[1], nlr[2]);
	    draw(nll[0], nll[1], nll[2]); draw(nul[0], nul[1], nul[2]);
	    draw(nur[0], nur[1], nur[2]);

	    color(RED);
	    move(fur[0], fur[1], fur[2]); draw(flr[0], flr[1], flr[2]);
	    draw(fll[0], fll[1], fll[2]); draw(ful[0], ful[1], ful[2]);
	    draw(fur[0], fur[1], fur[2]);

	closeobj();
        break;

    case ORTHO:

	if (isobj(viewing_pyramid)) delobj(viewing_pyramid);

	makeobj(viewing_pyramid);

	    color(YELLOW);
	    pushmatrix();
	    translate(0.0, 0.0, -near);
	    rect(left, bottom, right, top);
	    popmatrix();

	    color(RED);
	    pushmatrix();
	    translate(0.0, 0.0, -far);
	    rect(left, bottom, right, top);
	    popmatrix();

	    setlinestyle(DOTTED);
	    move(left, bottom, -near); draw(left, bottom, -far);
	    move(left, top, -near); draw(left, top, -far);
	    move(right, top, -near); draw(right, top, -far);
	    move(right, bottom, -near); draw(right, bottom, -far);
	    setlinestyle(0);

	closeobj();
        break;
    }

}

setupcolors() {

    long planes;
    Colorindex i;

    deflinestyle(DOTTED, dotted);

    if (getplanes() < 4) {
	printf("You do not have enough bitplanes for this program\n");
	gexit();
	exit(0);
    }
    tutorsavemap();
    tutormakemap();
}

restore_colors() {
    Colorindex i;

    tutorrestoremap();
}

reset_values() {

    fovy = 400;
    aspect = 1.00;
    near = 2.5; far = 7.5;
    left = -1.5; right = 1.5; bottom = -1.5; top = 1.5;
    vx = 3.0; vy = 3.0; vz = 3.0; px = 0.0; py = 0.0; pz = 0.0;
    dist = 5.0;
    azim = 0; inc = 0; twist = 0;

    make_viewing_pyramid();
    if (viewing==LOOKAT) redo_theta_phi();

}

int which_function() {

    long sx, sy;
    long ox, oy;

    winset(statw);
    getsize(&sx, &sy);
    getorigin(&ox, &oy);

    if ((mx >= ox) && (mx <= sx+ox) && (my >= oy) && (my <= oy+sy)) {
	return(1);		    /* select parameter */
    }

    winset(consw);
    getsize(&sx, &sy);
    getorigin(&ox, &oy);

    if ((mx >= (ox + sx/2)-150) && (ox <= (ox + sx/2)+150) &&
	(my >= (oy + sy/2)-16) && (my <= (oy + sy/2)+16)) {
        return(2);		    /* control bar */
    }

    return(0);
}

draw_complete_slider() {

    color(UNPICKCOLOR);
    cmov2i(-80, 20);
    if (line==1)
	switch (projection) {
	case PERSP:
	    switch (parameter) {
	    case 1:
		charstr("field of view in y");
		draw_knob_angle(fovy);
		break;
	    case 2:
		charstr("   aspect ratio   ");
		draw_knob_float(aspect);
		break;
	    case 3:
		charstr("near clip distance");
		draw_knob_float(near);
		break;
	    case 4:
		charstr("far clip distance ");
		draw_knob_float(far);
		break;
	    default:
		charstr("  Controller Bar   ");
		draw_blank_slider(0.0, 0.0, 100, 10);
		break;
	    }
	    break;
	case ORTHO:
	case WINDOW:
	    switch (parameter) {
	    case 1:
	        charstr("       left       ");
		draw_knob_float(left);
	        break;
	    case 2:
	        charstr("       right      ");
		draw_knob_float(right);
	        break;
	    case 3:
	        charstr("      bottom       ");
		draw_knob_float(bottom);
	        break;
	    case 4:
	        charstr("        top        ");
		draw_knob_float(top);
	        break;
	    case 5:
	        charstr("near clip distance ");
		draw_knob_float(near);
	        break;
	    case 6:
	        charstr(" far clip distance ");
		draw_knob_float(far);
	        break;
	    default:
		charstr("  Controller Bar   ");
		draw_blank_slider(0.0, 0.0, 100, 10);
		break;
	    }
	    break;
	}
    else if (line==2)
	switch (viewing) {
	case POLAR:
	    switch (parameter) {
	    case 1:
		charstr("     distance     ");
		draw_knob_float(dist);
		break;
	    case 2:
		charstr("     azimuth      ");
		draw_knob_angle(azim);
		break;
	    case 3:
		charstr("     incline      ");
		draw_knob_angle(inc);
		break;
	    case 4:
		charstr("      twist       ");
		draw_knob_angle(twist);
		break;
	    default:
		charstr("  Controller Bar   ");
		draw_blank_slider(0.0, 0.0, 100, 10);
		break;
	    }
	    break;
	case LOOKAT:
	    switch (parameter) {
	    case 1:
		charstr("  x of viewpoint  ");
		draw_knob_float(vx);
		break;
	    case 2:
	        charstr("  y of viewpoint  ");
		draw_knob_float(vy);
	        break;
	    case 3:
	        charstr("  z of viewpoint  ");
		draw_knob_float(vz);
	        break;
	    case 4:
	        charstr("x of reference point");
		draw_knob_float(px);
	        break;
	    case 5:
	        charstr("y of reference point");
		draw_knob_float(py);
	        break;
	    case 6:
	        charstr("z of reference point");
		draw_knob_float(pz);
	        break;
	    case 7:
		charstr("      twist       ");
		draw_knob_angle(twist);
	        break;
	    default:
		charstr("  Controller Bar   ");
		draw_blank_slider(0.0, 0.0, 100, 10);
		break;
	    }
	    break;
	}
    else {
	charstr("  Controller Bar   ");
	draw_blank_slider(0.0, 0.0, 100, 10);
    }
}


draw_knob_angle(pos)
Angle pos;
{
    static char buffer[10];

    if ( (pos < 0) || ( (pos == 0) && (low_pos < 0) ) ) {
	high_pos = (float) ( (pos/300) * 300);
	low_pos = high_pos - 299.0;
        pos = pos - (int)low_pos + 1;
    } else {
        low_pos = (float) ( (pos/300) * 300 );
	high_pos = low_pos + 299.0;
        pos -= (int) low_pos;
    }

    draw_int_slider(0.0, 0.0, 100, 10, (float) pos, low_pos, high_pos);
}

draw_knob_float(val)
float val;
{
    int pos;
    static char buffer[10];

    if (val < 0.0) {
	high_pos = (float) (-0.01 + (int) ((val + 0.01) / 3.0) * 3);
	low_pos = high_pos - 2.99;
	pos = (int) ( (val - low_pos) * 100.0 );
    } else {
        low_pos = (float) ( (int) (val/3.0) * 3 );
	high_pos = low_pos + 2.99;
	pos = (int) ( (val - low_pos) * 100.0 );
    }

    draw_slider(0.0, 0.0, 100, 10, (float) pos, low_pos, high_pos);
}

print_float(row, column, string, number, last)
int row, column;
char string[];
float number;
Boolean last;	/* last parameter in function (print comma?) */
{
    int x, y;
    static buffer[20];

    x = 60 + 70 * column;
    y = 100 - 50 * row;

    if (parameter==column && line==row) color(HIGHCOLOR);
    else color(NORMCOLOR);
	
    cmov2i(x, y+20); charstr(string);
    sprintf(buffer, "%6.2f", number);
    cmov2i(x, y); charstr(buffer);
    color(NORMCOLOR);
    if (last)
#ifdef FORTRAN
	charstr(" )");
#else
	charstr(" );");
#endif
    else charstr(",");
}

print_int(row, column, string, number, last)
int row, column;
char string[];
int number;
{
    int x, y;
    static buffer[20];

    x = 60 + 70 * column;
    y = 100 - 50 * row;

    if (parameter==column && line==row) color(HIGHCOLOR);
    else color(NORMCOLOR);
	
    cmov2i(x, y+20); charstr(string);
    sprintf(buffer, "%6d", number);
    cmov2i(x, y); charstr(buffer);
    color(NORMCOLOR);
    if (last)
#ifdef FORTRAN
	charstr(" )");
#else
	charstr(" ); ");
#endif
    else charstr(",");
}

modify_persp() {
    switch (parameter) {
	case 1:
	    fovy = low_pos + (Angle) dx + (Angle) (wrap * 300.0);
	    break;
	case 2:
	    aspect = low_pos + (float) dx / 100.0 + wrap * 3.0;
	    break;
	case 3:
	    near = low_pos + (Coord) dx / 100.0 + (Coord) (wrap * 3.0);
	    break;
	case 4:
	    far = low_pos + (Coord) dx / 100.0 + (Coord) (wrap * 3.0);
	    break;
	default:
	    break;
    }
}

modify_polar() {

    switch (parameter) {
	case 1:
	    dist = low_pos + (float) dx/100.0 + wrap * 3.0;
            break;
        case 2:
	    azim = low_pos + (Angle)dx + (Angle)(wrap * 300.0);
            break;
        case 3:
            inc = low_pos + (Angle) dx + (Angle) (wrap * 300.0);
	    break;
        case 4:
	    twist = low_pos + (Angle) dx + (Angle) (wrap * 300.0);
            break;
	default:
	    break;
    }
}

modify_lookat() {
    switch (parameter) {
    case 1:
        vx = low_pos + (float) dx/100.0 + wrap * 3.0;
        break;
    case 2:
        vy = low_pos + (float) dx/100.0 + wrap * 3.0;
        break;
    case 3:
        vz = low_pos + (float) dx/100.0 + wrap * 3.0;
        break;
    case 4:
        px = low_pos + (float) dx/100.0 + wrap * 3.0;
        break;
    case 5:
        py = low_pos + (float) dx/100.0 + wrap * 3.0;
        break;
    case 6:
        pz = low_pos + (float) dx/100.0 + wrap * 3.0;
        break;
    case 7:
        twist = low_pos + (Angle) dx + (Angle) (wrap * 300.0);
        break;
    default:
        break;
    }
}

modify_ortho() {
    switch (parameter) {
    case 1:
        left = low_pos + (float) dx/100.0 + wrap * 3.0;
        break;
    case 2:
        right = low_pos + (float) dx/100.0 + wrap * 3.0;
        break;
    case 3:
        bottom = low_pos + (float) dx/100.0 + wrap * 3.0;
        break;
    case 4:
	top = low_pos + (float) dx/100.0 + wrap * 3.0;
        break;
    case 5:
        near = low_pos + (float) dx/100.0 + wrap * 3.0;
        break;
    case 6:
        far = low_pos + (float) dx/100.0 + wrap * 3.0;
        break;
    }
}

modify_window() {
    switch (parameter) {
    case 1:
        left = low_pos + (float) dx/100.0 + wrap * 3.0;
        break;
    case 2:
        right = low_pos + (float) dx/100.0 + wrap * 3.0;
        break;
    case 3:
        bottom = low_pos + (float) dx/100.0 + wrap * 3.0;
        break;
    case 4:
	top = low_pos + (float) dx/100.0 + wrap * 3.0;
        break;
    case 5:
        near = low_pos + (float) dx/100.0 + wrap * 3.0;
        break;
    case 6:
        far = low_pos + (float) dx/100.0 + wrap * 3.0;
        break;
    }
}


make_popup()
{
#ifdef FORTRAN
    projmenu = defpup(" Projection transforms %t| call ortho  %f| call window  %f| call perspective  %f", p1, p2, p3);
    viewmenu = defpup(" Viewing transforms %t| call polarview %f| call lookat  %f", v1, v2);
#else
    projmenu = defpup(" Projection transforms %t| ortho()  %f| window()  %f| perspective()  %f", p1, p2, p3);
    viewmenu = defpup(" Viewing transforms %t| polarview() %f| lookat()  %f", v1, v2);
#endif

    menu = defpup(" Projection %t| Projection transform|Viewing transform|Reset parameters%f|Exit",
	reset_values);
}

p1() {
    projection=ORTHO;
    make_viewing_pyramid();
}

p2() {
    projection=WINDOW;
    make_viewing_pyramid();
}

p3() {
    projection=PERSP;
    make_viewing_pyramid();
}

v1() {
    viewing=POLAR;
    make_viewing_pyramid();
}

v2() {
    viewing=LOOKAT;
    redo_theta_phi();
    make_viewing_pyramid();
}

/*---------------------------------------------------------------------------
 * My stuff.  mjc
 *---------------------------------------------------------------------------
 */

init_windows()
/*---------------------------------------------------------------------------
 * Initialize all of the windows to be used.
 *---------------------------------------------------------------------------
 */
{
    backw = init_back();
    winattach(backw);
    doublebuffer();
    gconfig();
    helpw = init_help("Projection");
    statw = init_state();
    consw = init_cons();
    frontw = init_front();
    viewxw = init_view(X);
    viewzw = init_view(Z);
}

init_state()
/*---------------------------------------------------------------------------
 * Initialize the status window.
 *---------------------------------------------------------------------------
 */
{
    int res;

    prefposition(367, 1017, 475, 585);
    res = winopen("sat");
    wintitle("Projection -- STATUS");
    prefsize(650, 110);
    winconstraints();
    ortho2(-10.0, 640.0, -20.0, 90.0);

    return(res);
}

init_cons()
/*---------------------------------------------------------------------------
 * Initialize the console window.
 *---------------------------------------------------------------------------
 */
{
    int res;

    prefposition(522, 1022, 609, 735);
    res = winopen("cons");
    wintitle("Projection -- CONTROL BAR");
    prefsize(500, 126);
    winconstraints();
    reshapeviewport();
    ortho2(-250.0, 250.0, -63.0, 63.0);

    return(res);
}

init_front()
/*---------------------------------------------------------------------------
 * Initialize the front window.
 *---------------------------------------------------------------------------
 */
{
    int res;

    prefposition(450, 850, 25, 425);
    res = winopen("front");
    wintitle("Projection -- VIEWPORT");
    keepaspect(1, 1);
    winconstraints();
    reshapeviewport();

    return(res);
}

init_view(n)
/*---------------------------------------------------------------------------
 * Initialize the view window, X or Z.
 *---------------------------------------------------------------------------
 */
int n;
{
    int res;

    switch(n){
    case X:
	prefposition(85, 335, 25, 275);
	res = winopen("X");
	wintitle("Projection -- DOWN X AXIS");
	keepaspect(1, 1);
	winconstraints();
	break;
    case Z:
	prefposition(85, 335, 325, 575);
	res = winopen("Z");
	wintitle("Projection -- DOWN Z AXIS");
	keepaspect(1, 1);
	winconstraints();
	break;
    }

    return(res);
}

draw_frame()
/*---------------------------------------------------------------------------
 * Draw all of the windows.
 *---------------------------------------------------------------------------
 */
{
    draw_back();
    draw_help();
    draw_state();
    draw_cons();
    draw_front();
    draw_view(X);
    draw_view(Z);

    swapbuffers();
}

draw_state()
/*---------------------------------------------------------------------------
 * Draw the status window.
 *---------------------------------------------------------------------------
 */
{
    winset(statw);
    color(BLACK);
    clear();
    draw_text();
}

draw_cons()
/*---------------------------------------------------------------------------
 * Draw the console window.
 *---------------------------------------------------------------------------
 */
{
    winset(consw);
    color(BLACK);
    clear();
    reshapeviewport();
    draw_complete_slider();
}

draw_front()
/*---------------------------------------------------------------------------
 * Draw the front window.
 *---------------------------------------------------------------------------
 */
{
    winset(frontw);
    color(BLACK);
    clear();
    reshapeviewport();
    draw_viewport();
}

draw_view(n)
/*---------------------------------------------------------------------------
 * Draw the view window, X or Z.
 *---------------------------------------------------------------------------
 */
int n;
{
    switch(n){
    case X:
	winset(viewxw);
	color(BLACK);
	clear();
	reshapeviewport();
	draw_side_view();
	break;
    case Z:
	winset(viewzw);
	color(BLACK);
	clear();
	reshapeviewport();
	draw_top_view();
	break;
    }
}
