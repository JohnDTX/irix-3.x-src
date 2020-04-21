C  aim6.f  --  This program polls user input from the mouse.
C		The trajectory of the ball is determined from the
C		lower left hand corner of the field to a spot chosen
C		by the user.  The spot is chosen by pressing the
C		LEFT mouse button at some position.
C  graphics library calls introduced:  getori, getval

#include "fgl.h"
#include "fdevice.h"

C      The main routine has not changed since last example.

       CALL INITIALIZE
       CALL DRAWIMAGE

 100   IF (QTEST() .EQ. 0) THEN
            CALL SWAPBU
            IF (GETBUT(LEFTMO)) THEN
		CALL MOVEBALL
	    ENDIF
       ENDIF
       CALL PROCESSINPUT
       GOTO 100

       END

C   initialize has not changed since the last example.

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

C  processinput has not changed since the last example

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


C   drawimage has not changed since the last example

       SUBROUTINE DRAWIMAGE

#include "fgl.h"
#include "fdevice.h"

C   draw the field twice, once into each buffer
       CALL FRONTB(.TRUE.)
       CALL COLOR(0)
       CALL CLEAR
       CALL DIAMOND
       CALL FRONTB(.FALSE.)

       RETURN
       END

C  The user can now choose the location for the trajectory of the
C  ball.  The (x, y) location chosen is read from the valuators
C  MOUSEX and MOUSEY.  The difference between the (x, y) location
C  chosen and the (screenx, screeny) of the lower left corner of
C  the field (home plate) is the distance that the ball will travel
C  (distx, disty).  The ball moves 50 increments towards its final
C  destination.

       SUBROUTINE MOVEBALL

#include "fgl.h"
#include "fdevice.h"

       INTEGER I

C   screenx, screeny are position of lower left corner
C   distx, disty are distance ball will travel
C   incrx, incry are amount to move ball in 1 iteration
C   location of ball in flight
C   integer location of ball in flight

       INTEGER SCREENX, SCREENY
       INTEGER DISTX, DISTY
       REAL INCRX, INCRY
       REAL NEWX, NEWY
       INTEGER INEWX, INEWY

       CALL GETORI(SCREENX, SCREENY)
       DISTX = GETVAL(MOUSEX) - SCREENX
       DISTY = GETVAL(MOUSEY) - SCREENY
       INCRX = DISTX/50.0
       INCRY = DISTY/50.0
       NEWX = 0.0
       NEWY = 0.0

C   protect the first four bitplanes
       CALL WRITEM(16)

       DO 400 I=0, 49, 1

            NEWX = NEWX + INCRX
            NEWY = NEWY + INCRY
            INEWX = NEWX
            INEWY = NEWY

C         clear away old ball
            CALL COLOR(0)
            CALL CLEAR

C         draw ball at position (x, y)
            CALL COLOR(16)
            CALL CIRCFI(INEWX, INEWY, 3)

C         swapbuffers
            CALL SWAPBU

 400   CONTINUE

C   Draw the ball into both front and back buffers.
       CALL FRONTB(.TRUE.)
       CALL COLOR(0)
       CALL CLEAR
       CALL COLOR(16)
       CALL CIRCFI(INEWX, INEWY, 3)
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

