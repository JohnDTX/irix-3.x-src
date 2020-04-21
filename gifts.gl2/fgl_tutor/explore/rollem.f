C ------------- PROGRAM ROLLEM --------------
			

$INCLUDE /usr/include/fgl.h
$INCLUDE /usr/include/fdevice.h

	CALL INITIALIZE
	CALL DRAWIMAGE
 100	IF (QTEST() .EQ. 0) THEN
		CALL DRAWIMAGE
	ELSE
		CALL PROCESSINPUT
	ENDIF
	GOTO 100

	END


	SUBROUTINE INITIALIZE

$INCLUDE /usr/include/fgl.h
$INCLUDE /usr/include/fdevice.h

	INTEGER WINID
	CALL KEEPAS(1, 1)
	WINID = WINOPE('ROLLEM', 6)

	CALL DOUBLE
	CALL GCONFI
	CALL FRONTB(.TRUE.)
	CALL COLOR(0)
	CALL CLEAR
	CALL FRONTB(.FALSE.)

	CALL ORTHO(-400.0, 400.0, -400.0, 400.0, -400.0, 400.0)
	CALL TRANSL(0.0, 0.0, -300.0)

	RETURN
	END


	SUBROUTINE PROCESSINPUT

$INCLUDE /usr/include/fgl.h
$INCLUDE /usr/include/fdevice.h

	INTEGER*2 VAL

	IF (QREAD(VAL) .EQ. REDRAW) THEN
		CALL RESHAP
		CALL DRAWIMAGE
	ENDIF

	RETURN
	END


	SUBROUTINE DRAWIMAGE

$INCLUDE /usr/include/fgl.h
$INCLUDE /usr/include/fdevice.h

	CALL COLOR(0)
	CALL CLEAR

	CALL COLOR(RED)
	CALL ROLLEM

	CALL SWAPBU

	RETURN
	END


	SUBROUTINE ROLLEM

$INCLUDE /usr/include/fgl.h
$INCLUDE /usr/include/fdevice.h

	CALL ROTATE(150, 'X')
	CALL SCALE(1.05, 1.05, 1.05)
	CALL TRANSL(0.0, 0.0, 1.0)

	CALL DRAWSPHERE

	RETURN
	END

	SUBROUTINE DRAWSPHERE

$INCLUDE /usr/include/fgl.h
$INCLUDE /usr/include/fdevice.h

	INTEGER I

C -------------------------------------------
C Save the current matrix for future use. 
C -------------------------------------------
	CALL PUSHMA

C -------------------------------------------
C Rotate the circle 15 degrees about its y
C axis, then draw it.  Do this 24 times.
C This creates the "lines of longitude".
C -------------------------------------------
	DO 200, I = 0,24
		CALL ROTATE(150, 'Y')
		CALL CIRC(0.0, 0.0, 20.0)
 200	CONTINUE

C -------------------------------------------
C Push this matrix and undo the rotation.
C -------------------------------------------
	CALL PUSHMA
	CALL ROTATE(-150, 'Y')

C -------------------------------------------
C Create the "lines of latitude".
C -------------------------------------------
	DO 300, I = 0,24
		CALL  ROTATE(150, 'X')
 		CALL CIRC(0.0, 0.0, 20.0)
 300	CONTINUE

C -------------------------------------------
C Delete the second rotation matrix, then
C delete the first rotation matrix.  Now the
C original matrix is current again.
C -------------------------------------------
	CALL POPMAT
	
	CALL POPMAT

	RETURN
	END
