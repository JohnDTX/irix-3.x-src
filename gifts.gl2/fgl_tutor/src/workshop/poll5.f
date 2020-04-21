C  poll5.f  -- This program polls user input from the mouse.
C		The motion of the ball is initiated by the user.
C		When the LEFT mouse button is pressed, the motion
C		begins.
C  graphics library calls introduced:  getbut

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

C  processinput is altered to poll the mouse button

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


C  The former drawimage() routine is now split into two
C  routines.  drawimage() draws just the baseball field.
C  Also see moveball() below.

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

C  The second part of the former drawim routine is now
C  moveball().  In moveball(), the ball is sent into across
C  the surface of the playing field until it settles into
C  a final resting place.  Call moveball() again and again
C  restarts the motion sequence.

       SUBROUTINE MOVEBALL

#include "fgl.h"
#include "fdevice.h"

       INTEGER I

C   protect the first four bitplanes
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

