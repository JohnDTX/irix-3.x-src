C  overlay4.f  -- This program uses double buffering to draw a
C		   dynamic image of a ball traveling over a field.
C		   The ability to disable the bitplanes to which the
C		   field is drawn allows the field to be drawn only
C		   once.  The ball is drawn and erased over the field
C		   without damaging the drawing of the field.
C  graphics library calls introduced:  writem

C  WARNING:  A minimum of 16 bitplanes are required to run this
C  program.


#include "fgl.h"
#include "fdevice.h"

C      The main routine has not changed since last example.

       CALL INITIALIZE
       CALL DRAWIMAGE

 100   IF (QTEST() .NE. 0) THEN
            CALL PROCESSINPUT
       ENDIF
       GOTO 100

       END

C  The entries in the color map are manipulated so that when
C  the ball appears over a section of the field, the ball
C  stays the same RGB color.  The ball will actually appear
C  as colors 16, 24 and 25 (binary 10000, 11000 and 11001),
C  on top of 0, 8, 9 (binary 00000, 01000, and 01001), which
C  are the colors for the field, fences and background.
C  When the ball (color 16) is over either field, fence or
C  black background, the writemask/overlay protects the
C  color values of the underlying object, in effect, adding
C  16 to the color value.  When the ball moves, only the
C  16 is erased.  Mapping these three colors to the same RGB
C  values makes the ball appear as one color the entire trip
C  of the ball.

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

C   make overlay (ball) for colors 16, 24, 25
       CALL MAPCOL (16, 240, 150, 0)
       CALL MAPCOL (24, 240, 150, 0)
       CALL MAPCOL (25, 240, 150, 0)

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

C  drawimage draws the ball moving across the field.  The field
C  is only drawn ONCE.  The colors of the field utilizes bitplanes
C  number 1, 2, 3 and 4.
C  Then the writemask() call protects bitplanes 1, 2, 3, 4 from being
C  overwritten any further.  The traveling ball is written to the
C  fifth bitplane (color 10000 or 16 in decimal).  For each new scene,
C  only the ball is cleared and drawn.  The field is never drawn
C  again.

       SUBROUTINE DRAWIMAGE

#include "fgl.h"
#include "fdevice.h"

       INTEGER I

C   draw the field twice, once into each buffer
       CALL FRONTB(.TRUE.)
       CALL COLOR(0)
       CALL CLEAR
       CALL DIAMOND
       CALL FRONTB(.FALSE.)
       CALL WRITEM(16)

       DO 400 I=0, 299, 3

C         clear away old ball
            CALL COLOR(0)
            CALL CLEAR

C         draw ball at position (x, y)
            CALL COLOR(16)
            CALL CIRCFI(I, I, 3)

C         swapbuffers
            CALL SWAPBU

 400   CONTINUE

C   Draw the ball into both front and back buffers.
       CALL FRONTB(.TRUE.)
       CALL COLOR(0)
       CALL CLEAR
       CALL COLOR(16)
       CALL CIRCFI(I, I, 3)
       CALL FRONTB(.FALSE.)

C   Unprotect all bitplanes.
       CALL WRITEM($FFF)

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

