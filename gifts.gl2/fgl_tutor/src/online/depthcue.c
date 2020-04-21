/*  depthcue.c							*/
/*  April, 1986 -- Thant Tessman				*/
/*  Souped up Learning Environment (16 bitplanes)		*/

#include <gl.h>
#include <device.h>
#include "tutor.h"

#define BARX	0
#define BARY	0

#define FOVY		1
#define ASPECT		2
#define NEARCLIP	3
#define FARCLIP		4
#define NEAR		5
#define FAR		6
#define LOWINDEX 	7
#define HIGHINDEX	8
#define Z1		9
#define Z2		10
#define SLIDER		11
#define MAP		12

#define FOVERROR	100
#define CLIPERROR	101

short oldcolors[64][3];
char buffer[80];
int item=0;
Screencoord z1=0, z2=1023;
Screencoord near=0, far=1023;
Colorindex lowindex=16, highindex=31;
float low_pos, high_pos;
Angle azim=100, inc = -800;
float dist=5.0;
Angle fovy=400;
float aspect=1.0;
Coord nearclip=3.5, farclip=6.5;
short oldx, newx, oldy, newy;
float wrap;
long dx;
Boolean active=TRUE;

Object letter;

Boolean near_exit=FALSE;

int	helpw, 
	backw, 
	rampw, 
	statw, 
	frontw, 
	sidew, 
	consw;

int menu, exitmenu;

main () {
    short   val;
    long    dev;
    int	    pickme();
    int i;

    if (!ismex ()) {
	printf("This program can only be run under the window manager.\n");
 	printf("Type `mex' to start the window manager.\n");
        exit ();
    }

    init_windows();
    setup_colors();
    make_objects();
    init_queue();
    load_correct_help(0);

    menu = defpup("Depthcue %t|Reset|Exit");
    exitmenu = defpup("Exit Confirmation %t|Yes|No");

    while (TRUE) {
	while (qtest ()) {
	    dev = qread (&val);
	    switch (dev) {
		case LEFTMOUSE: 
		    if (val)
			pickme();
		    break;
		case MIDDLEMOUSE:
		    if (val) {
			qread(&oldx);
			qread(&oldy);
			reorient();
		    }
		    break;
		case RIGHTMOUSE:
		    if (val)
			switch (dopup(menu)) {
			    case 1:
				reset_values();
				break;
			    case 2:
				if (dopup(exitmenu)==1) {
				    restore_colors();
				    gexit(); exit();
				}
				break;
		        }
		    break;
		case REDRAW: 
		    redraw_window(val);
		    break;
		case INPUTCHANGE: 
		    active = val;
		    load_correct_help(0);
		    if (active == FALSE) {
			draw_frame();
			draw_frame();
		    }
		    break;
		case ESCKEY:
		    restore_colors();
		    gexit(); exit(0);
		    break;
		default:
		    break;
	    }
	}
	draw_frame();
    }
}

int pickme () {

    short   buffer[100];
    long    numpicked;
    int     i, picked;
    static oldpicked;


	winset(statw);
	pushmatrix();
	pick (buffer, 100);
	ortho2(-10.0, 450.0, -10.0, 60.0);
	initnames ();

	loadname( (short) FOVY);
	rectfi(120, 35, 199, 54);

	loadname( (short) ASPECT);
	rectfi(200, 35, 279, 54);

	loadname( (short) NEARCLIP);
	rectfi(280, 35, 359, 54);

	loadname( (short) FARCLIP);
	rectfi(360, 35, 439, 54);

	loadname( (short) NEAR);
	rectfi(120, 15, 199, 34);

	loadname( (short) FAR);
        rectfi(200, 15, 279, 34);

	loadname( (short) LOWINDEX);
        rectfi(120, -5, 199, 14);

	loadname( (short) HIGHINDEX);
        rectfi(200, -5, 279, 14);

	loadname( (short) Z1);
        rectfi(280, -5, 359, 14);

	loadname( (short) Z2);
        rectfi(360, -5, 439, 14);

	numpicked = endpick (buffer);
	popmatrix();

	if (numpicked){
	    picked = buffer[1];
	    oldpicked = picked;
	    item = picked;
	    load_correct_help(-1);
	    return(picked);
	}

	winset(consw);
	pushmatrix();
	pick (buffer, 100);
	    ortho2(-250.0, 250.0, -63.0, 63.0);
	    loadname( (short) SLIDER);
	    draw_blank_slider(0.0, 0.0, 100, 10);
	numpicked = endpick (buffer);
	popmatrix();
	load_correct_help(0);

	if (numpicked) {
	    picked = buffer[1];
	} else 
	    picked = oldpicked;

	if (picked==SLIDER)
	    change_parameter();
	else 
	   item = picked;

    return(picked);
}

