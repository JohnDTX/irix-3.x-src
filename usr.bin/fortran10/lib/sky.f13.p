{ File: sky.f13.text }
{ Date: 12-Mar-85 }


{ Replaces: GDCL, GUR, GPASC, GFTN and IUPI in PASLIB and FTNLIB }


{ Copyright Richard E. James III, 1981 }
{ Copyright 1981, 1982, 1983, 1984, 1985 Silicon Valley Software, Inc. }

{$R-} {$I-} {$M-} {$%+}

unit %f13;

interface

const MALE = 0;   { MC68000 }
      FEMALE = 1; { NS32000 }
      
      SEX = MALE;
      
      R_MIN_EXP = -127;
      R_MAX_EXP = 127;
      D_MIN_EXP = -1023;
      D_MAX_EXP = 1023;
 {??ck min/max exp}

type types = 1..5;
     error_type = (NONE,UNDERFLOW,OVERFLOW,DOMAIN_ERROR,
                   NOT_A_NUMBER,INFEASIBLE);
     unp_real = record upper: longint;
                       lower: longint;
                       exp: integer;
                       sgn: -128..127;
                       knd: types;
                end;
     alfa6 = packed array[1..6] of char;
     complex = record re: real; im: real; end;


function %gennan4(i: longint): real;
function %t_logb(arg: real): integer;
function %t_halfsz(arg: real): real;
function %myt_pack (value: real; power_of_two: integer): real;
function %myt_unpack(value: real; var exp: integer): real;

function %mypi_reduce(arg: real; var multiple: longint): real;

function %_ftan(var arg: real): real;
function %_fasin(var arg: real): real;
function %_facos(var arg: real): real;
function %_fatan2(var y,x: real): real;
function %_fsinh(var arg: real): real;
function %_fcosh(var arg: real): real;
function %_ftanh(var arg: real): real;
function %_flog10(var x: real): real;
function %_up_r(base,pow: real): real;
function %_up_i(base: real; pow: longint): real;

implementation

procedure %pf_pack(var val: real; var ur: unp_real); external;
procedure %pf_unpk(    val: real; var ur: unp_real); external;

{-----------------------------------------------------------gennan4---}
function %gennan4{i: longint): real};
  var l: longint; a: real;
begin
l := $7f800000 or i; moveleft(i,a,4); %gennan4 := a;
end; {%gennan4}
{---------------------------------------------------------my unpack---}
function %myt_unpack{value: real; var exp: integer): real};

    {  Routine to unpack real numbers, returning a signed
       mantissa in the range (-2,-1] or [1,2), and an exponent
       in the range [-127,+127] for single.    } 
  var r: record case integer of
                  0: (l: longint);
                  1: (r: real);
                  2: (ia,ib: integer);
         end;
      m: integer;
begin 
r.r := value;
if SEX = MALE
then m := (r.ia and $7f80) div $80
else m := (r.ib and $7f80) div $80;
if m = 255
then begin { NAN or INFINITY }
     exp := $6000 + 23;
     %myt_unpack := value;
     end
else if m = 0
     then if (r.l and $7fffffff) = 0
          then begin { ZERO }
               exp := -126;
               %myt_unpack := r.r;
               end
          else begin { Underflow }
               exp := -126;
               r.l := (r.l and $007fffff) or $3f800000;
               if value < 0.0
               then %myt_unpack := 1.0 - r.r
               else %myt_unpack := r.r - 1.0;
               end
     else begin { Regular number }
          exp := m - 127;
          r.l := (r.l and $807fffff) or $3f800000;
          %myt_unpack := r.r;
          end;
end; {%myt_unpack}

{------------------------------------------------------------logb-----}
function %t_logb{(arg: real): integer};
  var u: unp_real; temp: real;
    
    {  Routine to find the log (base 2) of a single precision number. }
begin
%pf_unpk(arg, u);
%t_logb := u.exp + 23;
end; {%t_logb} 
  
{---------------------------------------------------------my pack-----}
{ Returns: value*2**power if in range, else NAN }

function %myt_pack{value: real; power_of_two: integer;): real};
  var u: unp_real;
begin 
%pf_unpk(value,u);
u.exp := u.exp + power_of_two; 
if u.exp < r_min_exp - 23
then %myt_pack := %gennan4(ord(underflow))
else if u.exp > r_max_exp - 23
     then %myt_pack := %gennan4(ord(overflow))
     else begin %pf_pack(value,u); %myt_pack := value; end;
