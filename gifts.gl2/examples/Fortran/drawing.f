CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C      Installation note:
C    Various Fortran compilers may require different styles of INCLUDEs.
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
$     INCLUDE /usr/include/fgl.h
C
      INTEGER CONE(2,3), I, J, STRLEN
      CHARACTER*1 ONECH, STRING*50
      DATA CONE /100,300,  150,100,  200,300/
C
      CALL GINIT()
C       set the textport to a small area in the lower right corner of
C       the screen so that when the program is finished, the textport
C       will not cover up the image
      CALL TEXTPO(650,900,50,200)
      CALL CURSOF()
C
C       draw an ice cream cone
      CALL COLOR(WHITE)
      CALL CLEAR()
C       draw the cone
      CALL COLOR(YELLOW)
      CALL POLF2I(3,CONE)
C       the first scoop is mint
      CALL COLOR(GREEN)
C       only half of it shows
      CALL ARCFI(150,300,50,0,1800)
C       the second scoop is cherry
      CALL COLOR(RED)
      CALL CIRCF(150.0,400.0,50.0)
      CALL COLOR(BLACK)
C       outline the cone in black
      CALL POLY2I(3,CONE)
C
C       next draw a few filled and unfilled arcs in the upper
C       left corner of the screen
      CALL ARCF(100.0,650.0,40.0,450,2700)
      CALL ARCI(100,500,40,450,2700)
      CALL ARCFI(250,650,80,2700,450)
      CALL ARC(250.0,500.0,80.0,2700,450)
C
C       Now, put up a series of filled and unfilled rectangles with
C       the names of their colors printed inside of them across the
C       rest of the top of the screen.
      CALL COLOR(GREEN)
      CALL RECTI(400,600,550,700)
      CALL CMOV2I(420,640)
      CALL CHARST('Green',5)
C
      CALL COLOR(RED)
      CALL RECTFI(600,600,800,650)
      CALL COLOR(BLACK)
      CALL CMOV2(690.0,620.0)
      CALL CHARST('Red',3)
C
      CALL COLOR(BLUE)
      CALL RECT(810.0,700.0,1000.0,20.0)
      CALL CMOV2I(900,300)
      CALL CHARST('Blue',4)
C
C       Now draw some text with a ruler on top to measure it by.
C
C       First the ruler:
      CALL COLOR(BLACK)
      CALL MOVE2I(300,400)
      CALL DRAW2I(650,400)
      DO 100 I=300,650,10
        CALL MOVE2I(I,400)
        CALL DRAW2I(I,410)
100   CONTINUE
C
C       Then some text:
      CALL CMOV2I(300,380)
      STRING = 'The first line is drawn incorrectly '
      CALL CHARST(STRING,LEN(STRING))
      CALL CHARST('in two parts.',13)
C       NOTE:  Fortran pads STRING with spaces to its defined length,
C       and LEN(STRING) returns the defined length (50) instead of the
C       length of the character substring inserted into it.
C
      CALL CMOV2I(300,364)
      STRING = 'This line is drawn correctly '
      STRLEN = LEN('This line is drawn correctly ')
      CALL CHARST(STRING,STRLEN)
C       This is the only way (other than counting by hand, as was
C       done for the second part of this line, below) of getting
C       the length of the actual set of characters to be printed.
      CALL CHARST('in two parts.',13)
C
      CALL CMOV2I(300,352)
      STRLEN = LEN('This line is only 12 pixels lower.')
      CALL CHARST('This line is only 12 pixels lower.',STRLEN)
C
      CALL CMOV2I(300,338)
      STRLEN = LEN('Now move down 14 pixels ...')
      CALL CHARST('Now move down 14 pixels ...',STRLEN)
C
      CALL CMOV2I(300,322)
      STRLEN = LEN('And now down 16 ...')
      CALL CHARST('And now down 16 ...',STRLEN)
C
      CALL CMOV2I(300,304)
      STRLEN = LEN('Now 18 ...')
      CALL CHARST('Now 18 ...',STRLEN)
C
      CALL CMOV2I(300,284)
      STRLEN = LEN('And finally, 20 pixels.')
      CALL CHARST('And finally, 20 pixels.',STRLEN)
C
C       Finally, show off the entire font.  The cmov2i() before each
C       character is necessary in case that character is not defined.
      DO 300 I=0,3
        DO 200 J=0,31
          CALL CMOV2I(300 + 9*J, 200 - 18*I)
          ONECH(1:1) = CHAR(32*I + J)
          CALL CHARST(ONECH,1)
200     CONTINUE
300   CONTINUE
C
      DO 500 I=0,3
        CALL CMOV2I(300, 100 - 18*I)
        DO 400 J=0,31
          ONECH(1:1) = CHAR(32*I + J)
          CALL CHARST(ONECH,1)
400     CONTINUE
500   CONTINUE
C
      CALL GEXIT()
      STOP
      END
