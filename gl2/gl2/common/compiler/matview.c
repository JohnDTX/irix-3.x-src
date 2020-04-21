/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

#include "globals.h"
#include "TheMacro.h"
#include "math.h"

#define POLARVIEW_SIZE 48
#define LOOKAT_SIZE 54

INTERP_NAME (polarview);
INTERP_NAME (lookat);

extern int i_polarview(), i_lookat(),
	i_translate(), i_rotatex(), i_rotatey(), i_rotatez();

/*
 *  concatenates a polarview matrix onto the transformation
 * stack thus modifying the current transformation
 */
void polarview(dist, azimuth, incidence, twist)
float dist;
long azimuth, incidence, twist;
{
    float cosine, sine;

    /* if not in immediate mode then take care of the data	*/
    if (gl_openobjhdr != 0) {
	if (gl_replacemode) {	/* yuck yuck	*/
	    if (gl_checkspace(POLARVIEW_SIZE) == 0) return;
	}
	else {
	    if (gl_makeroom(POLARVIEW_SIZE) == 0) return;
	    if (gl_checkspace(10) == 0) return;
	}
	BEGINCOMPILE(10);
	ADDADDR(i_polarview);
	ADDFLOAT(dist);
	ADDLONG(azimuth);
	ADDLONG(incidence);
	ADDLONG(twist);
	ENDCOMPILE;
    }

    translate (0.0,0.0,-dist);
    rotate (-twist,'z');
    rotate (-incidence,'x');
    rotate (-azimuth,'z');
}

/*
 * concatenates a lookat matrix onto the transformation
 * stack thus modifying the current transformation
 */
void lookat(vx, vy, vz, px, py, pz, twist)
    float vx, vy, vz, px, py, pz;
    long twist;
{
    float sine, cosine, hyp, hyp1, dx, dy, dz;

    beginpicmandef(LOOKAT_SIZE);
    BEGINCOMPILE(LOOKAT_SIZE);
    /* note that this is a real waste in immediate mode but it makes
    	the code SO!!!! much nicer and it doesn't take too long compared
	to the floating point done later	*/
    ADDADDR(i_lookat);
    ADDFLOAT(vx);
    ADDFLOAT(vy);
    ADDFLOAT(vz);
    ADDFLOAT(px);
    ADDFLOAT(py);
    ADDFLOAT(pz);
    ADDLONG(twist);

    gl_sincos (-twist,&sine,&cosine);	/* use table lookup	*/
    ADDADDR(i_rotatez);			/* rotatez (-twist)	*/
    ADDFLOAT(cosine);
    ADDFLOAT(sine);
    ADDFLOAT(-sine);
    ADDFLOAT(cosine);

    dx = px - vx;
    dy = py - vy;
    dz = pz - vz;

    hyp = dx * dx + dz * dz;	/* hyp squared	*/
    hyp1 = sqrt (dy*dy + hyp);
    hyp = sqrt (hyp);		/* the real hyp	*/

    if (hyp1 != 0.0) {		/* rotate X	*/
	sine = -dy / hyp1;
	cosine = hyp / hyp1;
    } else {
	sine = 0.0;
	cosine = 1.0;
    }
    ADDADDR(i_rotatex);
    ADDFLOAT(cosine);
    ADDFLOAT(sine);
    ADDFLOAT(-sine);
    ADDFLOAT(cosine);

    if (hyp != 0.0) {		/* rotate Y	*/
	sine = dx / hyp;
	cosine = -dz / hyp;
    } else {
	sine = 0.0;
	cosine = 1.0;
    }

    ADDADDR(i_rotatey);
    ADDFLOAT(cosine);
    ADDFLOAT(-sine);
    ADDFLOAT(sine);
    ADDFLOAT(cosine);

    ADDADDR(i_translate);
    ADDFLOAT(-vx);
    ADDFLOAT(-vy);
    ADDFLOAT(-vz);

    ENDCOMPILE;
    endpicmandef;
}

#include "interp.h"

static bogus ()
{
    DECLARE_INTERP_REGS;

INTERP_LABEL (polarview, 48); /** POLARVIEW_SIZE == 48 */
    PC += 4;
    thread;
INTERP_LABEL (lookat, 54); /** LOOKAT_SIZE == 54 */
    PC += 7;
    thread;
}