end; {%myt_pack}
  
{------------------------------------------------------------halfsz---}
function %t_halfsz 
{ (arg:REAL) }                   { In: value to round up }
{ : REAL     }  ;                { Out: rounded value } 
  var temp: real; u: unp_real;
begin 
%pf_unpk(arg, u);
if SEX = MALE
then u.lower := (u.lower + 2048) div 4096
else u.upper := (u.upper + 2048) div 4096;
u.exp := u.exp + 12;
%pf_pack(temp, u);
%t_halfsz := temp; 
end; {%t_halfsz}

{---------------------------------------------------------------------}

{  Range reduction for SIN, COS, and TAN }
  
function %mypi_reduce{(arg: real; var multiple: longint): real};
  const PRECISION = 10;
        HALF_WORD_SIZE = 11;   { Precision of 'pi' chunks, in bits }
        MAX_INPUT = 2147483647.0;
  var pi: array[1..PRECISION] of
               record case Boolean of
                        TRUE:  (ii: longint);
                        FALSE: (rr: real);
                      end;
      upper,lower,r_lower,rtemp,result: real;
      cutoff,i,itemp: integer;
begin

{ Assumes: IsNum(arg) and (abs(arg) <= MAX_INPUT)! }

if abs(arg) < 0.7854 { This constant may be perturbed }
then begin           { to assure monotonicity }
     %mypi_reduce := arg;
     multiple := 0;
     end
else begin
     upper := %t_halfsz(arg);         lower := arg - upper;
       { PRECISION values of 2/pi, }
       { each containing HALF_WORD_SIZE bits: } 
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
     cutoff := %t_logb(result) - (HALF_WORD_SIZE - 2);   
     multiple := round(result); 
     result := result - multiple; 
     r_lower := 0.0;  
     i := 1;  
     while (%t_logb(result) < cutoff) and 
           (i + 2 < precision) do begin 
           r_lower := r_lower + (lower*pi[i].rr + upper*pi[i+1].rr); 
           rtemp := result + r_lower;  
           if %t_logb(rtemp) >= (-1)    { >= 0.5 }  
           then begin
                itemp := round(rtemp); 
                multiple := multiple + itemp;  
                result := result - itemp;  
                rtemp := result + r_lower; 
                end;
           r_lower := (result - rtemp) + r_lower;
           result := rtemp;
           cutoff := cutoff - HALF_WORD_SIZE;
           i := i + 1; 
           end; 
     %mypi_reduce := 1.5707963 * (( (lower * pi[i].rr
                                     + arg * (pi[i+1].rr + pi[i+2].rr))
                                 +  r_lower) + result); 
     end;
end; {%mypi_reduce}

{  Common code for ASIN and ACOS }
  
procedure %s_asncs(arg: real;
                   var result: real;
                   var Reduced: Boolean;
                   var error: error_type);
  var x,xsq: real;
begin 
if IsNum(arg)
then begin
     x := abs(arg);
     if x > 1.0
     then error := DOMAIN_ERROR
     else begin 
          error := NONE;
          if x < 0.7071
          then Reduced := FALSE
          else begin
               x := sqrt((1.0-x)*(1.0+x));
               Reduced := TRUE;
               end;
          xsq := sqr(x); 
          result := x + x*xsq *
              (0.491821737 + xsq*(-0.389772686 + xsq*(0.0238257453))) /
              (2.95091040  + xsq*(-3.66618323  + xsq)) 
          end; 
     end
else error := NOT_A_NUMBER;
end; {%s_asncs}

function %mys_hsncs(arg: real; sinh_cosh_flag: integer): real; {0=sinh, 1=cosh}
  const MIN_INPUT = -10000.0;
        MAX_INPUT = +10000.0;
        UPPER_LOG_2_INV = 1.4423828125;
        LOWER_LOG_2_INV = 0.000312228389;
  var x,xsq,upper,lower,result,p,q,tx: real;
      multiple: integer;
