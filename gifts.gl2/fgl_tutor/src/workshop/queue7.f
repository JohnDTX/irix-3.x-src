C  queue7.f  -- This program receives all input in an event queue.
C		 The effect of the program is not different from the
C		 previous program, aim6.  The user chooses a spot on
C		 the field with the LEFT MOUSE button, and the ball
C		 is sent to it.
C  graphics library calls introduced:  qdevic, tie,
C					qtest, qread

#include "fgl.h"
#include "fdevice.h"

C  While there is nothing on the queue, consider the program idle
C  and just swapbuffers.  When something gets this programs attention,
C  generally a queued device, its input will be processed.


       CALL INITIALIZE
       CALL DRAWIMAGE


 100   IF (QTEST() .EQ. 0) THEN
            CALL SWAPBU
       ENDIF
       CALL PROCESSINPUT
       GOTO 100

       END

C  One line of code has been added to initialize to
C  designate the LEFTMO button as a queued device.	Then
C  the tie routine queues the MOUSEX and MOUSEY.  Now
C  when a LEFTMO device entry is put on the queue,
C  corresponding MOUSEX and MOUSEY entries are also put
C  on the queue.

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

       CALL QDEVIC(LEFTMO)
       CALL TIE (LEFTMO, MOUSEX, MOUSEY)
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

C  Input devices can make entries in the event queue.  The
C  processinput() routine now handles those events.  The
C  qtest() call reads the event queue.  qtest() immediately
C  returns the device number of the first entry of the queue.
C  If the queue is empty, qtest() returns FALSE, and
C  processinput() is exited.  qtest() does NOT remove the
C  entry from the queue.
C  Each entry into the queue has two fields:  a device name
C  and an associated data value.  qread(val) waits until
C  there is an entry on the queue, then it reads that entry.
C  qread(val) returns the name of the device and also writes
C  the associated data value into the variable, val.  The
C  entry on the queue is REMOVED.
C  With many buttons and keys, the values 1 and 0 are 
C  associated with pressing down (1) and releasing (0) a key
C  or button.  With the MOUSEX and MOUSEY devices, the
C  associated value is the valuator location of the cursor
C  on the screen.

       SUBROUTINE PROCESSINPUT

#include "fgl.h"
#include "fdevice.h"

       INTEGER*2 VAL
       INTEGER*2 MX, MY
       INTEGER DEV
       LOGICAL DOMOVE

       DOMOVE = .FALSE.
 300   IF (QTEST() .EQ. 0) GOTO 350
            DEV = QREAD(VAL)
            IF (DEV .EQ. LEFTMO) THEN
C		When the mouse is pressed down, will do motion
		IF (VAL .EQ. 1) THEN
		    DOMOVE = .TRUE.
		ENDIF
	    ELSE IF (DEV .EQ. MOUSEX) THEN
		MX = VAL
	    ELSE IF (DEV .EQ. MOUSEY) THEN
		MY = VAL
	    ELSE IF (DEV .EQ. REDRAW) THEN
		CALL RESHAP
		CALL DRAWIMAGE
            ENDIF
            GOTO 300
 350   CONTINUE

       IF (DOMOVE) THEN
	    CALL MOVEBALL(MX, MY)
       ENDIF

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

C  moveball() now takes two arguments, which are the final (mx, my)
C  location of the ball on the field.

       SUBROUTINE MOVEBALL(MX, MY)

       INTEGER*2 MX, MY

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
       DISTX = MX - SCREENX
       DISTY = MY - SCREENY
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

