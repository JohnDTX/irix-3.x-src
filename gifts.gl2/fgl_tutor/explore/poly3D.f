C ------------ PROGRAM POLY3D ---------------

#include "fgl.h"
#include "fdevice.h"


	CALL INITIALIZE
	CALL DRAWIMAGE
 100	IF (QTEST() .NE. 0) THEN
		CALL DRAWIMAGE
	ENDIF
	CALL PROCESSINPUT
	GOTO 100

	END


	SUBROUTINE INITIALIZE

#include "fgl.h"
#include "fdevice.h"

	INTEGER WINID
	CALL KEEPAS(1, 1)	
	WINID = WINOPE('POLY3D', 6)

	CALL DOUBLE
	CALL GCONFI
	CALL FRONTB(.TRUE.)
	CALL COLOR(0)
	CALL CLEAR
	CALL FRONTB(.FALSE.)

	CALL ORTHO(-100.0, 100.0, -100.0, 100.0, -100.0, 100.0)
	
	RETURN
	END

	
	SUBROUTINE PROCESSINPUT

#include "fgl.h"
#include "fdevice.h"

	INTEGER*2 VAL

	IF (QREAD(VAL) .EQ. REDRAW) THEN
		CALL RESHAP
		CALL DRAWIMAGE
	ENDIF

 	RETURN
	END


	SUBROUTINE DRAWIMAGE

#include "fgl.h"
#include "fdevice.h"

	CALL COLOR(0)
	CALL CLEAR

	CALL COLOR (RED)
	CALL DRAWPOLY

	CALL SWAPBU

	RETURN
	END


	SUBROUTINE DRAWPOLY

#include "fgl.h"
#include "fdevice.h"

C -------------------------------------------
C Set up a 3D array.
C -------------------------------------------
	REAL PARRAY(3,4)

C -------------------------------------------
C Build the back face of the cube.
C -------------------------------------------
	PARRAY(1,1) = -10.0
	PARRAY(2,1) = -10.0
	PARRAY(3,1) = -10.0
	PARRAY(1,2) = -10.0
	PARRAY(2,2) = 10.0
	PARRAY(3,2) = -10.0
	PARRAY(1,3) = 10.0
	PARRAY(2,3) = 10.0
	PARRAY(3,3) = -10.0
	PARRAY(1,4) = 10.0
	PARRAY(2,4) = -10.0
	PARRAY(3,4) = -10.0

C -------------------------------------------
C Draw the back face.
C -------------------------------------------
	CALL POLF(4, PARRAY)

C -------------------------------------------
C Build the bottom face.
C -------------------------------------------

	PARRAY(1,1) = -10.0
	PARRAY(2,1) = -10.0
	PARRAY(3,1) = -10.0
	PARRAY(1,2) = 10.0
	PARRAY(2,2) = -10.0
	PARRAY(3,2) = -10.0
	PARRAY(1,3) = 10.0
	PARRAY(2,3) = -10.0
	PARRAY(3,3) = 10.0
	PARRAY(1,4) = -10.0
	PARRAY(2,4) = -10.0
	PARRAY(3,4) = 10.0
C -------------------------------------------
C Draw the bottom face.
C -------------------------------------------
	CALL POLF(4, PARRAY)

C -------------------------------------------
C Build and draw the front face
C -------------------------------------------
	PARRAY(1,1) = -10.0
	PARRAY(2,1) = 10.0
	PARRAY(3,1) = 10.0
	PARRAY(1,2) = -10.0
	PARRAY(2,2) = -10.0
	PARRAY(3,2) = 10.0
	PARRAY(1,3) = 10.0
	PARRAY(2,3) = -10.0
	PARRAY(3,3) = 10.0
	PARRAY(1,4) = 10.0
	PARRAY(2,4) = 10.0
	PARRAY(3,4) = 10.0

	CALL POLF(4, PARRAY)
	
C -------------------------------------------
C Build and draw the top face
C -------------------------------------------
	PARRAY(1,1) = -10.0
	PARRAY(2,1) = 10.0
	PARRAY(3,1) = 10.0
	PARRAY(1,2) = 10.0
	PARRAY(2,2) = 10.0
	PARRAY(3,2) = 10.0
	PARRAY(1,3) = 10.0
	PARRAY(2,3) = 10.0
	PARRAY(3,3) = -10.0
	PARRAY(1,4) = -10.0
	PARRAY(2,4) = 10.0
	PARRAY(3,4) = -10.0

	CALL POLF(4, PARRAY)

C -------------------------------------------
C Build and draw the left face
C -------------------------------------------
	PARRAY(1,1) = -10.0
	PARRAY(2,1) = 10.0
	PARRAY(3,1) = -10.0
	PARRAY(1,2) = -10.0
	PARRAY(2,2) = -10.0
	PARRAY(3,2) = -10.0
	PARRAY(1,3) = -10.0
	PARRAY(2,3) = -10.0
	PARRAY(3,3) = 10.0
	PARRAY(1,4) = -10.0
	PARRAY(2,4) = 10.0
	PARRAY(3,4) = 10.0

	CALL POLF(4, PARRAY)

C -------------------------------------------
C Build and draw the right face
C -------------------------------------------
	PARRAY(1,1) = 10.0
	PARRAY(2,1) = 10.0
	PARRAY(3,1) = 10.0
	PARRAY(1,2) = 10.0
	PARRAY(2,2) = -10.0
	PARRAY(3,2) = 10.0
	PARRAY(1,3) = 10.0
	PARRAY(2,3) = -10.0
	PARRAY(3,3) = -10.0
	PARRAY(1,4) = 10.0
	PARRAY(2,4) = 10.0
	PARRAY(3,4) = -10.0

	CALL POLF(4, PARRAY)
	
	RETURN
	END