begin 
if IsNum(arg)
then begin
     x := abs(arg);
     if x < 0.75
     then begin
          xsq := sqr(x);
          if sinh_cosh_flag = 0
          then result := x + x * xsq * (0.166666786
                               + xsq * (0.00833225464
                               + xsq * (0.000201509721)))
          else result := 1.0 + xsq * (0.500001028
                             + xsq * (0.0416571793
                             + xsq * (0.00141651677)));
          end
     else if x > MAX_INPUT
          then result := %gennan4(ord(OVERFLOW))      
          else begin
               upper := %t_halfsz(x);
               lower := x - upper;
               tx := upper * UPPER_LOG_2_INV;
               multiple := round(tx);
               tx := (tx - multiple) +
                         (lower*UPPER_LOG_2_INV + x*LOWER_LOG_2_INV);
               xsq := sqr(tx); 
               p := tx * (7.21504804 + 0.05769958151 * xsq); 
               q := 20.81822806+xsq; 
               x := p / (q-p); 
               if multiple < 14
               then begin
                    xsq := %myt_pack(0.5/(x+0.5),-2*multiple-1);
                    if sinh_cosh_flag = 0
                    then x := x-xsq  
                    else x := x+xsq; 
                    end;
               result := %myt_pack(0.5 + x,multiple); 
               end;
     if (arg < 0.0) and IsNum(result) and (sinh_cosh_flag = 0)
     then %mys_hsncs := -result
     else %mys_hsncs := result;
     end
else %mys_hsncs := %gennan4(ord(NOT_A_NUMBER));
end; {%mys_hsncs}

function %_ftan{(var arg: real): real};
  const MAX_INPUT = 2147483647.0;
  var result,x,xsq: real; multiple: longint;
begin
if IsNum(arg)
then if abs(arg) <= MAX_INPUT
     then begin
          if abs(arg) < 0.7854     { This constant may be perturbed }
          then begin               { to assure monotonicity }
               multiple := 0;
               x := arg;
               end
          else x := %mypi_reduce(arg,multiple);
          xsq := sqr(x);
          result := x + x * xsq * (0.333335034 -
                            xsq * (0.329212565 / (xsq - 2.46931313)));
          if odd(multiple)
          then %_ftan := -1.0 / result
          else %_ftan := result;
          end
     else %_ftan := %gennan4(ord(INFEASIBLE))
else %_ftan := %gennan4(ord(NOT_A_NUMBER));
end; {%_ftan}

function %_fasin{(var arg: real): real}; 
  const PIOVER2 = 1.5707963268;
  var result: real; error: error_type; Reduced: Boolean;
begin
%s_asncs(arg,result,Reduced,error);
if error = NONE
then begin
     if Reduced
     then result := PIOVER2 - result;
     if arg < 0.0
     then result := -result;
     %_fasin := result;
     end
else %_fasin := %gennan4(ord(error));
end; {%_fasin}

function %_facos{(var arg: real): real}; 
  const PI = 3.141592654;
        PIOVER2 = 1.5707963268;
  var result: real; error: error_type; Reduced: Boolean;
begin 
%s_asncs(arg,result,Reduced,error);
if error = NONE
then if arg < 0.0
     then if Reduced
          then %_facos := PI - result 
          else %_facos := PIOVER2 + result
     else if Reduced
          then %_facos := result
          else %_facos := PIOVER2 - result
else %_facos := %gennan4(ord(error)); 
end; {%_facos}

function %_fatan2{(var y,x: real):real};
  const PI = 3.141592654;
        PIOVER2 = 1.570796327; 
  var result: real;
begin 
if IsNum(x)
then if IsNum(y)
     then if x = 0.0
          then if y = 0.0
               then %_fatan2 := %gennan4(ord(DOMAIN_ERROR))
               else if y < 0.0
                    then %_fatan2 := -PIOVER2
                    else %_fatan2 := PIOVER2
          else if (%t_logb(y) - %t_logb(x)) > 26.0
               then if y < 0.0
                    then %_fatan2 := -PIOVER2
                    else %_fatan2 := PIOVER2
               else begin
                    result := arctan(y/x);
                    if x < 0.0
                    then if y < 0.0
                         then %_fatan2 := result - PI
                         else %_fatan2 := result + PI
                    else %_fatan2 := result;
                    end
     else %_fatan2 := %gennan4(ord(NOT_A_NUMBER))
else %_fatan2 := %gennan4(ord(NOT_A_NUMBER));
end; {%_fatan2}

function %_fsinh{(var arg: real): real}; 
begin 
%_fsinh := %mys_hsncs(arg,0);
end; {%_fsinh}

function %_fcosh{(var arg: real): real}; 
begin 
%_fcosh := %mys_hsncs(arg,1);
end; {%_fcosh}

