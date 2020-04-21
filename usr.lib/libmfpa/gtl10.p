(*UNIT tan_log10;*)
MODULE $tan_log10;



INCLUDE 'gdcl.h';
INCLUDE 'gur.h';
INCLUDE 'gpasc.h';

{ Single precision routines for Tangent and Log base 10. }
{ Copyright 1981, Richard E. James III. }

{$R-}
{$I-}
{$%+}

(*#B-*)

(*INTERFACE

USES {$U gdcl.obj} util_dcl,
     {$U gur.obj}  util_real,
     {$U gpasc.obj} pascal_single;
*)


FUNCTION $tan(arg:REAL) : REAL;FORWARD;
FUNCTION $log10(arg:REAL) : REAL;FORWARD;

(*IMPLEMENTATION { -------------------------------------- }*)


PROCEDURE s_tan
  (VAR result:REAL;
   arg:REAL;
   VAR error:ERROR_TYPE);
VAR
   multiple:INTEGER;
   x:REAL;
   xsq:REAL;
BEGIN
   $pi_reduce(arg, x, multiple, error);
   IF error = none THEN BEGIN
      xsq := sqr(x);
      result := x + x * xsq * (0.333335034 -
                        xsq * (0.329212565 / (xsq - 2.46931313)));
      IF odd(multiple) THEN
         result := -1.0 / result;
   END;
END;

FUNCTION $tan { ( arg:REAL ) : REAL } ;
VAR
   error:ERROR_TYPE;
   result:REAL;
BEGIN
   s_tan(result, arg, error);
   IF error = none THEN $tan := result
                   ELSE $tan := $t_error('tan   ', error);
END;

FUNCTION $log10 { ( arg:REAL ) : REAL } ;
VAR
   error:ERROR_TYPE;
   result:REAL;
BEGIN
   $t_log(result, arg, error);
   IF error = none THEN $log10 := result / 2.30258509
                   ELSE $log10 := $t_error('alog10', error);
END;

(*END.*)
.



