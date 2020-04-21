(*UNIT ftn_single;*)
MODULE $ftn_single;


INCLUDE 'gdcl.h';		(* global declarations *)
INCLUDE 'gur.h';
INCLUDE 'gpasc.h';

{ Single precision routines for Fortran (other than those for Pascal). }
{ Copyright 1981, 1982, 1983, Richard E. James III. }

{$R-}
{$I-}
{$$+}

(*#B-*)

(*INTERFACE

USES {$U gdcl.obj} util_dcl,
     {$U gur.obj}  util_real,
     {$U gpasc.obj} pascal_single;
*)

FUNCTION $asin(arg:REAL) : REAL;FORWARD;
FUNCTION $acos(arg:REAL) : REAL;FORWARD;
FUNCTION $atan2(y,x:REAL) : REAL;FORWARD;
FUNCTION $sinh(arg:REAL) : REAL;FORWARD;
FUNCTION $cosh(arg:REAL) : REAL;FORWARD;
FUNCTION $tanh(arg:REAL) : REAL;FORWARD;
FUNCTION $pow(base,power:REAL) : REAL;FORWARD;
FUNCTION $up_i(base:REAL; pow:LONGINT) : REAL;FORWARD;

(*IMPLEMENTATION { -------------------------------------- }*)


FUNCTION $s_g_n(arg:REAL) : REAL;
BEGIN
   IF arg < 0 THEN
      $s_g_n := -1.0
   ELSE
      $s_g_n := +1.0;
END;

{  Common code for ASIN and ACOS }

PROCEDURE $s_asncs
  (arg:REAL;
   VAR result:REAL;
   VAR reduced:BOOLEAN;
   VAR error:ERROR_TYPE);
VAR
   x:REAL;
   xsq:REAL;
BEGIN
   IF $t_check(error, arg) = none THEN BEGIN
      x := abs(arg);
      IF x > 1.0 THEN
         error := domain_error
      ELSE BEGIN         IF x < 0.7071 THEN
            reduced := false
         ELSE BEGIN
            $t_sqrt(x, (1.0-x)*(1.0+x), error);
            reduced := true;
         END;
         xsq := sqr(x);
         result := x + x*xsq *
             (0.491821737 + xsq*(-0.389772686 + xsq*(0.0238257453))) /
             (2.95091040  + xsq*(-3.66618323  + xsq))
      END;
   END;
END;
PROCEDURE $s_$f_asin
  (VAR result:REAL;
   arg:REAL;
   VAR error:ERROR_TYPE);
CONST
   piover2=1.5707963268;
VAR
   reduced:BOOLEAN;
BEGIN
   $s_asncs(arg, result, reduced, error);
   IF error = none THEN BEGIN
      IF reduced THEN
         result := piover2 - result;
      IF arg < 0 THEN
         result := -result;
   END;
END;



PROCEDURE $s_acos
  (VAR result:REAL;
   arg:REAL;
   VAR error:ERROR_TYPE);
CONST
   pi=3.141592654;
   piover2=1.5707963268;
VAR
   reduced:BOOLEAN;
BEGIN
   $s_asncs(arg, result, reduced, error);
   IF error = none THEN BEGIN
      IF arg < 0 THEN
         IF reduced THEN
            result := pi - result
         ELSE
            result := piover2 + result
      ELSE
         IF NOT reduced THEN
            result := piover2 - result;
       { ELSE
            result := result }
   END;
END;
PROCEDURE $s_atan2
  (VAR result:REAL;
   y:REAL;
   x:REAL;
   VAR error:ERROR_TYPE);
CONST
   pi=3.141592654;
   piover2=1.570796327;
BEGIN
   IF $t_check(error, y) = none THEN
      IF $t_check(error, x) = none THEN
         IF (x=0) AND (y=0) THEN
            error := domain_error
         ELSE
            IF (x=0) OR (($t_logb(y)-$t_logb(x)) > 26.0) THEN
               result := $s_g_n(y) * piover2
            ELSE BEGIN
               $t_atan(result, y/x, error);
               IF x < 0 THEN
                  result := $s_g_n(y) * pi + result;
            END;
END;
{  Range reduction for EXP, SINH, COSH }

PROCEDURE $s_tanh
  (VAR result:REAL;
   arg:REAL;
   VAR error:ERROR_TYPE);
VAR
   multiple:INTEGER;
   x,xsq,p,q,temp:REAL;
BEGIN
   IF $t_check(error, arg) = none THEN BEGIN
      x := abs(arg);
      IF x < 0.75 THEN BEGIN
         xsq := sqr(x);
         result := arg + arg*xsq *
            (-0.8239962892 + xsq*(-0.003757325)) /
            (+2.47199617 + xsq);
      END
      ELSE BEGIN
         IF x > 9.011 THEN
            result := 1.0
         ELSE BEGIN
            x := x * 2.88539008;       { 2/ln(2) }
            multiple := round(x);
            x := x - multiple;
            xsq := sqr(x);
            p := x * (7.21504804 + xsq * 0.0576995815);
            q := 20.8182281 + xsq;
            $t_pack(q+p, multiple, temp, error);
            result := 1.0 - 2.0*(q-p) / (temp+(q-p))
         END;
         IF arg < 0.0 THEN
            result := -result;
      END;
   END;
END;

PROCEDURE $s_sncsh
  (VAR result:REAL;
   arg:REAL;
   VAR error:ERROR_TYPE;
   sinh_cosh_flag:INTEGER);       {0=sinh, 1=cosh}
VAR
   multiple:INTEGER;
   x:REAL;
   xsq:REAL;
   p:REAL;
   q:REAL;
BEGIN
   IF $t_check(error, arg) = none THEN BEGIN
      x := abs(arg);
      IF x < 0.75 THEN BEGIN
         xsq := sqr(x);
         IF sinh_cosh_flag = 0 THEN
            result := x + x * xsq * (0.166666786
                            + xsq * (0.00833225464
                            + xsq * (0.000201509721)))
         ELSE
            result := 1.0 + xsq * (0.500001028
                          + xsq * (0.0416571793
                          + xsq * (0.00141651677)));
      END ELSE BEGIN
         $e_reduce(x, p, q, multiple, error);
         IF error = none THEN BEGIN
            x := p / (q-p);
            IF multiple < 14 THEN BEGIN
               $t_pack(0.5/(x+0.5), -2*multiple-1, xsq, error);
               IF sinh_cosh_flag = 0 THEN
                  x := x-xsq
               ELSE                         x := x+xsq;
            END;
            $t_pack(0.5+x, multiple, result, error);
         END;
      END;
      IF (arg<0) AND (error=none) AND (sinh_cosh_flag=0) THEN
         result := -result;
   END;
END;

PROCEDURE $s_up_s
  (VAR result:REAL;
   base, pow:REAL;
   VAR error:ERROR_TYPE);
VAR    base_log:REAL;
BEGIN
   IF $t_check(error, base) = none THEN
   IF $t_check(error, pow) = none THEN BEGIN
      IF base=0 THEN BEGIN
         IF pow>0 THEN result:=0.0
                  ELSE error:=overflow;
      END ELSE BEGIN
         $t_log(base_log, base, error);
         IF error = none THEN $t_exp(result, base_log * pow, error);
      END;
   END;
END;


PROCEDURE $s_up_i
  (VAR result:REAL;
   base:REAL;
   pow:LONGINT;
   VAR error:ERROR_TYPE);
VAR    temp:REAL;
BEGIN
   IF $t_check(error, base) = none THEN
   IF (-1<=pow) AND (pow<=2) THEN BEGIN
      CASE pow OF
        -1:  result := 1.0/base;
         0:  IF base=0 THEN error:=domain_error
                       ELSE result:=1;
         1:  result:=base;
         2:  result:=base*base
      END;
   END ELSE BEGIN
      temp:=pow;
      $s_up_s(result, abs(base), temp, error);
      IF (base<0) and (odd(abs(pow))) THEN result:=-result;
   END;
END;

FUNCTION $asin { ( arg:REAL ) : REAL } ;
VAR
   error:ERROR_TYPE;
   result:REAL;
BEGIN
   $s_$f_asin(result, arg, error);
   IF error = none THEN $asin := result
                   ELSE $asin := $t_error('asin  ', error);
END;


FUNCTION $acos { ( arg:REAL ) : REAL } ;
VAR
   error:ERROR_TYPE;
   result:REAL;
BEGIN
   $s_acos(result, arg, error);
   IF error = none THEN $acos := result
                   ELSE $acos := $t_error('acos  ', error);
END;


FUNCTION $atan2 { ( y,x:REAL ) : REAL } ;
VAR
   error:ERROR_TYPE;
   result:REAL;
BEGIN
   $s_atan2(result, y, x, error);
   IF error = none THEN $atan2 := result
                   ELSE $atan2 := $t_error('atan2 ', error);
END;

FUNCTION $sinh { ( arg:REAL ) : REAL } ;
VAR
   error:ERROR_TYPE;
   result:REAL;
BEGIN
   $s_sncsh(result, arg, error, 0);
   IF error = none THEN $sinh := result
                   ELSE $sinh := $t_error('sinh  ', error);
END;


FUNCTION $cosh { ( arg:REAL ) : REAL } ;
VAR
   error:ERROR_TYPE;
   result:REAL;
BEGIN
   $s_sncsh(result, arg, error, 1);
   IF error = none THEN $cosh := result
                   ELSE $cosh := $t_error('cosh  ', error);
END;


FUNCTION $tanh { ( arg:REAL ) : REAL } ;
VAR
   error:ERROR_TYPE;
   result:REAL;
BEGIN
   $s_tanh(result, arg, error);
   IF error = none THEN $tanh := result
                   ELSE $tanh := $t_error('tanh  ', error);
END;

FUNCTION $pow { ( base,power:REAL ) : REAL } ;
VAR
   error:ERROR_TYPE;
   result:REAL;
BEGIN
   $s_up_s(result, base,power, error);
   IF error = none THEN $pow := result
                   ELSE $pow := $t_error('r**r  ', error);
END;

FUNCTION $up_i { ( base:REAL; pow:LONGINT ) : REAL } ;
VAR
   error:ERROR_TYPE;
   result:REAL;
BEGIN
   $s_up_i(result, base,pow, error);
   IF error = none THEN $up_i := result
                   ELSE $up_i := $t_error('r**r  ', error);
END;

(*END.*)
.


