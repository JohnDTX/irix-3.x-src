C File: cmplxf.for
C Date: 23-Jun-86

$system
Copyright Richard E. James III, 1981.
C Basic Complex arithmetic for Fortran.

      subroutine %C_ARI ( n, ar, ai, br, bi )
      real ai,ar, bi,br
c Input: two complex numbers;
c Output: one complex number, put into ar,ai.
      real q,s, cr

      goto (111,222,333,444),n

111   continue
*---------------------------------------------------------ADD
      ar = ar+br
      ai = ai+bi
      RETURN

222   continue
*---------------------------------------------------------SUB
      ar = ar-br
      ai = ai-bi
      RETURN

333   continue
*---------------------------------------------------------MUL
      cr = ar
      ar = cr*br - ai*bi
      ai = cr*bi + ai*br
      RETURN

444   continue
*---------------------------------------------------------DIV
C The algorithm given here avoids spurious overflows
C   and most underflows, plus it is fairly fast.
      cr = ar
      IF (abs(br).gt.abs(bi)) THEN
          q = bi/br
          s = br + bi*q
          ar = (cr+ai*q)/s
          ai = (ai-cr*q)/s
          RETURN
      ELSE
          q = br/bi
          s = bi + br*q
          ar = (ai+cr*q)/s
          ai = (q*ai-cr)/s
          RETURN
      ENDIF

      END

C Complex raised to a complex power.
*---------------------------------------------- (complex ** complex)
      complex function %_cupc(cb,ce)

      complex cb,ce
      integer ierror
      logical %c_cck
      complex %_cexp,%_clog

      if (%c_cck(ierror,cb)) then
          if (%c_cck(ierror,ce)) then
              %_CUPC = %_cexp(ce * %_clog(cb))
          endif
      endif
      if (ierror.ne.0) then
          call %_cerr(ce,'C ** C',ierror)
          %_cupc = ce
      endif
      end
C Complex raised to an integer power.
*---------------------------------------------- (complex ** integer)
      complex function %_cupi(cb,ie)

      complex cb,ans,pow
      integer ierror,ie
      logical %c_cck,Signed

      if (%c_cck(ierror,cb)) then
          if (ie .lt. 0) then
             Signed = .true.
             ie = -ie
          else
             Signed = .false.
          endif
          ans = (1.0,0.0)
          if (ie .gt. 0) then
             pow = cb
  200        if (mod(ie,2) .gt. 0) ans = ans*pow
             pow = pow*pow
             ie = ie / 2
             if (ie .gt. 0) goto 200
          else
             if (real(cb).eq.0 .and. aimag(cb).eq.0) ierror = 3
          endif
          if (Signed) then
             %_cupi = 1 / ans
          else
             %_cupi = ans
          endif
      endif
      if (ierror .ne. 0) then
          call %_cerr(ce,'C ** I',ierror)
          %_cupi = ce
      endif

      end
C Complex Fortran functions.
*-----------------------------------------------------------CABS
      real    function %_CABS
     +                        ( c )
      complex c
      integer ierror
      real mn,mx, q
      logical %c_cck

      IF (%c_cck(ierror,c)) THEN
          mn = min(abs(real(c)), abs(aimag(c)))
          mx = max(abs(real(c)), abs(aimag(c)))
          IF (mx.eq.0) THEN
              %_CABS = 0
          ELSE
C This algorithm is good to about 1.4 ulp.
              q = mn/mx
              %_CABS = mx + mn*q / (1.+sqrt(1.+q*q))
          ENDIF
      ELSE
          call %_cerr(mn, 'CABS  ',ierror)
          %_cabs = mn
      ENDIF
      RETURN
      END
*-----------------------------------------------------------CCOS
      complex function %_CCOS
     +                        ( c )
      complex c, ce
      logical %c_cck

      IF (%c_cck(ierror,c)) THEN
          %_CCOS = cmplx( cos(real(c)) * cosh(aimag(c)),
     ,                  - sin(real(c)) * sinh(aimag(c)) )
      ELSE
          call %_cerr(ce, 'CCOS  ',ierror)
          %_ccos = ce
      ENDIF
      RETURN
      END
*-----------------------------------------------------------CEXP
      complex function %_CEXP
     +                        ( c )
      complex c, ce
      integer ierror
      real e
      logical %c_cck

      IF (%c_cck(ierror,c)) THEN
          e = exp(real(c))
          %_CEXP = cmplx( e * cos(aimag(c)), e * sin(aimag(c)) )
      ELSE
          call %_cerr(ce, 'CEXP  ',ierror)
          %_cexp = ce
      ENDIF
      RETURN
      END
*-----------------------------------------------------------CLOG
      complex function %_CLOG
     +                        ( c )
      complex c, ce
      integer ierror,domerr
      real mn,mx,u
      logical %c_cck
      parameter (domerr=3)

      IF (%c_cck(ierror,c)) THEN
          mn = min(abs(real(c)),abs(aimag(c))) 
          mx = max(abs(real(c)),abs(aimag(c)))
          IF (.5 .lt. mx .and. mx .lt. 1.22) THEN
C             Work hard on getting an accurate real part to the answer
C              when the argument is near the unit circle.
              u = .5 * log(1. + ((mx-1)*(mx+1) + mn*mn))
          ELSEIF (mx.eq.0) THEN
              ierror = domerr
          ELSE
              u = mn/mx
              u = log(mx) + .5 * log(1. + u*u)
          ENDIF
      ENDIF
      IF (ierror.eq.0) THEN
          %_CLOG = cmplx ( u, atan2(aimag(c), real(c)) )
      ELSE
          call %_cerr(ce, 'CLOG  ',ierror)
          %_clog = ce
      ENDIF
      RETURN
      END
*-----------------------------------------------------------CSIN
      complex function %_CSIN
     +                        ( c )
      complex c, ce
      integer ierror
      logical %c_cck

      IF (%c_cck(ierror,c)) THEN
          %_CSIN = cmplx( sin(real(c)) * cosh(aimag(c)),
     ,                    cos(real(c)) * sinh(aimag(c)) )
      ELSE
          call %_cerr(ce, 'CSIN  ',ierror)
          %_csin = ce
      ENDIF
      RETURN
      END
*-----------------------------------------------------------CSQRT
      complex function %_CSQT
     +                        ( c )
      complex c, ce
      integer ierror
      real t,s,tt, %_cabs
      logical %c_cck

      IF (%c_cck(ierror,c)) THEN
          t = %_cabs(c)
          IF (t.eq.0) THEN
              %_CSQT = c
              RETURN
          ENDIF
          s = sqrt(0.5 * (t + abs(real(c))))
          t = aimag(c) / (s+s)
          IF (real(c).lt.0) THEN
              tt = t
              t = sign(s,t)
              s = abs(tt)
          ENDIF
      ENDIF
      IF (ierror.eq.0) THEN
          %_CSQT = cmplx(s,t)
      ELSE
          call %_cerr(ce, 'CSQRT ',ierror)
          %_csqt = ce
      ENDIF
      RETURN
      END