change_parameter() {

    short mx;
    long iwx;
    long sx, sy;
    long ox, oy;

    while (getbutton(LEFTMOUSE)) {

	wrap = 0.0;

	winset(consw);
	getsize(&sx, &sy);
	getorigin(&ox, &oy);
	mx = (short) getvaluator(MOUSEX);

	iwx = mx - (ox + sx/2);

	if (iwx < -150) {
	    if (item!=LOWINDEX && item!=HIGHINDEX) {
		iwx += 300;
		setvaluator(MOUSEX, iwx+ox+sx/2, 0, 1023);
		wrap = -1.0;
	    } else {
		iwx = -150;
	    }
	}

	else if (iwx > 149) {
	    if (item!=LOWINDEX && item!=HIGHINDEX) {
		iwx -= 300;
		setvaluator(MOUSEX, iwx+ox+sx/2, 0, 1023);
		wrap = 1.0;
	    } else {
		iwx = 149;
	    }
	}

	dx = (iwx + 150);

	modify_item();

	draw_frame();
    }
}

modify_item() {
    switch (item) {
	case FOVY:
	    fovy = low_pos + (Angle) dx + (Angle) (wrap * 300.0);
	    break;
	case ASPECT:
	    aspect = low_pos + (float) dx / 100.0 + wrap * 3.0;
	    break;
	case NEARCLIP:
	    nearclip = low_pos + (Coord) dx / 100.0 + (Coord) (wrap * 3.0);
	    break;
	case FARCLIP:
	    farclip = low_pos + (Coord) dx / 100.0 + (Coord) (wrap * 3.0);
	    break;
	case NEAR:
	    near = low_pos + (Angle) dx + (Angle) (wrap * 300.0);
	    break;
	case FAR:
	    far = low_pos + (Angle) dx + (Angle) (wrap * 300.0);
	    break;
	case LOWINDEX:
	    lowindex = (Colorindex) dx*64.0/299.0;
	    break;
	case HIGHINDEX:
	     highindex = (Colorindex) dx*64.0/299.0;
	    break;
	case Z1:
	    z1 = low_pos + (Angle) dx + (Angle) (wrap * 300.0);
	    break;
	case Z2:
	    z2 = low_pos + (Angle) dx + (Angle) (wrap * 300.0);
	    break;
	default:
	    break;
    }


    if(item == HIGHINDEX)
	if (highindex < lowindex)
	    highindex = lowindex;

    if (item == LOWINDEX)
	if(highindex < lowindex)
	    lowindex = highindex;
}

