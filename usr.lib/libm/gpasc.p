(*UNIT pascal_single;*)
MODULE $pascal_single;

INCLUDE 'gdcl.h';
INCLUDE 'gur.h';

{ Single precision routines for Fortran (other than those for Pascal). }
{ Copyright 1981, Richard E. James III. }

{$R-}
{$I-}
{$%+}

(*#B-*)

(*INTERFACE

USES {$U gdcl.obj} util_dcl,
     {$U gur.obj}  util_real;
*)


PROCEDURE $pi_reduce
  (arg:REAL;
   VAR result:REAL;
   VAR multiple:INTEGER;
   VAR error:ERROR_TYPE);FORWARD;
PROCEDURE $e_reduce
  (arg:REAL;
   VAR p:REAL;
   VAR q:REAL;
   VAR multiple:INTEGER;
   VAR error:ERROR_TYPE);FORWARD;
PROCEDURE $t_atan(VAR result:REAL; arg:REAL; VAR error:ERROR_TYPE);FORWARD;
PROCEDURE $t_exp(VAR result:REAL; arg:REAL; VAR error:ERROR_TYPE);FORWARD;
PROCEDURE $t_log(VAR result:REAL; arg:REAL; VAR error:ERROR_TYPE);FORWARD;
PROCEDURE $t_sqrt
  (VAR y:REAL;
   x:REAL;
   VAR error:ERROR_TYPE);FORWARD;
FUNCTION $sin ( arg: REAL ) : REAL;FORWARD;
FUNCTION $cos ( arg: REAL ) : REAL;FORWARD;
FUNCTION $atan ( arg: REAL ) : REAL;FORWARD;
FUNCTION $exp ( arg: REAL ) : REAL;FORWARD;
FUNCTION $log ( arg: REAL ) : REAL;FORWARD;
FUNCTION $sqrt ( arg: REAL ) : REAL;FORWARD;

(*IMPLEMENTATION*)

PROCEDURE $t_sqrt
  {VAR y:REAL;
   x:REAL;
   VAR error:ERROR_TYPE};
VAR
   mantissa:real;
   approx:real;
   exponent:INTEGER;
BEGIN
   IF $t_check(error, x) = none THEN BEGIN
      IF x < 0 THEN
         error := domain_error
      ELSE IF x = 0 THEN
         y := 0.0
      ELSE BEGIN
         $t_unpack(x,mantissa,exponent);
         IF odd(exponent) THEN BEGIN
            mantissa := mantissa / 2.0;
            exponent := exponent + 1;
            approx := 0.5901621 * mantissa + 0.4173076;
         END ELSE
            approx := 0.4173076 * mantissa + 0.5901621;

         approx := approx + (mantissa/approx);
         approx := approx + (4.0*mantissa/approx);
         $t_pack(approx, (exponent DIV 2)-2, y, error);
      END;
   END;
END;
{ $PAGE$ }
{  Range reduction for SIN, COS, and TAN }

PROCEDURE $pi_reduce
  {arg:REAL;
   VAR result:REAL;
   VAR multiple:INTEGER;
   VAR error:ERROR_TYPE};
CONST
   precision=10;
   half_word_size=11;   { Precision of 'pi' chunks, in bits }
   max_input=2147483647.0;
VAR
   pi:ARRAY[1..precision] OF RECORD
      CASE BOOLEAN OF
         true:  (ii: LONGINT);
         false: (rr: REAL)
      END;
   upper:REAL;
   lower:REAL;
   r_lower:REAL;
   cutoff:INTEGER;
   rtemp:REAL;
   i:INTEGER;
   itemp:INTEGER;
BEGIN
   IF $t_check(error, arg) = none THEN BEGIN
      IF abs(arg) > max_input THEN
         error := infeasible
      ELSE IF abs(arg) < 0.7854 THEN BEGIN  { This constant may }
         result := arg;                     { be perturbed to   }
         multiple := 0;                   { assure monotonicity }
{ $PAGE$ }
      END ELSE BEGIN
         upper := $t_halfsz(arg);         lower := arg - upper;
           { 'precision' values of 2/pi, }
           { each containing 'half_word_size' bits: }
         pi[1].ii := 1059250176  { 0.63623047 } ;
         pi[2].ii :=  969670656  { 0.38909912E-03 } ;
         pi[3].ii :=  878411776  { 0.20442531E-06 } ;
         pi[4].ii :=  782008320  { 0.71167960E-10 } ;
         pi[5].ii :=  679641088  { 0.14488410E-13 } ;
         pi[6].ii :=  596959232  { 0.16141060E-16 } ;
         pi[7].ii :=  510689280  { 0.12731964E-19 } ;
         pi[8].ii :=  418045952  { 0.60713764E-23 } ;
         pi[8].ii :=  324132864  { 0.26489949E-26 } ;
         pi[10].ii:=  229220352  { 0.10453948E-29 } ;
         result := pi[1].rr * upper;
         cutoff := $t_logb(result) - (half_word_size - 2);
         multiple := round(result);
         result := result - multiple;
         r_lower := 0.0;
         i := 1;
         WHILE ($t_logb(result) < cutoff) AND
               (i+2 < precision) DO BEGIN
            r_lower := r_lower + (lower*pi[i].rr + upper*pi[i+1].rr);
            rtemp := result + r_lower;
            IF $t_logb(rtemp) >= (-1) THEN BEGIN   { >= 0.5 }
               itemp := round(rtemp);
               multiple := multiple + itemp;
               result := result - itemp;
               rtemp := result + r_lower;
            END;
            r_lower := (result - rtemp) + r_lower;
            result := rtemp;
            cutoff := cutoff - half_word_size;
            i := i+1;
         END;
         result := 1.5707963 * (( (lower * pi[i].rr
                                   + arg * (pi[i+1].rr + pi[i+2].rr))
                               +  r_lower) + result);
      END;
   END;
END;
{ $PAGE$ }
PROCEDURE s_sncs
  (VAR result:REAL;
   arg:REAL;
   VAR error:ERROR_TYPE;
   sin_cos_flag:INTEGER);
VAR
   multiple:INTEGER;
   x:REAL;
   xsq:REAL;
BEGIN
   $pi_reduce(arg, x, multiple, error);
   IF error = none THEN BEGIN
      xsq := sqr(x);
      multiple := multiple + sin_cos_flag;
      IF odd(multiple) THEN BEGIN
         result := 1.0 + xsq * (-4.99998424e-1 +       { cos }
                         xsq * (+4.16544195e-2  +
                         xsq * (-1.35794036e-3 )));
         multiple := multiple - 1;
      END ELSE
         result := x + x * xsq * (-1.66666502e-1 +     { sin }
                           xsq * (+8.33201640e-3 +
                           xsq * (-1.95018148e-4 )));
      IF odd(multiple DIV 2) THEN
         result := - result;
   END;
END;


PROCEDURE $t_atan
  {VAR result:REAL;
   arg:REAL;
   VAR error:ERROR_TYPE};
CONST
   piover2=1.5707963268;
VAR
   x:REAL;
   xsq:REAL;
   reduced:BOOLEAN;
BEGIN
   IF $t_check(error, arg) = none THEN BEGIN
      x := abs(arg);
      IF x < 1.0 THEN
         reduced := false
      ELSE BEGIN
         x := 1.0 / x;
         reduced := true;
      END;
      xsq := sqr(x);
      result := x + x*xsq *
         (-1.21348506 + xsq*(-0.675258397+ xsq*(-0.010497842))) /
         (+3.64048526 + xsq*(+4.20958442 + xsq));
      IF reduced THEN
         result := piover2 - result;
      IF arg < 0 THEN
         result := -result;
   END;
END;


PROCEDURE $e_reduce
  {arg:REAL;
   VAR p:REAL;
   VAR q:REAL;
   VAR multiple:INTEGER;
   VAR error:ERROR_TYPE};
CONST
   min_input=-10000.0;
   max_input=+10000.0;
   upper_log_2_inv=1.4423828125;
   lower_log_2_inv=0.000312228389;
VAR
   upper:REAL;
   lower:REAL;
   x:REAL;
   xsq:REAL;
   i:INTEGER;
   itemp:INTEGER;
BEGIN
   IF $t_check(error, arg) = none THEN 
      IF arg < min_input THEN 
         error := underflow
      ELSE IF arg > max_input THEN
         error := overflow
      ELSE BEGIN
         upper := $t_halfsz(arg);
         lower := arg - upper;
         x := upper * upper_log_2_inv;
         multiple := round(x);
         x := (x - multiple) +
                   (lower*upper_log_2_inv + arg*lower_log_2_inv);
         xsq := sqr(x);
         p := x * (7.21504804 + 0.05769958151 * xsq);
         q := 20.81822806+xsq;
      END;
END;
{ $PAGE$ }
PROCEDURE $t_exp
  {VAR result:REAL;
   arg:REAL;
   VAR error:ERROR_TYPE};
VAR
   multiple:INTEGER;
   p:REAL;
   q:REAL;
BEGIN
   $e_reduce(arg, p, q, multiple, error);
   IF error = none THEN BEGIN
      result := 0.5 + p/(q-p);
      $t_pack(result, multiple+1, result, error);
   END;
END;


PROCEDURE $t_log
  {VAR result:REAL;
   arg:REAL;
   VAR error:ERROR_TYPE};
CONST
   log_2=0.693147181;
VAR    x:REAL;
   z:REAL;
   zsq:REAL;
   n:INTEGER;
BEGIN
   IF $t_check(error, arg) = none THEN
      IF arg <= 0.0 THEN
         error := domain_error
      ELSE BEGIN
         $t_unpack(arg, x, n);
         IF x > 1.414213 THEN BEGIN
            x := x / 2.0;
            n := n + 1;
         END;
         z := (x-1.0) / (x+1.0);
         zsq := sqr(z);
         result := n*log_2 + (z+z) +
                   z*zsq*(1.10445938/(1.65677798-zsq));
      END;
END;
{ $PAGE$ }
FUNCTION $sin{(arg:REAL) : REAL};
VAR
   error:ERROR_TYPE;
   result:REAL;
BEGIN
   s_sncs(result, arg, error, 0);
   IF error = none THEN $sin := result
                   ELSE $sin := $t_error('sin   ', error);
END;


FUNCTION $cos{(arg:REAL) : REAL};
VAR
   error:ERROR_TYPE;
   result:REAL;
BEGIN
   s_sncs(result, arg, error, 1);
   IF error = none THEN $cos := result
                   ELSE $cos := $t_error('cos   ', error);
END;


FUNCTION $atan{(arg:REAL) : REAL};
VAR
   error:ERROR_TYPE;
   result:REAL;
BEGIN
   $t_atan(result, arg, error);
   IF error = none THEN $atan := result
                   ELSE $atan := $t_error('atan  ', error);
END;


FUNCTION $exp{(arg:REAL) : REAL};
VAR
   error:ERROR_TYPE;
   result:REAL;
BEGIN
   $t_exp(result, arg, error);
   IF error = none THEN $exp := result
                   ELSE $exp := $t_error('exp   ', error);
END;


FUNCTION $log{(arg:REAL) : REAL};
VAR
   error:ERROR_TYPE;
   result:REAL;
BEGIN
   $t_log(result, arg, error);
   IF error = none THEN $log := result
                   ELSE $log := $t_error('alog  ', error);
END;

FUNCTION $sqrt{(arg:REAL) : REAL};
VAR
   error:ERROR_TYPE;
   result:REAL;
BEGIN
   $t_sqrt(result, arg, error);
   IF error = none THEN $sqrt := result
                   ELSE $sqrt := $t_error('sqrt  ', error);
END;

(*END.*)
.



