C  parabola14.f -- parabola14 is similar to the previous workshop	
C  except that the ball leaves the surface of the playing field.	
C  The ball travels in a parabola from home plate to the selected	
C  point on the field.  This really gives the illusion of 3D.		
C  No new graphics commands introduced here.				
C  parabola is a completely new procedure.				

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
C		First View

		CALL DRAWIMAGE
		CALL MOVEBALL(MX, MY)

C		Freeze ball

		CALL WAITANDSWAP

C  		instant replay	

		CALL OUTFIELDIMAGE
		CALL MOVEBALL(MX, MY)

C  		freeze ball 	

		CALL WAITANDSWAP
		CALL DRAWIMAGE
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

C  outfieldimage has not changed since the last example.

	SUBROUTINE OUTFIELDIMAGE

#include "fgl.h"
#include "fdevice.h"

    	CALL PERSPE(1350, 1.0, 0.1, 10000.0)
	CALL POLARV(400.0, 1350, 800, 0)
C  	draw the field twice, once to each buffer	
    	CALL FRONTB(.TRUE.)
    	CALL COLOR(BLACK)
    	CALL CLEAR
    	CALL DIAMOND
    	CALL FRONTB(.FALSE.)

	RETURN
	END

C  waitandswap has not changed since the last example.		

	SUBROUTINE WAITANDSWAP  
#include "fgl.h"
#include "fdevice.h"

	INTEGER I

    	DO 900 I=0,61
		CALL SWAPBU
 900		CALL GSYNC

	RETURN
	END


C  moveball has been modified.  The initial position of the	
C  ball (0.0, 0.0), and the relative distance the ball will	
C  travel.  I say "relative" because the distance depends on	
C  how large a window is made.  The parabola routine is	
C  called to draw the moving ball.				

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

	CALL GETORI(SCREENX, SCREENY)
    	CALL GETSIZ(SIZEX, SIZEY)
	DISTX = MX - SCREENX
	DISTY = MY - SCREENY

C 	This stuff calculates the relative position of the ball in the   
C	window as defined by the window size and projection.             
    	RDISTX =  DISTX * 425.0 / SIZEX
    	RDISTY =  DISTY * 425.0 / SIZEY

	CALL PARABOLA(0.0, 0.0, RDISTX, RDISTY, 100.0, 100)

	RETURN
	END

C  Given initial conditions, the parabola routine draws	
C  the moving ball.  (xstart, ystart) are the coordinates	
C  of the starting position of the parabola, assuming z = 0.	
C  (xdone, ydone) is where the ball will meet the ground 	
C  again.  At the apex, the parabola reaches a height of	
C  zmax units.  The position of the ball will be calculated	
C  the number of times specified by the iterates variable.	

	SUBROUTINE PARABOLA(XSTART, YSTART, XDONE, YDONE, ZMAX, ITERATES)
	REAL XSTART, YSTART, XDONE, YDONE, ZMAX
	INTEGER ITERATES

#include "fgl.h"
#include "fdevice.h"

    	REAL T, X, Y, Z
    	REAL TINCR

C  	protect the first four bitplanes		
    	CALL WRITEM(16)
    	TINCR = 1.0 / ITERATES

    	DO 900 T = 0.0, 1.0, TINCR
		CALL COLOR(BLACK)
		CALL CLEAR
		CALL COLOR (16)
C 		velocity constant in x direction   
		X = (XDONE - XSTART) * T
C 		velocity constant in y direction   
		Y = (YDONE - YSTART) * T
C 		formula for z motion is a parabola 
		Z = 4 * ZMAX * (T * (1 - T))
		CALL PUSHMA
		CALL TRANSL(X, Y, Z)
		CALL DRAWBALL
		CALL POPMAT 
 900		CALL SWAPBU

C  	Draw the ball into both front and back buffers.	
C	draw final resting place of ball to		
    	CALL FRONTB (.TRUE.)
C  	both buffers				
    	CALL COLOR (BLACK)
    	CALL CLEAR 
    	CALL COLOR (16)

    	CALL PUSHMA
    	CALL TRANSL (X, Y, Z)
C 	draw ball at new position          
    	CALL DRAWBALL
    	CALL POPMAT
    	CALL FRONTB(.FALSE.)
C  	unprotect all bitplanes		
    	CALL WRITEM($FFF)

	RETURN
	END

C  drawball has not changed since the last example.		

	SUBROUTINE DRAWBALL 
#include "fgl.h"
#include "fdevice.h"

    	CALL CIRC (0.0, 0.0, 3.0)
    	CALL ROTATE (900, 'Y')
    	CALL CIRC (0.0, 0.0, 3.0)
    	CALL ROTATE (600, 'X')
    	CALL CIRC (0.0, 0.0, 3.0)
    	CALL ROTATE (600, 'X')
    	CALL CIRC (0.0, 0.0, 3.0)

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