load_correct_help(s)
int s;
{
    if (!active) {
	load_help("Press the RIGHT MOUSE button and select ATTACH.", 
		  "", "", "", "", "");
	return;
    }

    if ( (fovy<2) || (fovy>1799) ) {
    load_help("Field of view must be between 2 and 1799.", "", "", "", "","");
    return;
    }

    if (nearclip==farclip) {
 load_help("The far clipping plane shouldn't equal the near clipping plane.", 
	    "", "", "", "", "");
	return;
    }

    if (aspect==0.0) {
	load_help("The aspect ratio can't be zero.","","","","","");
	return;
    }

    switch (s) {
	case 0:
    load_help("Press the LEFT MOUSE button to select a parameter to", 
	      "  modify.", 
	      "Press the MIDDLE MOUSE button and move the mouse to", 
	      "  reorient the object.", "", "");
	    break;
	case HIGHINDEX:
    load_help("Use the LEFT MOUSE button on the color map to select the end", 
	      "  of a color range for depthcueing.", 
	      "  `highindex' can't be lower than `lowindex.'", "", "", "");
	    break;
	case LOWINDEX:
    load_help("Use the LEFT MOUSE button on the color map to select the", 
	      "  beginning of a color range for depthcueing.", 
	      "  `lowindex' can't be higher than `highindex.'");
	    break;
	default:
    load_help("Use the LEFT MOUSE button on the Controller Bar to", 
	      "  modify the value of the selected parameter.", "", "",
	      "", "");
	    break;
    }
}

clear_values() {
    color(BLACK);
    rectfi(1, 512, 510, 637);
}

draw_values (highlighted)
int highlighted;
{
    int i;

    color(NORMCOLOR);
    cmov2i(0, 40);
#ifdef FORTRAN
    charstr("CALL PERSPE (");
#else
    charstr("perspective (");
#endif

    cmov2i(120, 40);
    if (highlighted==FOVY) color(HIGHCOLOR); else color(NORMCOLOR);
    sprintf(buffer, "%6d", fovy);
    charstr(buffer);
    color(NORMCOLOR); charstr(",");

    cmov2i(200, 40);
    if (highlighted==ASPECT) color(HIGHCOLOR); else color(NORMCOLOR);
    sprintf(buffer, "%7.2f", aspect);
    charstr(buffer);
    color(NORMCOLOR); charstr(",");

    cmov2i(280, 40);
    if (highlighted==NEARCLIP) color(HIGHCOLOR); else color(NORMCOLOR);
    sprintf(buffer, "%7.2f", nearclip);
    charstr(buffer);
    color(NORMCOLOR); charstr(",");

    cmov2i(360, 40);
    if (highlighted==FARCLIP) color(HIGHCOLOR); else color(NORMCOLOR);
    sprintf(buffer, "%7.2f", farclip);
    charstr(buffer);
    color(NORMCOLOR);
#ifdef FORTRAN
    charstr(" )");
#else
    charstr(" );");
#endif

    cmov2i(0, 20);
#ifdef FORTRAN
    charstr("CALL SETDEP ( ");
#else
    charstr("setdepth    ( ");
#endif

    cmov2i(120, 20);
    if (highlighted==NEAR) color(HIGHCOLOR); else color(NORMCOLOR);
    sprintf(buffer, "%6d", near);
    charstr(buffer);
    color(NORMCOLOR); charstr(",");

    cmov2i(200, 20);
    if (highlighted==FAR) color(HIGHCOLOR); else color(NORMCOLOR);
    sprintf(buffer, "%6d", far);
    charstr(buffer);

    color(NORMCOLOR);
#ifdef FORTRAN
    charstr(" )");
#else
    charstr(" );");
#endif

    cmov2i(0, 0);
#ifdef FORTRAN
    charstr("CALL SHADER ( ");
#else
    charstr("shaderange  ( ");
#endif

    cmov2i(120, 0);
    if (highlighted==LOWINDEX) color(HIGHCOLOR); else color(NORMCOLOR);
    sprintf(buffer, "%6d", lowindex);
    charstr(buffer);
    color(NORMCOLOR); charstr(",");

    cmov2i(200, 0);
    if (highlighted==HIGHINDEX) color(HIGHCOLOR); else color(NORMCOLOR);
    sprintf(buffer, "%6d", highindex);
    charstr(buffer);
    color(NORMCOLOR); charstr(",");

    cmov2i(280, 0);
    if (highlighted==Z1) color(HIGHCOLOR); else color(NORMCOLOR);
    sprintf(buffer, "%6d", z1);
    charstr(buffer);
    color(NORMCOLOR); charstr(",");

    cmov2i(360, 0);
    if (highlighted==Z2) color(HIGHCOLOR); else color(NORMCOLOR);
    sprintf(buffer, "%6d", z2);
    charstr(buffer);

    color(NORMCOLOR);
#ifdef FORTRAN
    charstr(" )");
#else
    charstr(" );");
#endif
}

