C -------------------------- PROGRAM RAMP ---------------------------
C This program creates a color ramp of 240 shades of red
C and displays them with their color indices written in white.
C -------------------------------------------------------------------

$INCLUDE /usr/include/fgl.h
$INCLUDE /usr/include/fdevice.h

C -------------------------------------------------------------------
C Set up constants for the base and end color index, the number of
C boxes in the x direction, the number of boxes in the y direction,
C and the size (in pixels) of each box.
C -------------------------------------------------------------------
       INTEGER BASEC, ENDC, NXBOX, NYBOX, SIZE
       COMMON /GLBLS/BASEC, ENDC, NXBOX, NYBOX, SIZE

       BASEC = 8
       ENDC = 248
       NXBOX = 24
       NYBOX = 10
       SIZE = XMAXSC/NXBOX

       CALL INITIALIZE
 100   CALL DRAWIMAGE
       CALL CHECKINPUT
       GOTO 100

       END


       SUBROUTINE INITIALIZE


$INCLUDE /usr/include/fgl.h
$INCLUDE /usr/include/fdevice.h

       INTEGER BASEC, ENDC, NXBOX, NYBOX, SIZE
       COMMON /GLBLS/BASEC, ENDC, NXBOX, NYBOX, SIZE

C -------------------------------------------------------------------
C      Declare i as an integer variable.
C -------------------------------------------------------------------
      INTEGER I
      INTEGER WINID

       CALL PREFPO(0, 1023, 0, 767)
       WINID = WINOPE('ramp.f',6)

C -------------------------------------------------------------------
C Load color map locations 8-247 with RGB values of increasing
C red component.
C -------------------------------------------------------------------
       DO 200 I = BASEC, ENDC-1, 1
 200   CALL MAPCOL(I, I, 0, 0)

       RETURN
       END


       SUBROUTINE DRAWIMAGE

$INCLUDE /usr/include/fgl.h
$INCLUDE /usr/include/fdevice.h

       INTEGER BASEC, ENDC, NXBOX, NYBOX, SIZE
       COMMON /GLBLS/BASEC, ENDC, NXBOX, NYBOX, SIZE

       INTEGER I,J

C -------------------------------------------------------------------
C buf temporarily holds the value of the color index variable so
C charst can print it.
C -------------------------------------------------------------------
       CHARACTER*100  BUF

       CALL COLOR(BLACK)
       CALL CLEAR

       DO 400 I = 0, NYBOX-1, 1
           DO 300 J = 0, NXBOX-1, 1
		CALL COLOR(I*NXBOX + J + BASEC)
		CALL RECTFI(J*SIZE, I*SIZE, (J+1)*SIZE, (I+1)*SIZE)
		CALL COLOR(WHITE)
		CALL CMOV2I(5 + J*SIZE, 5 + I*SIZE)
		WRITE(BUF, '(I3)') I * NXBOX + J + BASEC
		CALL CHARST(BUF, 3)
 300       CONTINUE
 400   CONTINUE

       RETURN
       END


       SUBROUTINE CHECKINPUT


$INCLUDE /usr/include/fgl.h
$INCLUDE /usr/include/fdevice.h

       INTEGER BASEC, ENDC, NXBOX, NYBOX, SIZE
       COMMON /GLBLS/BASEC, ENDC, NXBOX, NYBOX, SIZE

       INTEGER*2 VAL

       IF (QREAD(VAL) .EQ. REDRAW) THEN
           CALL RESHAP
       ENDIF

       RETURN
       END
