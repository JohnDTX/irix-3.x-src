#include "tutor.h"
#include "math.h"
#include "gl.h"
#include "device.h"
#include "mymath.h"

/*---------------------------------------------------------------------------
 * Color indecies.
 *---------------------------------------------------------------------------
 */
#define BACKC		BLACK	    /* Color of the general background.	*/
#define TEXTC		WHITE	    /* Color of the text.		*/
#define ARROWC		WHITE	    /* Color of the arrows.		*/
/*#define CUBEBOT		9	     Bottom of the cube color ramp.	*/
#define CUBEBOT		64	    /* Bottom of the cube color ramp.	*/
#define EXAMBC		BLACK	    /* Color of the example background.	*/
#define HELPBC		WHITE	    /* Color of the help box.		*/

#define CUBCR		255.0	    /* Brightest color of the cube.	*/
#define CUBCG		0.0	    /* Brightest color of the cube.	*/
#define CUBCB		0.0	    /* Brightest color of the cube.	*/

#define HALFTONE    1		    /* Index to pattern.		*/
#define DASHED	    1		    /* Index to linesytle.		*/

Matrix	    ident;		    /* Identity matrix.			*/

Boolean	    exiting,		    /* TRUE if exiting program. 	*/
	    spinning,		    /* TRUE if spinning the cube.	*/
	    arrows, 		    /* TRUE if showing polygon arrows.	*/
	    shading,		    /* True if shading the cube.	*/
	    attached;		    /* True if attached to the window.	*/

Angle	    curexam;		    /* Current angle of the example.	*/

int	helpw,			    /* ID of the help box.		*/
	examw,			    /* ID of the example box.		*/
	withw,			    /* ID of the with window.		*/
	withow;			    /* ID of the without window.	*/

int mainmen,			    /* ID number of the main menu.  */
    exitmen;			    /* ID of the exit menu.	    */

Pattern16   halftone;
Linestyle   dashed;

#define NUMSIDES    6		    /* Number of polygons in our solid. */
#define NUMVERTS    4		    /* Number of vertecies on each side.*/

Vector	    eye;		    /* Eye vector.			*/
Vector	    points[8];
int	    verts[NUMSIDES][NUMVERTS];
Vector	    normals[NUMSIDES];


Vector	    new_normals[NUMSIDES];  /* Rotated normals.			*/
Boolean	    visible[NUMSIDES];	    /* TRUE if side n is visible.	*/

Matrix	    current;