restore_colors() {
    Colorindex i;

    for (i=0; i<64; i++) 
	mapcolor(i, oldcolors[i][0], oldcolors[i][1], oldcolors[i][2]);
}

redraw_everything() {

    clear_values();
    draw_values(item);

    clear_map();
    draw_map();

    clear_main_view();
    draw_main_view();

    clear_side_view();
    draw_side_view();

}

clear_slider() {
    color(BLACK);
    rectfi(645, 639, 1022, 766);
}

draw_complete_slider() {

    color(UNPICKCOLOR);
    cmov2i(BARX-80, BARY+23);
    switch (item) {
	case FOVY:
	    charstr("field of view in y");
	    draw_knob_angle( fovy );
	    break;
	case ASPECT:
	    charstr("   aspect ratio   ");
	    draw_knob_float(aspect);
	    break;
	case NEARCLIP:
	    charstr("near clipping plane");
	    draw_knob_float(nearclip);
	    break;
	case FARCLIP:
	    charstr("far clipping plane ");
	    draw_knob_float(farclip);
	    break;
	case LOWINDEX:
	    charstr("low color index ");
	    draw_int_slider(0.0, 0.0, 100, 10, (float) lowindex *299.0/64.0, 0.0, 64.0);
	    break;
	case HIGHINDEX:
	    charstr("high color index ");
	    draw_int_slider(0.0, 0.0, 100, 10, (float) highindex * 299.0/64.0, 0.0, 64.0);
	    break;
	case NEAR:
	    charstr(" map near to coord ");
	    draw_knob_angle( (int) near);
	    break;
	case FAR:
	    charstr(" map far to coord  ");
	    draw_knob_angle( (int) far);
	    break;
	case Z1:
	    charstr("        z1        ");
	    draw_knob_angle( (int) z1);
	    break;
	case Z2:
	    charstr("        z2        ");
	    draw_knob_angle( (int) z2);
	    break;
	default:
	    charstr("  Controller Bar   ");
	    draw_blank_slider(0.0, 0.0, 100, 10);
	    break;
    }
}

draw_knob_angle(pos)
int pos;
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

    if ( (val < 0.0) || ( (val==0.0) && (low_pos<0.0) ) ) {
	high_pos = (float) ( (int) ((val-0.01)/3.0) * 3);
	low_pos = high_pos - 2.99;
	pos = (int) ( (val - low_pos) * 100.0 ) + 1;
    } else {
        low_pos = (float) ( (int) (val/3.0) * 3 );
	high_pos = low_pos + 2.99;
	pos = (int) ( (val - low_pos) * 100.0 );
    }

    draw_slider(0.0, 0.0, 100, 10, (float) pos, low_pos, high_pos);
}

setup_colors() {

    int i;

    if (getplanes () < 6) {	/*  if less than 6 bitplanes in mex	 */
	printf("You do not have enough bitplanes for this program\n");
	gexit();
	exit(0);
    }
    for (i=0;i<64;i++) {
	getmcolor(i, oldcolors[i], oldcolors[i]+1, oldcolors[i]+2);
    }

    tutorsavemap();
    tutormakemap();

    makerange( (Colorindex) 16, (Colorindex) 31, 0, 0, 0, 255, 0, 255);
    makerange( (Colorindex) 32, (Colorindex) 47, 0, 0, 0, 0, 255, 200);
    makerange( (Colorindex) 48, (Colorindex) 63, 0, 0, 0, 255, 255, 255);
}

draw_main_view() {

    pushviewport();
    pushmatrix();

    setdepth(near, far);
    shaderange(lowindex, highindex, z1, z2);
    depthcue(TRUE);

    if ( !((fovy<2) || (fovy>1799))
    && !(nearclip==farclip)
    && !(aspect==0.0)) {
	perspective(fovy, aspect, nearclip, farclip);
	polarview(dist,
	          azim + (Angle)(newx-oldx),
		  inc + (Angle)(newy-oldy),
		  0);
	callobj(letter);
    } else {
	load_correct_help();
    }

    depthcue(FALSE);

    popviewport();
    popmatrix();

}

