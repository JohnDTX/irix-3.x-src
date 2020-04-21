#include "gouraud.h"

#define GRIDSIZE	20	    /* Size of the polygon grid. */
#define GRIDX		0	    /* X coordinate of the grid. */
#define GRIDY		0	    /* Y coordinate of the grid. */
#define TOP		0
#define BOT		1
#define LEFT		0
#define RIGHT		1

int ed[NUMVERTS][2];		    /* Current edges.		 */
int ned;			    /* Number of current edges.	 */
int ae[2][2];			    /* Current active edges.	 */
