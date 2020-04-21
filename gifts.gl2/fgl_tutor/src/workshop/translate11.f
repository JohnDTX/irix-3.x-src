C  translate11.f -- translate11 demonstrates and introduces	
C  modeling transformations.  The translate command is being	
C  used to move the ball into different positions along its	
C  flight path.  For each frame, the ball is translated from	
C  the origin to its position in space.			
C  moveball is the only routine changed.			
C  graphics commands introduced:  transl

#include "fgl.h"
#include "fdevice.h"


C  global variables for menus		

	INTEGER  MENU, KILLMENU
	COMMON /MENB/MENU,KILLMENU

C  the main routine has not changed since the last example.	

	CALL INITIALIZE
	CALL DRAWIMAGE

 100   IF (QTEST() .EQ. 0) THEN
		CALL SWAPBU
	ENDIF
	CALL PROCESSINPUT
	GOTO 100

	END

C  the initialize routine has not changed since the last example.	

	SUBROUTINE INITIALIZE

#include "fgl.h"
#include "fdevice.h"

	INTEGER DUMMY
	INTEGER  MENU, KILLMENU
	COMMON /MENB/MENU,KILLMENU

	CALL KEEPAS(1, 1)
	DUMMY = WINOPE ('DIAMOND', 7)

C   display mode to become double buffer
	CALL DOUBLE
C   display mode change takes effect
	CALL GCONFI

	CALL QDEVIC(LEFTMO)
	CALL QDEVIC(RIGHTM)
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

C  Now we define two pop-up menus using newpup call, then we put in the 
C  text using an addtop all.		

C  Note: the '|' character separates menu items, and the %t flag is    
C  used to designate the menu title.					

C  newpup returns the menu id number used later.  

	KILLMENU = NEWPUP()
	CALL ADDTOP(KILLMENU, 'Do you want to exit? %t|yes|no', 30, 1)
	MENU = NEWPUP ()
	CALL ADDTOP (MENU, 'Baseball %t|exit program', 24, 1)

	RETURN
	END

C  processinput has not changed since the last example.	

	SUBROUTINE PROCESSINPUT

#include "fgl.h"
#include "fdevice.h"

	INTEGER*2 VAL
	INTEGER*2 MX, MY
	INTEGER DEV
	LOGICAL DOMOVE
	INTEGER*2 MENUVAL, KMENUVAL
	INTEGER  MENU, KILLMENU
	COMMON /MENB/MENU,KILLMENU

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
	    ELSE IF (DEV .EQ. RIGHTM) THEN
C	    When RIGHTM is pressed, do the popup.
		IF (VAL .EQ. 1) THEN
		    MENUVAL = DOPUP(MENU)
C		    If exit, then get confirmation.
		    IF (MENUVAL .EQ. 1) THEN
			KMENUVAL = DOPUP(KILLMENU)
			IF (KMENUVAL .EQ. 1) THEN
			    STOP
			ENDIF
		    ENDIF
		ENDIF
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


C  the drawimage routine has not changed since the last example.	

	SUBROUTINE DRAWIMAGE

#include "fgl.h"
#include "fdevice.h"

C   draw the field twice, once into each buffer
    	CALL ORTHO (0.0, 425.0, 0.0, 425.0, -10000.0, 10000.0)
	CALL FRONTB(.TRUE.)
	CALL COLOR(0)
	CALL CLEAR
	CALL DIAMOND
	CALL FRONTB(.FALSE.)

	RETURN
	END

C  The transl command is now used moveball to position the	
C  The ball is a circle, which would be drawn at the origin, but is	
C  then translated to different positions along its flight path.	

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
    	INTEGER SIZEX, SIZEY
	REAL RDISTX, RDISTY
	REAL INCRX, INCRY
	REAL NEWX, NEWY

	CALL GETORI(SCREENX, SCREENY)
    	CALL GETSIZ(SIZEX, SIZEY)
	DISTX = MX - SCREENX
	DISTY = MY - SCREENY
	INCRX = DISTX/50.0
	INCRY = DISTY/50.0
	NEWX = 0.0
	NEWY = 0.0

C 	This stuff calculates the relative position of the ball in the   
C	window as defined by the window size and projection.             
    	RDISTX =  DISTX * 425.0 / SIZEX
    	RDISTY =  DISTY * 425.0 / SIZEY

	INCRX = RDISTX / 50.0 
	INCRY = RDISTY / 50.0
    	NEWX = 0.0
    	NEWY = 0.0

C   protect the first four bitplanes
	CALL WRITEM(16)

	DO 400 I=0, 49, 1

	     NEWX = NEWX + INCRX
	     NEWY = NEWY + INCRY

C	  clear away old ball
	     CALL COLOR(0)
	     CALL CLEAR

C	  draw ball at position (x, y)
	     CALL COLOR(16)
	     CALL PUSHMA
	     CALL TRANSL(NEWX, NEWY, 0.0)
	     CALL CIRCF(0.0, 0.0, 3.0)
	     CALL POPMAT
C	  swapbuffers
	     CALL SWAPBU

 400   CONTINUE

C   Draw the ball into both front and back buffers.
	CALL FRONTB(.TRUE.)
	CALL COLOR(0)
	CALL CLEAR
	CALL COLOR(16)
	CALL CIRCF(NEWX, NEWY, 3.0)
	CALL FRONTB(.FALSE.)

C   Unprotect all bitplanes.
	CALL WRITEM($FFF)

	RETURN
	END

C  diamond has not changed since the last example.		

	SUBROUTINE DIAMOND

#include "fgl.h"
#include "fdevice.h"

C   draw grass
	CALL COLOR(9)
	CALL ARCF (0.0, 0.0, 375.0, 0, 900)

	CALL COLOR(8)

C   change thickness of lines
	CALL LINEWI(2)

C   fences
	CALL ARC(0.0, 0.0, 375.0, 0, 900)
	CALL ARC(0.0, 0.0, 150.0, 0, 900)

C   foul lines
	CALL MOVE2(0.0, 0.0)
	CALL DRAW2(0.0, 400.0)
	CALL MOVE2(0.0, 0.0)
	CALL DRAW2(400.0, 0.0)

C   restore thickness of lines
	CALL LINEWI(1)

C   pitcher's mound
	CALL CIRCF(43.0, 43.0, 10.0)

C   first, second and third base
	CALL DRAWBASE(90.0, 0.0, 0.0)
	CALL DRAWBASE(90.0, 90.0, 0.0)
	CALL DRAWBASE(0.0, 90.0, 0.0)

C   draw home plate
	CALL PMV2(0.0, 0.0)
	CALL PDR2(0.0, 3.0)
	CALL PDR2(3.0, 6.0)
	CALL PDR2(6.0, 3.0)
	CALL PDR2(3.0, 0.0)
	CALL PCLOS

C   draw scoreboard
	CALL CMOV2 (100.0, 400.0)
	CALL CHARST ('New York 3', 10)
	CALL CMOV2 (100.0, 385.0)
	CALL CHARST ('Boston   2', 10)

	RETURN
	END

C  drawbase has not changed since the last example.		

	SUBROUTINE DRAWBASE(X, Y, Z)

	REAL X,Y,Z

#include "fgl.h"
#include "fdevice.h"

	CALL PMV(X, Y, Z)
	CALL RPDR(5.0, 0.0, 0.0)
	CALL RPDR(0.0, 5.0, 0.0)
	CALL RPDR(-5.0, 0.0, 0.0)
	CALL PCLOS

	RETURN
	END

