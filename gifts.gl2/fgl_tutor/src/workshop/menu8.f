C  menu8.f -- This program allows the user a pop-up menu	
C  interface to an application program.  In this example,	
C  the RIGHT MOUSE button controls a popup menu.  This popup	
C  menu can be used to exit the program.  Intention to exit	
C  the program must be reconfirmed by the user responding to	
C  a second popup menu.  Popup menu support is ONLY available	
C  in the window manager.					
C  graphics library calls introduced:  defpup, newpup,	
C		addtop, dopup.				

#include "fgl.h"
#include "fdevice.h"


C  new global variables for menus		

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

C  In initialize, queue the RIGHTMOUSE button for popup menus.	
C  Also the pup calls are used to create popup menus.			
	SUBROUTINE INITIALIZE

#include "fgl.h"
#include "fdevice.h"

	INTEGER DUMMY
	INTEGER  MENU, KILLMENU
	COMMON /MENB/MENU,KILLMENU

	CALL PREFSI(450, 450)
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

C  In processinput, the RIGHTM button, when it is pressed	
C  down (val .eq. 1), displays a popup menu.  The value returned	
C  by the user's selection is stored in variable menuval.  The		
C  value returned may be:						
C	 1 -- for exit program						
C	-1 -- nothing on menu is chosen					
C  Then the user is asked to reconfirm an exit program decision.	
C	 1 -- for yes							
C	 2 -- for no							
C	-1 -- nothing on menu is chosen					

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

C  moveball() has not changed since the last example.		

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

C	  clear away old ball
	     CALL COLOR(0)
	     CALL CLEAR

C	  draw ball at position (x, y)
	     CALL COLOR(16)
	     CALL CIRCFI(INEWX, INEWY, 3)

C	  swapbuffers
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

