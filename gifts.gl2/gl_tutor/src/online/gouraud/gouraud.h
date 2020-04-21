#include "gl.h"
#include "stdio.h"
#include "device.h"
#include "mymath.h"

#define I	2		    /* For accessing color index in array. */
#define UNITSIZE	32	    /* Distance between points on the grid. */

/*---------------------------------------------------------------------------
 * Tokens returned from check_for_hit.
 *---------------------------------------------------------------------------
 */
#define GRID		1	    /* A hit on the grid has occured. */
#define CONSOLE		2	    /* A hit on the cons has occured. */

/*---------------------------------------------------------------------------
 * Tolken for not attached in cursor location. 
 *---------------------------------------------------------------------------
 */
#define NOTATT		    -2

/*---------------------------------------------------------------------------
 * Color definitions.
 *---------------------------------------------------------------------------
 */
#define GRIDCOLOR	RED	    /* Polygon grid. */
#define GRIDBACK	BLACK	    /* Grid background. */
#define EDGECOLOR	GREEN	    /* Wire frame color. */
#define LABEL		BLACK	    /* Color of slider bar lable. */
#define HELPBACK	WHITE	    /* Color of help background. */
#define HELPTEXT	BLACK	    /* Color of help text. */
#define RAMPBACK	WHITE	    /* Background color of the ramp. */
#define RAMPHIGH	RED	    /* Color of ramp highlight. */
#define CONSBACK	RAMPBOT-1   /* Console background. */

typedef int Gridcoord;

int	current_vertex;		    /* Current vertex being edited. */
long ocx, ocy;			    /* Origin of the console.	    */
long scx, scy;			    /* Size of the console.	    */
long ogx, ogy;			    /* Origin of the grid.	    */
long sgx, sgy;			    /* Size of the grid.	    */
Boolean exiting,     		    /* True if leaving the program. */
        attached;		    /* True if attached in mex.	    */

Boolean moving,			    /* True if moving a vertex.     */
	selecting, 		    /* True if selecting a colorind.*/
	moved, 
	selected;

int cursloc;			    /* Current cursor location.	    */
int mainmen,			    /* ID number of the main menu.  */
    exitmen;			    /* ID of the exit menu.	    */

/*---------------------------------------------------------------------------
 * Graphics ID numbers for each of the windows.
 *---------------------------------------------------------------------------
 */
int backid,			    /* background id	*/
    consid, 			    /* Main program console. */
    gridid,			    /* Polygon grid ID. */
    helpid;			    /* Help box. */

/*---------------------------------------------------------------------------
 * Functions.
 *---------------------------------------------------------------------------
 */
Screencoord screenc();
float get_dxdy();
float get_didy();
int check_for_hit();
float get_current_pointer();
/*---------------------------------------------------------------------------
 * Arrays used for polygon.
 *---------------------------------------------------------------------------
 */
#define NUMVERTS    4		    /* Number of vertecies in the polygon. */
float polyarray[NUMVERTS][3];	    /* Array for holding data on each vertex
				       of the polygon:

					polyarray[v][0]: X coordinate of
				       			 vertex v.
					polyarray[v][1]: Y coordinate of
							 vertex v.
					polyarray[v][2]: Color index of
							 vertex v.
					*/

/*---------------------------------------------------------------------------
 * Picking buffer and number in the list.
 *---------------------------------------------------------------------------
 */
#define MAXPICK		    100
short pick_buffer[MAXPICK];	    /* Picking buffer. */
long retnumber;			    /* Number of tof names in the list. */

/*---------------------------------------------------------------------------
 * Color ramp bounds.
 *---------------------------------------------------------------------------
 */
#define RAMPTOP		63	    /* Top of the color ramp for shading.   */
#define RAMPBOT		32	    /* Bottom of the color ramp for shading.*/
#define RAMPLEN		(RAMPTOP - RAMPBOT + 1)	    /* Ramp length. */

/*---------------------------------------------------------------------------
 * Display list numbers for the grid window.
 *---------------------------------------------------------------------------
 */
#define PIXEL	    1		    /* Round pixel on the big grid. */
#define HPIXEL	    2		    /* Highlighted pixel. */
#define HIGHLIGHT   3		    /* Highlight. */
#define CONSTRAN    4		    /* Console transformation. */
#define GRIDTRAN    5		    /* Grid transformation. */


#define GRIDX		0	    /* X coordinate of the grid. */
#define GRIDY		0	    /* Y coordinate of the grid. */
#define GRIDSIZE	20	    /* Size of the polygon grid. */
#define SQRSIZE		9.34

