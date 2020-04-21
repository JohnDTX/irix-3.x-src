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

gl_makediffmatrix(n, mat)
long	n;
float	mat[6];
{
    register float e,e2,e3;
    register float *locmat;

    locmat = (float *)mat;
    e = 1.0 / ((float) n);
    e2 = e * e;
    e3 = e2 * e;
    locmat[0] = (6.0*e3);
    locmat[1] = (6.0*e3);
    locmat[2] = (2.0*e2);
    locmat[3] = (e3); 
    locmat[4] = (e2);
    locmat[5] = (e); 
}

gl_makeconvmatrix(n, mult, convmat)
long	n;
long	mult;
float	convmat[6];
{
    register float 	fn,n2,n3;
    register float 	m,m2,m3;
    register float	*locmat;

    locmat = convmat;
    fn = (float)n;
    n2 = n * n;
    n3 = n2 * n;
    m = (float) mult;
    m2 = m * m;
    m3 = m2 * m;
    locmat[0] = (n3/m3);
    locmat[1] = ((n3/m3) - (n2 / m2));
    locmat[2] = (n2/m2); 
    locmat[3] = ((n3/(6.0*m3)) - (n2/(2.0*m2)) + (fn/(3.0*m)) );
    locmat[4] = ((n2/(2.0*m2)) - (fn/(2.0*m)) ); 
    locmat[5] = (fn/m); 
}
