#include "tutor.h"
#include <math.h>
#include "mymath.h"
#include <gl.h>
#include <device.h>

typedef float single;
#define float double

#define NUMVIEWS	3	    /* Number of different views.	*/
#define DOWNZ		0	    /* Token for view down z-axis.	*/
#define DOWNY		1	    /* Token for view down y-axis.	*/
#define DOWNX		2	    /* Token for view down x-axis.	*/

#define BEZIER		1	    /* ID number for Bezier basis.	*/
#define CARDINAL	2	    /* ID number for Cardinal basis.	*/
#define BSPLINE		3	    /* ID number for B-Spline basis.	*/

#define NUMPNTS		4	    /* Number of control points.	*/
#define PRECISION	NUMPNTS	    /* Token for precision editing.	*/
#define NONE		-1	    /* Token for no editing.		*/

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

#define CTOS(C)		(299.0*C/DELTAC + 149.5)
#define STOC(S)		((S-149.5)*DELTAC/299.0)

#define PTOS(P)		(299.0*(P - MINPRE)/DELTAP)
#define STOP(S)		(DELTAP * S/299.0 + MINPRE)

#define SLIDERTRANS	1
#define CURVE		2

char	    helpmess[6][56];	    /* Help message.			*/
int	    curbasis;		    /* Current curve basis.		*/
short	    curprec;		    /* Current curve precision.		*/
Coord	    curgeom[4][3];	    /* Current curve geometry.		*/
int	    curcont;		    /* Number of the current control.	*/

char	basis_names[3][20];

int	    curslider;		    /* Current slider being moved.	*/

Boolean	    exiting,		    /* TRUE if exiting program. 	*/
	    moving,		    /* TRUE if moving a control point.	*/
	    attached;		    /* True if attached to the window.	*/

int	    exitmen,		    /* Exit menu id number.		*/
	    basismen,		    /* Basis menu.			*/
    	    mainmen;		    /* Main menu ud number.		*/

int	    numslides;		    /* Number of sliders.		*/

Matrix	    downz,		    /* Matrix for view down z-axis.	*/
	    downy,		    /* Matrix for view down y-axis.	*/
	    downx;		    /* Matrix for view down z axis.	*/

int	    helpw,		    /* Window for the help box.		*/
	    consw,		    /* Console window.			*/
	    statw,		    /* Status window.			*/
	    frontxw, 		    /* Window with the curves in it.	*/
	    frontyw,		    
	    frontzw, 
	    backw;;

#define MAXPICK		100	    /* Maximum names in the pick buffer.*/
short	pick_buffer[MAXPICK];	    /* Pick buffer.			*/
long retnumber;			    /* Number returned from pickbuffer.	*/
