C  color2.f  -- This program draws a static image of a baseball
C		 diamond with several colors.  The field is covered
C		 with grass.
C  graphics library calls introduced:  mapcol(index, r, g, b)

#include "fgl.h"
#include "fdevice.h"

C  The main routine has not changed since the last example.

       CALL INITIALIZE
       CALL DRAWIMAGE

C      loop forever (killed from the window manager

 100   IF (QTEST() .NE. 0) THEN
            CALL PROCESSINPUT
       ENDIF
       GOTO 100

       END

C  Initialization now includes defining colors 8 and 9.

       SUBROUTINE INITIALIZE

#include "fgl.h"
#include "fdevice.h"

       INTEGER DUMMY

       CALL PREFSI(450, 450)
       DUMMY = WINOPE ('DIAMOND', 7)

       CALL QDEVIC( REDRAW )

C   make color of fences, lines
       CALL MAPCOL (8, 240, 240, 240)

C   make color of grass
       CALL MAPCOL (9, 0, 175, 0)

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

C  drawimage() has not changed since the last example.			*/

       SUBROUTINE DRAWIMAGE

#include "fgl.h"
#include "fdevice.h"

       CALL COLOR (0)
       CALL CLEAR
       CALL DIAMOND

       RETURN
       END

C  Declare color 9 to be the current color.  Then draw a filled arc
C  (in color 9) for the grass of the entire field.  Draw remaining
C  field features with color 8.

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

