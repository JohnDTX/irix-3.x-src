CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C      Installation note:
C    Various Fortran compilers may require different styles of INCLUDEs.
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
$     INCLUDE /usr/include/fgl.h
C
      REAL BEZMAT(4,4), CARMAT(4,4), BSPMAT(4,4)
      REAL GEOMX(4,4), GEOMY(4,4), GEOMZ(4,4)
      INTEGER*2 BEZI, CARD, BSPL
      REAL ASIXTH, MSIXTH, TTHRDS, XMAX, YMAX
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
      DATA GEOMX /  0.0, 100.0, 200.0, 300.0,
     2              0.0, 100.0, 200.0, 300.0,
     3            700.0, 600.0, 500.0, 400.0,
     4            700.0, 600.0, 500.0, 400.0/
      DATA GEOMY /400.0, 500.0, 600.0, 700.0,
     2              0.0, 100.0, 200.0, 300.0,
     3              0.0, 100.0, 200.0, 300.0,
     4            400.0, 500.0, 600.0, 700.0/
      DATA GEOMZ /100.0, 200.0, 300.0, 400.0,
     2            100.0, 200.0, 300.0, 400.0,
     3            100.0, 200.0, 300.0, 400.0,
     4            100.0, 200.0, 300.0, 400.0/
C
      CALL GINIT
      CALL COLOR(BLACK)
      CALL CLEAR()
      XMAX = XMAXSC
      YMAX = YMAXSC
      CALL ORTHO(0.0,XMAX,0.0,YMAX,XMAX,-XMAX)
C       define a basis matrix called BEZIER
      CALL DEFBAS(BEZIER,BEZMAT)
C       define a basis matrix called CARDIN
      CALL DEFBAS(CARDIN,CARMAT)
C       define a basis matrix called BSPLIN
      CALL DEFBAS(BSPLIN,BSPMAT)
C
C       a Bezier basis will be used for both directions in the first patch
      CALL PATCHB(BEZIER,BEZIER)
C       7 curve segments will be drawn in the u direction
C       and 4 in the v direction
      CALL PATCHC(4,7)
C       the curve segments in the u direction will consist of 20 line
C       segments (the lowest multiple of vcurves greater than usegments)
C       & the curve segments in the v direction will consist of 21 line
C       segments (the lowest multiple of ucurves greater than vsegments)
      CALL PATCHP(20,20)
      CALL COLOR(RED)
C       the patch is drawn based on the 16 specified control points
      CALL PATCH(GEOMX,GEOMY,GEOMZ)
C
C       reset the bases for both directions
      CALL PATCHB(CARDIN,CARDIN)
      CALL COLOR(GREEN)
C       draw another patch using the same control points
C       but a different basis
      CALL PATCH(GEOMX,GEOMY,GEOMZ)
C
C       reset the bases for both directions again
      CALL PATCHB(BSPLIN,BSPLIN)
      CALL COLOR(BLUE)
C       draw a third patch
      CALL PATCH(GEOMX,GEOMY,GEOMZ)
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
