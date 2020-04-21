#include "mymath.h"
#include "tutor.h"
#include <math.h>
#include <gl.h>
#include <device.h>

/*---------------------------------------------------------------------------
 * Color indecies.
 *---------------------------------------------------------------------------
 */

#define BACKC		BLACK	    /* Color of the general background.	*/
#define TEXTC		WHITE	    /* Color of the text.		*/
#define ARROWC		WHITE	    /* Color of the arrows.		*/
#define CUBEBOT		64	    /* Bottom of the cube color ramp.	*/
#define EXAMBC		BLACK	    /* Color of the example background.	*/
#define HELPBC		CUBEBOT+7   /* Color of the help box.		*/

#define CUBCR		255.0	    /* Brightest color of the cube.	*/
#define CUBCG		0.0	    /* Brightest color of the cube.	*/
#define CUBCB		0.0	    /* Brightest color of the cube.	*/

#define NUMSIDES    6		    /* Number of polygons in our solid. */
#define NUMVERTS    4		    /* Number of vertecies on each side.*/

#define HALFTONE    1		    /* Index to pattern.		*/
#define DASHED	    1		    /* Index to linesytle.		*/

Boolean	    exiting,		    /* TRUE if exiting program. 	*/
	    spinning,		    /* TRUE if spinning the cube.	*/
	    arrows, 		    /* TRUE if showing polygon arrows.	*/
	    shading,		    /* True if shading the cube.	*/
	    attached;		    /* True if attached to the window.	*/

Angle	    curexam = 0;	    /* Current angle of the example.	*/

int	backw,			    /* ID of the background		*/
	helpw,			    /* ID of the help box.		*/
	examw,			    /* ID of the example box.		*/
	withw,			    /* ID of the with window.		*/
	withow;			    /* ID of the without window.	*/

int mainmen,			    /* ID number of the main menu.  */
    exitmen;			    /* ID of the exit menu.	    */

Pattern16 halftone = {	0x5555, 0xaaaa, 0x5555, 0xaaaa, 
			0x5555, 0xaaaa, 0x5555, 0xaaaa, 
			0x5555, 0xaaaa, 0x5555, 0xaaaa, 
			0x5555, 0xaaaa, 0x5555, 0xaaaa};

Linestyle   dashed = 0xff00;

Matrix ident = {1.0, 0.0, 0.0, 0.0, 
		0.0, 1.0, 0.0, 0.0, 
		0.0, 0.0, 1.0, 0.0, 
		0.0, 0.0, 0.0, 1.0};

Vector	    points[8] = {   0.5, 0.5, 0.5, 
			    0.5, 0.5, -0.5, 
			    0.5, -0.5, -0.5, 
			    0.5, -0.5, 0.5, 
			    -0.5, -0.5, 0.5, 
			    -0.5, 0.5, 0.5, 
			    -0.5, 0.5, -0.5, 
			    -0.5, -0.5, -0.5 };

int	    verts[NUMSIDES][NUMVERTS] = {   0, 5, 4, 3, 
					    7, 6, 1, 2, 
					    0, 1, 6, 5, 
					    4, 7, 2, 3, 
					    0, 3, 2, 1, 
					    4, 5, 6, 7};
					    
Vector	    normals[NUMSIDES] = {   0.0, 0.0, 1.0, 
				    0.0, 0.0, -1.0, 
				    0.0, 1.0, 0.0, 
				    0.0, -1.0, 0.0, 
				    1.0, 0.0, 0.0, 
				    -1.0, 0.0, 0.0};

Vector	    eye = {0.0, 0.0, 1.0};  /* Eye vector.			*/

Vector	    new_normals[NUMSIDES];  /* Rotated normals.			*/
Boolean	    visible[NUMSIDES];	    /* TRUE if side n is visible.	*/

Matrix	    current;

RGBvalue    curmap[NUMSIDES][3];