function %_ftanh{(var arg: real): real}; 
  var x,xsq,p,q,temp,result: real; multiple: integer;
begin
if IsNum(arg)
then begin
     x := abs(arg);
     if x < 0.75
     then begin
          xsq := sqr(x); 
          %_ftanh := arg + arg*xsq *
              (-0.8239962892 + xsq*(-0.003757325)) /  
              (+2.47199617 + xsq);
          end
     else begin
          if x > 9.011
          then result := 1.0
          else begin
               x := x * 2.88539008;       { 2/ln(2) }
               multiple := round(x); 
               x := x - multiple;
               xsq := sqr(x);
               p := x * (7.21504804 + xsq * 0.0576995815); 
               q := 20.8182281 + xsq;
               temp := %myt_pack(q+p, multiple); 
               if IsNum(temp)
               then result := 1.0 - 2.0*(q-p) / (temp+(q-p));
               end;
          if arg < 0.0
          then %_ftanh := -result
          else %_ftanh := result;
          end;
     end
else %_ftanh := %gennan4(ord(NOT_A_NUMBER));
end; {%_ftanh}

function %_flog10{(var x: real): real};  
  var result: real; 
begin 
result := ln(x); 
if IsNum(result)
then %_flog10 := result / 2.30258509  
else %_flog10 := result; 
end; {%_flog10}

function %_up_i{(base: real; pow: longint): real};
  var answer: real; Invert: Boolean; bit: longint;
begin
if IsNum(base)
then begin
     Invert := pow < 0;
     pow := abs(pow); answer := 1;
     if pow > 0
     then begin
          if odd(pow) then answer := base;
          bit := 2;
          while pow >= bit do begin
                base := base*base;
                if (pow and bit) <> 0 then answer := answer*base;
                bit := bit + bit;
                end;
          end;
     if Invert
     then %_up_i := 1 / answer
     else %_up_i := answer;
     end
else %_up_i := %gennan4(15);
end; {%_up_i}

function %_up_r{(base,pow: real): real};
  var l: longint;
begin
if IsNum(base) and IsNum(pow)
then if base <= 0.0
     then if base < 0.0
          then begin
               l := trunc(pow);
               if l = pow
               then %_up_r := %_up_i(base,l)
               else %_up_r := %gennan4(ord(DOMAIN_ERROR));
               end
          else if pow > 0.0
               then %_up_r := 0.0
               else %_up_r := %gennan4(ord(OVERFLOW))
     else %_up_r := exp(ln(base)*pow)
else %_up_r := %gennan4(ord(NOT_A_NUMBER));
end; {%_up_r}

function %_fsin(var x: real): real;
begin
%_fsin := sin(x);
end; {%_fsin}

function %_fcos(var x: real): real;
begin
%_fcos := cos(x);
end; {%_fcos}

function %_fatan(var x: real): real;
begin
%_fatan := arctan(x);
end; {%_fatan}

function %_fexp(var x: real): real;
begin
%_fexp := exp(x);
end; {%_fexp}

function %_fln(var x: real): real;
begin
%_fln := ln(x);
end; {%_fln}

function %_fsqrt(var x: real): real;
begin
%_fsqrt := sqrt(x);
end; {%_fsqrt}

function %i_up_i(a,b: longint): longint;
  var answer: longint; Negate,Invert: Boolean;
begin
Negate := (a < 0) and odd(b);
Invert := b < 0;
a := abs(a); b := abs(b); answer := 1;
while b > 0 do begin
      if odd(b) then answer := answer*a;
      a := a*a; b := b div 2;
      end;
if Negate then answer := -answer;
if Invert then answer := 1 div answer;
%i_up_i := answer;
end; {%i_up_i}

procedure %_cerr(var result: complex; var routine_name: alfa6;
                 len: longint; var error: longint);
begin 
result.re := %gennan4(error); result.im := result.re;
end; {%_cerr} 

{  Routine to test for illegal numbers.
Return an error code as function result & 1st parameter.  }
  
function %c_cck(var error: longint; var value: complex): longint;
begin
if IsNan(value.re) or IsNan(value.im)
then begin error := ord(NOT_A_NUMBER); %c_cck := 0; {Fortran .FALSE.} end
else begin error := ord(NONE); %c_cck := 1; {Fortran .TRUE.} end;
end; {%c_cck}

end. {%f13}

                                                                                                                                        