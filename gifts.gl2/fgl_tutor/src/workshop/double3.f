C  double3.f  --  This program uses double buffering to draw a
C		   dynamic image of a ball traveling over a field
C		   without flashing.  The motion is accomplished by
C		   redrawing the scene to the back buffer.  When an
C		   image is completed, the entire buffer is swapped
C		   to the fore, and its image is displayed.
C  calls introduced:  double, gconfi, swapbu,
C		       frontb

C  WARNING:  For machines with a minimum configuration of bitplanes,
C  there may not be enough bitplanes to run all the colors in this
C  and subsequent double buffered programs.  A minimum of 12
C  bitplanes are required for this program.  For subsequent programs,
C  a minimum of 16 bitplanes will be required.

#include "fgl.h"
#include "fdevice.h"

C  In the window manager, double buffered programs MUST swapbuffers,
C  even when idling.  The window manager waits for all active double
C  double buffered programs to issue a swapbuffers() call before
C  actually swapping buffers.  If a program idles without issuing
C  a swapbuffers() call, all other double buffered programs will
C  wait for it.	

       CALL INITIALIZE
       CALL DRAWIMAGE

 100   IF (QTEST() .NE. 0) THEN
            CALL PROCESSINPUT
       ENDIF
       GOTO 100

       END

C  To initiate double buffering, the doublebuffer() call is made.
C  The display mode change to double buffering does not take place
C  until the gconfig() call is made.

       SUBROUTINE INITIALIZE

#include "fgl.h"
#include "fdevice.h"

       INTEGER DUMMY

       CALL PREFSI(450, 450)
       DUMMY = WINOPE ('DIAMOND', 7)

C   display mode to become double buffer
       CALL DOUBLE
C   display mode change takes effect
       CALL GCONFI

       CALL QDEVIC( REDRAW )

C   make color of fences, lines
       CALL MAPCOL (8, 240, 240, 240)

C   make color of grass
       CALL MAPCOL (9, 0, 175, 0)

C   make color of ball
       CALL MAPCOL (10, 240, 150, 0)

       RETURN
       END

C  Process input from the window manager

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

C  drawimage() creates motion which doesn't flash.  The for loop
C  changes the value of i, which in turn, changes the position of
C  the ball.  Each iteration through the loop, the field is redrawn
C  and the ball is redrawn in a new position.

       SUBROUTINE DRAWIMAGE

#include "fgl.h"
#include "fdevice.h"

       INTEGER I

       DO 400 I=0, 299, 3
            CALL COLOR(0)
            CALL CLEAR
            CALL DIAMOND

C   draw ball with color 10 at position (x, y)

            CALL COLOR(10)
            CALL CIRCFI(I, I, 3)

C   swapbuffers
            CALL SWAPBU

 400   CONTINUE

C  At the end of the loop, the ball will stop moving.  However,
C  the two buffers will have the ball in different positions. 
C  You want to be certain that the ball is in the same
C  position in both buffers.  Otherwise, when at rest, the
C  program will swap between the two buffers, and the ball
C  will appear to bounce back and forth between the disparate
C  positions in the two buffers.  To draw the same scene
C  into both buffers, use frontbuffer(TRUE).  Then draw	
C  the scene--that draws it into both front and back buffers.
C  frontbuffer(FALSE) restores the default state, where only
C  the back buffer is drawn into.

       CALL FRONTB(.TRUE.)
       CALL COLOR(0)
       CALL CLEAR
       CALL DIAMOND
       CALL COLOR(10)
       CALL CIRCFI(I, I, 3)
       CALL FRONTB(.FALSE.)

       RETURN
       END

C   diamond has not changed since last example.

       SUBROUTINE DIAMOND

#include "fgl.h"
#include "fdevice.h"

C   draw grass
       CALL COLOR(9)
       CALL ARCFI (0, 0, 375, 0, 900)

       CALL COLOR(8)

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

C   draw scoreboard
       CALL CMOV2I (100, 400)
       CALL CHARST ('New York 3', 10)
       CALL CMOV2I (100, 385)
       CALL CHARST ('Boston   2', 10)

       RETURN
       END

C  draw_base has not changed since the last example.

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

