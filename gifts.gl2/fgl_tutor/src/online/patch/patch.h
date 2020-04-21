#include <math.h>
#include "mymath.h"
#include "tutor.h"
#include <gl.h>
#include <device.h>

typedef float single;
#define float double

float get_geomx();
float get_geomy();
float get_geomz();

#define NUMVIEWS	3	    /* Number of different views.	*/
#define DOWNZ		0	    /* Token for view down z-axis.	*/
#define DOWNY		1	    /* Token for view down y-axis.	*/
#define DOWNX		2	    /* Token for view down x-axis.	*/

#define BEZIER		1	    /* ID number for Bezier basis.	*/
#define CARDINAL	2	    /* ID number for Cardinal basis.	*/
#define BSPLINE		3	    /* ID number for B-Spline basis.	*/

#define NUMPNTS		16	    /* Number of control points.	*/
#define NONE		-1	    /* Token for no editing.		*/
#define PRECISION	NUMPNTS	    /* Token for precision editing.	*/
#define NUMBER		NUMPNTS+1   /* Token for the numbers editinng.	*/

#define TOP		0	    /* Top slider ID.			*/
#define MIDDLE		1	    /* Middle slider ID.		*/
#define BOTTOM		2	    /* Bottom slider ID.		*/

#define CONTROL		0	    /* Token for the curve.		*/
#define SLIDERS		1	    /* Token for the slider bars.	*/

#define MINC		-2.0	    /* Minimum coordinate.		*/
#define MAXC		2.0	    /* Maximum coordinate.		*/
#define DELTAC		(MAXC - MINC)

#define MAXPRE		25.0	    /* Maximum curve precision.		*/
#define MINPRE		1.0	    /* Minimum curve precision.		*/
#define DELTAP		(MAXPRE - MINPRE)

#define MAXN		10.0	    /* Maximum number of curves.	*/
#define MINN		1.0	    /* Minimum number of curves.	*/
#define DELTAN		(MAXN - MINN)

#define CTOS(C)		(299.0*C/DELTAC + 149.5)
#define STOC(S)		((S-149.5)*DELTAC/299.0)

#define PTOS(P)		(299.0*(P - MINPRE)/DELTAP)
#define STOP(S)		(DELTAP * S/299.0 + MINPRE)

#define NTOS(N)		(299.0*(N - MINN)/DELTAN)
#define STON(S)		(S*DELTAN/299.0 + MINN)

#define ROW(N)		((int)((float) N / 4.0))
#define COL(N)		(N % 4)

#define SLIDERTRANS	1
#define SURFACE		2

char	    helpmess[6][56];	    /* Help message.			*/

long	    curbasisu;		    /* Current curve basis.		*/
	    curbasisv;		    /* Current curve basis.		*/

long	    curcursu,		    /* Number of curves along U.	*/
	    curcursv;		    /* Number of curves along V.	*/

short	    curprecu;		    /* Current curve precision.		*/
	    curprecv;		    /* Current curve precision.		*/

Matrix	    geomx,		    /* X coordinate of control points.	*/
	    geomy, 		    /* Y coordinates of control points.	*/
	    geomz;		    /* Z coordinates of control points.	*/

int	    backw,		    /* background id			*/
	    helpw,		    /* Window for the help box.		*/
	    consw,		    /* Console window.			*/
	    statw,		    /* Status window.			*/
	    frontxw, 		    /* Window with the curves in it.	*/
	    frontyw,		    
	    frontzw;

int	    curcont;		    /* Number of the current control.	*/

char	    basis_names[3][20];

int	    curslider;		    /* Current slider being moved.	*/

Boolean	    exiting,		    /* TRUE if exiting program. 	*/
	    moving,		    /* TRUE if moving a control point.	*/
	    attached;		    /* True if attached to the window.	*/

int	    exitmen,		    /* Exit menu id number.		*/
	    basisumen,		    /* Basis menu for U.		*/
	    basisvmen,		    /* Basis menu for V.		*/
    	    mainmen;		    /* Main menu ud number.		*/

int	    numslides;		    /* Number of sliders.		*/

Matrix	    downz,		    /* Matrix for view down z-axis.	*/
	    downy,		    /* Matrix for view down y-axis.	*/
	    downx, 		    /* Matrix for view down z axis.	*/
	    state;		    /* Matrix for the state box.	*/


#define MAXPICK		100	    /* Maximum names in the pick buffer.*/
short	pick_buffer[MAXPICK];	    /* Pick buffer.			*/
long retnumber;			    /* Number returned from pickbuffer.	*/
