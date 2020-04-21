CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C      Installation note:
C    Various Fortran compilers may require different styles of INCLUDEs.
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
$     INCLUDE /usr/include/fgl.h
C
      REAL BEZMAT(4,4), CARMAT(4,4), BSPMAT(4,4), GEOM1(3,4)
      INTEGER*2 BEZI, CARD, BSPL
      REAL ASIXTH, MSIXTH, TTHRDS
      INTEGER I, NOP
      PARAMETER ( BEZIER = 1 )
      PARAMETER ( CARDIN = 2 )
      PARAMETER ( BSPLIN = 3 )
      PARAMETER ( ASIXTH = 1.0/6.0 )
      PARAMETER ( MSIXTH = -1.0/6.0 )
      PARAMETER ( TTHRDS = 2.0/3.0 )
C
      DATA BEZMAT /-1.0,  3.0, -3.0,  1.0,
     2              3.0, -6.0,  3.0,  0.0,
     3             -3.0,  3.0,  0.0,  0.0,
     4              1.0,  0.0,  0.0,  0.0/
      DATA CARMAT /-0.5,  1.5, -1.5,  0.5,
     2              1.0, -2.5,  2.0, -0.5,
     3             -0.5,  0.0,  0.5,  0.0,
     4              0.0,  1.0,  0.0,  0.0/
      DATA BSPMAT /MSIXTH,    0.5,   -0.5, ASIXTH,
     2                0.5,   -1.0,    0.5,    0.0,
     3               -0.5,    0.0,    0.5,    0.0,
     4             ASIXTH, TTHRDS, ASIXTH,    0.0/
      DATA GEOM1 /100.0, 100.0, 0.0,
     2            200.0, 200.0, 0.0,
     3            200.0,   0.0, 0.0,
     4            300.0, 100.0, 0.0/
C
      CALL GINIT
      CALL COLOR(BLACK)
      CALL CLEAR()
C       define a basis matrix called BEZIER
      CALL DEFBAS(BEZIER,BEZMAT)
C       identify the BEZIER matrix as the current basis matrix
      CALL CURVEB(BEZIER)
C       set the current precision to 20
C       (the curve segment will be drawn using 20 line segments)
      CALL CURVEP(20)
      CALL COLOR(RED)
C       draw the curve based on the four control points in geom1
      CALL CRV(GEOM1)
C
C       define a new basis
      CALL DEFBAS(CARDIN,CARMAT)
C       reset the current basis
      CALL CURVEB(CARDIN)
C       note that the curveprecision does not have to
C       be restated unless it is to be changed
      CALL COLOR(BLUE)
C       draw a new curve segment
      CALL CRV(GEOM1)
C
C       define a new basis
      CALL DEFBAS(BSPLIN,BSPMAT)
C       reset the current basis
      CALL CURVEB(BSPLIN)
      CALL COLOR(GREEN)
C       draw a new curve segment
      CALL CRV(GEOM1)
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C      Installation note:
C       This is a delay loop, which may require adjustment.
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
      CALL GFLUSH()
      DO 1000 I=1,1000000
      NOP = I * 2
1000  CONTINUE
      CALL GEXIT()
      STOP
      END
