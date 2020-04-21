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

/* File: invertview.c
 *
 * The routine in this file takes a viewing object, and generates a matrix
 * which is the inverse of the viewing transformation.  For now,
 * a viewing object is an object containing only transformations,
 * viewing, and modelling commands.  In the future, we may wish to add other
 * things.
 */

#include "gl.h"
#include "glerror.h"

void mapw(vobj, sx, sy, wx1, wy1, wz1, wx2, wy2, wz2)
Object vobj;
Screencoord sx, sy;
Coord *wx1, *wy1, *wz1, *wx2, *wy2, *wz2;
{
    Screencoord x1, y1, x2, y2;
    Coord clipx, clipy, f, f1, f2, t1, t2, t3, denom;
    Matrix mat;

    gl_invertview(vobj, mat, &x1, &x2, &y1, &y2);
    f = sx; f1 = x1 - 0.5; f2 = x2 + 0.5;
    if (f1 == f2) {
	gl_ErrorHandler(ERR_ZEROVIEWPORT, WARNING, 0);
	return;
    }
    clipx = (2.0*f - f1 - f2)/(f2 - f1);
    f = sy; f1 = y1 - 0.5; f2 = y2 + 0.5;
    if (f1 == f2) {
	gl_ErrorHandler(ERR_ZEROVIEWPORT, WARNING, 0);
	return;
    }
    clipy = (2.0*f - f1 - f2)/(f2 - f1);

    denom = clipx*mat[0][3] + clipy*mat[1][3] + mat[3][3];
    t1 = clipx*mat[0][0] + clipy*mat[1][0] + mat[3][0];
    t2 = clipx*mat[0][1] + clipy*mat[1][1] + mat[3][1];
    t3 = clipx*mat[0][2] + clipy*mat[1][2] + mat[3][2];
    *wx1 = t1/denom;
    *wy1 = t2/denom;
    *wz1 = t3/denom;

    denom += 3.14159*mat[2][3];
    *wx2 = (t1 + 3.14159*mat[2][0])/denom;
    *wy2 = (t2 + 3.14159*mat[2][1])/denom;
    *wz2 = (t3 + 3.14159*mat[2][2])/denom;
}

void mapw2(vobj, sx, sy, wx, wy)
Object vobj;
Screencoord sx, sy;
Coord *wx, *wy;
{
    Coord dummy;

    mapw(vobj, sx, sy, wx, wy, &dummy, &dummy, &dummy, &dummy);
}

gl_invertview(vobj, mat, vl, vr, vb, vt)
Object vobj;
Matrix mat;
Screencoord *vl, *vr, *vb, *vt;
{
    Matrix mtemp;		/* temp matrix			*/

    pushmatrix ();		/* save the current matrix	*/
    pushviewport ();		/* save the current viewport	*/
    callobj (vobj);		/* call the viewing object	*/
    getmatrix (mtemp);		/* get the new matrix back	*/
    gl_invertmat (mtemp,mat);	/* and invert it		*/
    getviewport (vl,vr,vb,vt);	/* get the current viewport back*/
    popviewport ();		/* pop the viewport stack	*/
    popmatrix ();		/* pop the matrix stack		*/
}

static float DET3(a, b, c, d, e, f, g, h, i)
float a, b, c, d, e, f, g, h, i;
{
    return (a)*(e*i-f*h) -(b)*(d*i-f*g) + (c)*(d*h-e*g);
}

gl_invertmat(mat, matinv)
register Matrix mat, matinv;
{
    float a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    float det, a1, a2, a3, a4;

    a = mat[0][0]; b = mat[0][1]; c = mat[0][2]; d = mat[0][3];
    e = mat[1][0]; f = mat[1][1]; g = mat[1][2]; h = mat[1][3];
    i = mat[2][0]; j = mat[2][1]; k = mat[2][2]; l = mat[2][3];
    m = mat[3][0]; n = mat[3][1]; o = mat[3][2]; p = mat[3][3];

    det = a*(a1=DET3(f, g, h, j, k, l, n, o, p));
    det -= b*(a2=DET3(e, g, h, i, k, l, m, o, p));
    det += c*(a3=DET3(e, f, h, i, j, l, m, n, p));
    det -= d*(a4=DET3(e, f, g, i, j, k, m, n, o));

    if ((det > -1.0E-30) && (det < 1.0E-30)) {
	gl_ErrorHandler(ERR_SINGMATRIX, WARNING, 0);
	return;
    }

    matinv[0][0] = (a1)/det;
    matinv[1][0] = -(a2)/det;
    matinv[2][0] = (a3)/det;
    matinv[3][0] = -(a4)/det;

    matinv[0][1] = -(DET3(b, c, d, j, k, l, n, o, p))/det;
    matinv[1][1] = (DET3(a, c, d, i, k, l, m, o, p))/det;
    matinv[2][1] = -(DET3(a, b, d, i, j, l, m, n, p))/det;
    matinv[3][1] = (DET3(a, b, c, i, j, k, m, n, o))/det;

    matinv[0][2] = (DET3(b, c, d, f, g, h, n, o, p))/det;
    matinv[1][2] = -(DET3(a, c, d, e, g, h, m, o, p))/det;
    matinv[2][2] = (DET3(a, b, d, e, f, h, m, n, p))/det;
    matinv[3][2] = -(DET3(a, b, c, e, f, g, m, n, o))/det;

    matinv[0][3] = -(DET3(b, c, d, f, g, h, j, k, l))/det;
    matinv[1][3] = (DET3(a, c, d, e, g, h, i, k, l))/det;
    matinv[2][3] = -(DET3(a, b, d, e, f, h, i, j, l))/det;
    matinv[3][3] = (DET3(a, b, c, e, f, g, i, j, k))/det;
}
