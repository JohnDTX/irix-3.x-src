C -------------------- PROGRAM TEMPLATE ----------------------
C
C The first two lines attach "include files" to your program.
C "fgl.h" is a file that contains the IRIS Graphics Library
C routines, along with various useful constants. "fdevice.h"
C defines a set of common input/output devices.  You must 
C always begin your program and all subroutines with these 
C lines.  Always type the path names in lower case letters.
C ------------------------------------------------------------
$INCLUDE /usr/include/fgl.h
$INCLUDE /usr/include/fdevice.h

C ------------------------------------------------------------
C The main section of the program calls three other subroutines
C which are defined below.  It is common for the main section
C to loop infinitely.  You exit the program by killing the 
C graphics window with the right mouse button.
C ------------------------------------------------------------
	CALL INITIALIZE
	CALL DRAWIMAGE
 100	CALL CHECKINPUT
	GOTO 100

	END

C ------------------------------------------------------------
C "initialize" sets various graphics modes (which will be
C discussed later).  Right now, all it does is create a
C graphics window.  Just before the "winope" routine,
C you can set the characteristics of the window.  In the
C beginning, all of your programs will use the entire screen.
C ------------------------------------------------------------
	SUBROUTINE INITIALIZE

$INCLUDE /usr/include/fgl.h
$INCLUDE /usr/include/fdevice.h
       
	INTEGER WINID
	CALL PREFPO(0, 1023, 0, 767)
	WINID = WINOPE('NAME_OF_PROGRAM', 15)

	RETURN
	END

C ------------------------------------------------------------
C "drawimage" is the subroutine you will learn about
C in this chapter.  It contains the graphics routines
C that actually put things on the screen.
C The general form of this subroutine is simple:
C	select a color and draw something
C	select another color and draw something else
C and so on.
C Right now, this subroutine clears the screen to black.
C ------------------------------------------------------------
	SUBROUTINE DRAWIMAGE

$INCLUDE /usr/include/fgl.h
$INCLUDE /usr/include/fdevice.h

	CALL COLOR(BLACK)
	CALL CLEAR

	RETURN
	END

C ------------------------------------------------------------
C "checkinput" checks the event queue to see if anything
C has happened (e.g., has a mouse button been pushed or
C has the keyboard been touched?).  REDRAW is a special
C event that the window manager puts in the event queue
C to tell the program to redraw the graphics window
C (e.g., when another window has been moved so that this
C one is now visible).  Don't worry about how this
C subroutine works for now.
C ------------------------------------------------------------
	SUBROUTINE CHECKINPUT

$INCLUDE /usr/include/fgl.h
$INCLUDE /usr/include/fdevice.h
	
	INTEGER*2 VAL

	IF (QREAD(VAL) .EQ. REDRAW) THEN
		CALL RESHAP
	ENDIF

	RETURN
	END