clear_main_view() {
    color(BLACK);
    rectfi(1, 1, 510, 510);
}

clear_side_view() {
    color(BLACK);
    rectfi(512, 1, 1022, 510);
}

draw_side_view() {

    Colorindex i;
    Coord z;
    float glsin, glcos;

    pushviewport();
    pushmatrix();


    rotate( 900, 'y');

    rotate( -(inc + (Angle)(newy-oldy)), 'x');

    rotate( -(azim + (Angle)(newx-oldx)), 'z');

    color(CYAN);
    callobj(letter);

    ortho(6.0, -2.0, -4.0, 4.0, -4.0, 4.0);
    color(NORMCOLOR);

    gl_sincos(fovy/2, &glsin, &glcos);
    move( dist - 10.0 * glcos, 10.0 * glsin);
    draw( dist, 0.0, 0.0);
    draw( dist - 10.0 * glcos, -10.0 * glsin);

    cmov( dist + 0.6, 0.0, 0.0);
    charstr("eye");
    cmov( dist-nearclip+0.6, -3.8, 0.0);
    charstr("near ");
    cmov( dist-farclip+0.5, -3.8, 0.0);
    charstr("far ");

    move( dist-nearclip, 4.0, 0.0);
    draw( dist-nearclip, -4.0, 0.0);
    move( dist-farclip, 4.0, 0.0);
    draw( dist-farclip, -4.0, 0.0);

/*    color(YELLOW);
    cmov( dist-nearclip, 2.8, 0.0);
    charstr("   advertised");
    cmov( dist-nearclip, -2.5, 0.0);
    charstr("   actual?");	*/

/*    linewidth( (short)8 );
    if (lowindex!=highindex && far!=near)
    for (i=lowindex; i<=highindex; i++) {		*/

	/* screen space */

/*	z = (float) ( (i-highindex) * (z2-z1) )
	    / (float) (lowindex-highindex) 
	    + (float) z1;				*/

	/* world space */

/*	z = z * (farclip - nearclip) / (float) (far-near) + nearclip; */

/*	color(i);
	if (i==lowindex)
	    move(dist - z, 2.5, 0.0);
	else
	    draw(dist - z, 2.5, 0.0);
    }							*/

    setdepth(near, far);
    shaderange(lowindex, highindex, z1, z2);
    depthcue(TRUE);
    move(dist-nearclip, -2.7, 4.0);
    draw(dist-farclip, -2.7, -4.0);
    move(dist-nearclip, -2.68, 4.0);
    draw(dist-farclip, -2.68, -4.0);
    move(dist-nearclip, -2.66, 4.0);
    draw(dist-farclip, -2.66, -4.0);
    move(dist-nearclip, -2.64, 4.0);
    draw(dist-farclip, -2.64, -4.0);
    move(dist-nearclip, -2.62, 4.0);
    draw(dist-farclip, -2.62, -4.0);
    depthcue(FALSE);
    linewidth( (short) 1);
    popviewport();
    popmatrix();
}

reorient() {

    while(getbutton(MIDDLEMOUSE)) {
	newx = getvaluator(MOUSEX);
	newy = getvaluator(MOUSEY);
	draw_frame();
    }

    azim += (Angle) (newx-oldx);
    inc += (Angle) (newy-oldy);
    newx=0; newy=0; oldx=0; oldy=0;
}

