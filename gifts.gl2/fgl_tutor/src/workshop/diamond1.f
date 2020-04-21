C  diamond1.f  --  This program draws a static image of a baseball	
C		    diamond.  From this simple starting block, a	
C		    more sophisticated series of programs with color,	
C		    motion, user interaction and 3-D will be built.	

C  Every graphics program which utilizes the IRIS graphics library	
C  should include the gl.h and device.h files.				

#include "fgl.h"
#include "fdevice.h"

C  For each program in the series, new graphics library routines	
C  will be used.  Each new routine will be introduced in a short	
C  header comment.  For more information on each routine, see the	
C  Reference Manual section of the User's Guide.			
C									
C  graphics library calls introduced:					
C	keepas (x, y) -- constrain any window opened in the future	
C 		to an aspect ratio of y divided by x units		
C	prefsi (x, y) -- constrain any window opened in the future to 
C              a size of x units by y units                            
C	winoped ("diamond") -- open a window, name it "diamond"		
C	color (colorname) -- change the current color to colorname	
C	clear () -- clear every pixel to the current color		
C	arcs (x, y, rad, startang, endang) -- draw an UNfilled arc	
C		with a center at (x, y), a radius of length rad.	
C		The arc begins at angle startang and ends at endang,	
C		where startang and endang are measured in tenths of 	
C		degrees, counterclockwise from the x-axis.		
C		arcs is different from arc, because arcs specifies	
C		the x, y and rad arguments to be 16-bit integers	
C		(Screen coordinates) rather than 32-bit floating point	
C		real numbers, which are used for arc.  arcf is used	
C		for filled arcs.					
C	move2s (x, y) -- move, without drawing, the current graphics	
C		position to the point (x, y).  move2 (x, y) without	
C		the s means to move to an (x, y) location specified	
C		by 32-bit real numbers.  move (x, y, z) without the 2	
C		means to move to a 3-D location.  Other calls in the	
C		move family are move2i (2-D integer), moves (3-D short	
C		integer), movei (3-D integer).				
C	draw2s (x, y) -- draws a line from the current graphics 	
C		position (specified by move) to (x, y).  The current	
C		graphics position is then moved to (x, y).  draw2s	
C		also has its sibling calls:  draw, draw2, drawi,	
C		draw2i, draws.						
C	circfs (x, y, rad) -- draw a FILLED circle with a center	
C		at (x, y) and a radius of length rad.  An unfilled	
C		(hollow) circle would be drawn with the circ, circi,	
C		and circs commands.					
C	pmv2s (x, y)							
C	pdr2s (x, y)							
C	pclos () -- The pmv, pdr and pclos are used in conjunction	
C		with one another to draw a FILLED polygon.  The 	
C		location of the first point is determined by pmv2s	
C		(polygon move).  The location of the next points	
C		are determined by a sequence of pdr2s's.  The polygon	
C		is closed by pclos, which connects the final point	
C		(last pdr) with the first point (pmv).  		
C		Also see the polf command for another method of		
C		drawing filled polygons.				
C		A concave polygon will not look right.			
C	rpdr2s (rx, ry) -- relative polygon draw			
C		rpdr is similar to pdr, except that the edge of a	
C		polygon is drawn from the current graphics position	
C		to a point a distance away.  You are no longer 		
C		specifying the point you are drawing to, but how	
C		far away to draw FROM your current position.		
C                                                                      
C  The following calls are used to handle input from the window        
C  manager and other input sources. They will be discussed in gory     
C  detail in the queue7 workshop.                                      
C      qdevic () -- Establish input device.                           
C      qtest () -- check input queue for any input.                    
C      qread () -- Read data from the input queue.                     
C  Some lines are drawn twice their previous thickness with the 	
C  linewidth() command.						

C  In the main routine of the program, initialize() is called to	
C  open a window.  Then, drawimage() draws the baseball diamond.	
C  The while(TRUE) loop keeps the window open forever.  The user	
C  can kill the program from the window manager menu.			

       CALL INITIALIZE
       CALL DRAWIMAGE

C      loop forever (killed from the window manager

 100   IF (QTEST() .NE. 0) THEN
            CALL PROCESSINPUT
       ENDIF
       GOTO 100

       END

C  Open the window.  The prefsi() call forbids changing the window
C  size from the size designated.  The "qdevic" call establishes	
C  contact with the window manager in regard to screen refreshes.

       SUBROUTINE INITIALIZE

#include "fgl.h"
#include "fdevice.h"

       INTEGER DUMMY

       CALL PREFSI(450, 450)
       DUMMY = WINOPE ('DIAMOND', 7)

       CALL QDEVIC( REDRAW )

       RETURN
       END

C  Process input from the window manager
C  while there is input on the queue
C      read input device data
C      if the window manager is asking for a redraw then
C          reshape the window
C          and redraw the image.
C      eventually look for other things on the queue here
C  return

       SUBROUTINE PROCESSINPUT

#include "fgl.h"
#include "fdevice.h"

       INTEGER*2 VAL
       INTEGER DEV

 300   IF (QTEST() .EQ. 0) GOTO 350
            DEV = QREAD(VAL)
	    IF (DEV .EQ. REDRAW) THEN
		CALL RESHAP
		CALL DRAWIMAGE
            ENDIF
            GOTO 300
 350   CONTINUE

       RETURN
       END

C  clear the window to BLACK.  Draw the baseball diamond.

       SUBROUTINE DRAWIMAGE

#include "fgl.h"
#include "fdevice.h"

       CALL COLOR (0)
       CALL CLEAR
       CALL DIAMOND

       RETURN
       END

C  Draw the baseball field in yellow.  Use arcs, circles, lines
C  and polygons.  Note that we are drawing to a 2-D screen and all
C  values are integers, so far.

       SUBROUTINE DIAMOND

#include "fgl.h"
#include "fdevice.h"

C   color yellow
       CALL COLOR(3)

C   change thickness of lines
       CALL LINEWI(2)

C   fences
       CALL ARCI(0, 0, 375, 0, 900)
       CALL ARCI(0, 0, 150, 0, 900)

C   foul lines
       CALL MOVE2I(0, 0)
       CALL DRAW2I(0, 400)
       CALL MOVE2I(0, 0)
       CALL DRAW2I(400, 0)

C   restore thickness of lines
       CALL LINEWI(1)

C   pitcher's mound
       CALL CIRCFI(43, 43, 10)

C   first, second and third base
       CALL DRAWBASE(90, 0)
       CALL DRAWBASE(90, 90)
       CALL DRAWBASE(0, 90)

C   draw home plate
       CALL PMV2I(0, 0)
       CALL PDR2I(0, 3)
       CALL PDR2I(3, 6)
       CALL PDR2I(6, 3)
       CALL PDR2I(3, 0)
       CALL PCLOS

       RETURN
       END

C  Draw a base.  Note the use of relative draws to create the base.
C  If absolute draws were used, the code would look like this:
C	call pmv2i(x, y)
C	call pdr2i(x + 5, y)
C	call pdr2i(x + 5, y + 5)
C	call pdr2i(x, y + 5)
C	call pclos

       SUBROUTINE DRAWBASE(X, Y)

       INTEGER*2 X,Y

#include "fgl.h"
#include "fdevice.h"

       CALL PMV2I(X, Y)
       CALL RPDR2I(5, 0)
       CALL RPDR2I(0, 5)
       CALL RPDR2I(-5, 0)
       CALL PCLOS

       RETURN
       END