make_objects() {

    makeobj(letter=genobj());
	rotate(-900, 'x');
	draw_fflat(.5);
	draw_fflat(.25);
	draw_fflat(0.0);
	draw_fflat(-.25);
	draw_fflat(-.5);
	draw_fedges(.75);
    closeobj();

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

clear_map() {
    color(BLACK);
    rectfi(645, 512, 1022, 637);
}

draw_map() {

    int i, j;
    Colorindex c;

    color(WHITE);
    rectfi(lowindex * 20, 0, highindex * 20 + 20, 20);

    for (i = 0 ; i < 64 ; i++){
	color(i);
	rectfi(i*20 + 2, 2, i*20 + 18 , 18);
    }
}

makerange(a,b,r1,g1,b1,r2,g2,b2)

Colorindex a,b;
RGBvalue r1,r2,g1,g2,b1,b2;

{
	float i;
	int j;
	float dr,dg,db;

	i = (float)(b-a);
	dr = (float)(r2-r1)/i;
	dg = (float)(g2-g1)/i;
	db = (float)(b2-b1)/i;

	for (j=0;j<=(int)i;j++)
		mapcolor((Colorindex)j+a,
			 (RGBvalue) (dr * (float)j + r1),
			 (RGBvalue) (dg * (float)j + g1),
			 (RGBvalue) (db * (float)j + b1));
}

reset_values() {
	z1=0;
	z2=1023;
	near=0;
	far=1023;
	lowindex=16;
	highindex=31;
	fovy=400;
	aspect=1.0;
	dist=5.0;
	nearclip=3.5;
	farclip=6.5;
}

/*---------------------------------------------------------------------------
 * My stuff.  mjc
 *---------------------------------------------------------------------------
 */

init_windows()
/*---------------------------------------------------------------------------
 * Open all of the windows.
 *---------------------------------------------------------------------------
 */
{
    backw = init_back();
    doublebuffer();
    gconfig();
    winattach(backw);
    helpw = init_help("Depthcue -- INFORMATION");
    consw = init_cons();
    rampw = init_ramp();
    statw = init_state();
    frontw = init_front();
    sidew = init_side();

    
}

init_cons()
/*---------------------------------------------------------------------------
 * Initialize the console window.
 *---------------------------------------------------------------------------
 */
{
    int res;

    prefposition(516, 1016, 609, 735);
    res = winopen("cons");
    wintitle("Depthcue -- CONTROL BAR");
    prefsize(500, 126);
    winconstraints();
    reshapeviewport();
    ortho2(-250.0, 250.0, -63.0, 63.0);

    return(res);
}

init_ramp()
/*---------------------------------------------------------------------------
 * Initialize the ramp window.
 *---------------------------------------------------------------------------
 */
{
    int res;
    
    prefposition(4, 1019, 424, 424+54);
    res = winopen("ramp");
    wintitle("Depthcue -- RAMP");
    keepaspect(322, 17);
    winconstraints();
    reshapeviewport();
    ortho2(-4.0, 1284.0, -14.0, 34.0);
    
    return(res);
}

init_state()
/*---------------------------------------------------------------------------
 * Initialize the state window.
 *---------------------------------------------------------------------------  */
{
    int res;

    prefposition(542, 542 + 460, 512, 512 + 70);
    res = winopen("stat");
    wintitle("Depthcue -- STATUS");
    prefsize(460, 70);
    winconstraints();
    reshapeviewport();
    ortho2(-10.0, 450.0, -10.0, 60.0);

    return(res);
}

init_front()
/*---------------------------------------------------------------------------
 * Init the main window.
 *---------------------------------------------------------------------------
 */
{
    int res;

	prefposition(92, 92+395, 5, 5+395);
	res = winopen("frop");
	wintitle("Depthcue -- MAIN VIEW");
	keepaspect(1, 1);
	winconstraints();
	reshapeviewport();
	perspective(fovy, aspect, nearclip, farclip);
    return(res);
}

init_side()
/*---------------------------------------------------------------------------
 * Initialize the side view.
 *---------------------------------------------------------------------------
 */
{
    int res;

    prefposition(494, 494 + 394, 6, 6 + 394);
    res = winopen("side");
    wintitle("Depthcue -- SIDE VIEW");
    keepaspect(1, 1);
    winconstraints();
    reshapeviewport();
    ortho(6.0, -2.0, -4.0, 4.0, -4.0, 4.0);

    return(res);
}

draw_frame()
/*---------------------------------------------------------------------------
 * Draw a frame of the program.
 *---------------------------------------------------------------------------
 */
{
    draw_back();
    draw_help();
    draw_cons();
    draw_ramp();
    draw_state();
    draw_front();
    draw_side();

    swapbuffers();
}

draw_cons()
/*---------------------------------------------------------------------------
 * Draw the console window.
 *---------------------------------------------------------------------------
 */
{
    winset(consw);
    reshapeviewport();
    color(BLACK);
    clear();
    draw_complete_slider();
}


draw_ramp()
/*---------------------------------------------------------------------------
 * Draw the ramp window.
 *---------------------------------------------------------------------------
 */
{
    winset(rampw);
    reshapeviewport();
    color(BLACK);
    clear();
    draw_map();

}

draw_state()
/*---------------------------------------------------------------------------
 * Draw the state window.
 *---------------------------------------------------------------------------
 */
{
    winset(statw);
    reshapeviewport();
    color(BLACK);
    clear();
    draw_values(item);
}

draw_front()
/*---------------------------------------------------------------------------
 * Init the main window.
 *---------------------------------------------------------------------------
 */
{
    winset(frontw);
    reshapeviewport();
    color(BLACK);
    clear();
    draw_main_view();
}

draw_side()
/*---------------------------------------------------------------------------
 * Draw the side view.
 *---------------------------------------------------------------------------
 */
{
    winset(sidew);
    reshapeviewport();
    color(BLACK);
    clear();
    draw_side_view();
}

init_queue()
/*---------------------------------------------------------------------------
 * Initialize the queue.
 *---------------------------------------------------------------------------
 */
{
    qdevice (LEFTMOUSE);
    qdevice (RIGHTMOUSE);
    qdevice (MIDDLEMOUSE);
    tie(MIDDLEMOUSE, MOUSEX, MOUSEY);
    qdevice (INPUTCHANGE);
    qdevice (REDRAW);
    qdevice (ESCKEY);
}

redraw_window(n)
/*---------------------------------------------------------------------------
 * Redraw window n.
 *---------------------------------------------------------------------------
 */
int n;
{
    if (n == helpw){
	draw_help();
	swapbuffers();
	draw_help();
    } else if (n == backw){
	draw_back();
	swapbuffers();
	draw_back();
    } else if (n == statw){
	draw_state();
	swapbuffers();
	draw_state();
    } else if (n == frontw) {
	draw_front();
	swapbuffers();
	draw_front();
    } else if (n == sidew){
	draw_side();
	swapbuffers();
	draw_side();
    } else if (n == consw){
	draw_cons();
	swapbuffers();
	draw_cons();
    } else if (n == rampw){
	draw_ramp();
	swapbuffers();
	draw_ramp();
    }
}

draw_windata()
{
    long ox, oy;
    long sx, sy;

    winset(helpw);
    getorigin(&ox, &oy);
    getsize(&sx, &sy);
    printf("help: ox = %d oy = %d sx = %d sy = %d\n", ox, oy, sx, sy);

    winset(consw);
    getorigin(&ox, &oy);
    getsize(&sx, &sy);
    printf("cons: ox = %d oy = %d sx = %d sy = %d\n", ox, oy, sx, sy);

    winset(rampw);
    getorigin(&ox, &oy);
    getsize(&sx, &sy);
    printf("ramp: ox = %d oy = %d sx = %d sy = %d\n", ox, oy, sx, sy);

    winset(sidew);
    getorigin(&ox, &oy);
    getsize(&sx, &sy);
    printf("side: ox = %d oy = %d sx = %d sy = %d\n", ox, oy, sx, sy);

    winset(statw);
    getorigin(&ox, &oy);
    getsize(&sx, &sy);
    printf("state: ox = %d oy = %d sx = %d sy = %d\n", ox, oy, sx, sy);

    winset(frontw);
    getorigin(&ox, &oy);
    getsize(&sx, &sy);
    printf("view: ox = %d oy = %d sx = %d sy = %d\n", ox, oy, sx, sy);

}
