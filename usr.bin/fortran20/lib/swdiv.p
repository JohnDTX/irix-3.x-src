{ File: d13.text }
{ Date: 01-Feb-85 }

unit dble_transcendentals;
{ Copyright 1982, Richard E. James III. }
{$%+}
{$R-}
{$I-}
{$M-}

INTERFACE

FUNCTION %_dsqt(var arg: double ) : double;
FUNCTION %_dsin(var arg: double ) : double;
FUNCTION %_dcos(var arg: double ) : double;
FUNCTION %_datn(var arg: double ) : double;
FUNCTION %_dexp(var arg: double ) : double;
FUNCTION %_dlog(var arg: double ) : double;
FUNCTION %_dupd (var db: double;   var de: double ): double;
FUNCTION %_dupi (var db: double;   var ie: longint ): double;
FUNCTION %_dtan(var arg: double ) : double;
FUNCTION %_dasn(var arg: double ) : double;
FUNCTION %_dacs(var arg: double ) : double;
FUNCTION %_dat2(var y, x: double ) : double;
FUNCTION %_dsnh(var arg: double ) : double;
FUNCTION %_dcsh(var arg: double ) : double;
FUNCTION %_dtnh(var arg: double ) : double;
FUNCTION %_dl10(var arg: double ) : double;
function %_dupiv(db: double; ie: longint): double;

IMPLEMENTATION


CONST
      pi  =3.14159265358979324d0;
      pi_2=1.57079632679489662d0;
      pi_4=0.78539816339744831d0;
      {codes for 'types':}
      zero = 1;
      inf = 4;
      NaN = 5;

TYPE TYPES = 1..5;
     ERROR_TYPE=(none, underflow, overflow, domain_error, 
                 not_a_number, infeasible); 
     UNP_REAL = RECORD
       UPPER: LONGINT;
       LOWER: LONGINT;
       EXP: INTEGER;
       SGN: -128..127;
       KND: TYPES 
     END;
      err_type = (noerr, undflw, ovrflw, domerr, N_a_N, infeas);
      overlaid = record
          case boolean of
              true:  (d: double);
              false: (i: record  u, l: longint  end);
          end;

PROCEDURE %pd_unpk(var d: double;  var u: unp_real);  EXTERNAL;
PROCEDURE %pd_pack(var d: double;  var u: unp_real);  EXTERNAL;
FUNCTION  %_nint(r: real): longint;  EXTERNAL;
FUNCTION  %d_dnint(d: double): double;  EXTERNAL;
FUNCTION  %d_dint (d: double): double;  EXTERNAL;
FUNCTION  %d_sign (d,s: double): double;  EXTERNAL;

FUNCTION %gen_nan( num: integer): double;
var n: overlaid;
begin
      n.i.u := $7ff00000 + num;
      n.i.l := $00000000;
      %gen_nan := n.d;
end;

FUNCTION %gen_inf: double;
begin %gen_inf := %gen_nan(0); end;

FUNCTION %gen_neginf: double;
var n: overlaid;
begin
      n.i.u := $fff00000;
      n.i.l := $00000000;
	%gen_neginf := n.d;
end;

FUNCTION %_dck1(arg: double;  var result: double): boolean;
{        ------ }
var u: unp_real;
begin
{ This routine checks the validity of arguments to SQRT and LOG. }
{   Argument:     Result:  }
{   NaN           same NaN }
{   -INF, <0      NaN      }
{   0             -INF (correct for log; changed to 0 by sqrt) }
{   +INF          +INF     }

      %pd_unpk(arg, u);
      %_dck1 := true;
      if u.sgn < 0 then begin  {-NaN, -INF}
          if (u.knd = 5)
              then  result := arg          {-NaN}
              else  result := %gen_nan(10); {-INF -> NaN #10}
      end else
          case u.knd of
              1:     result := -%gen_inf;   {0 -> -INF}
              2,3:   %_dck1 := false;      {ok}
              4,5:   result := arg;        {+INF, +NaN}
          end;
end {%_dck1};

FUNCTION %_dck2(arg: double;  limit: double;  outside: types;
                var result: double) : boolean;
{        ------  }
begin
{ Do range checking on most double transcendentals. }
{ If abs(argument) > limit then result is to be of type "outside". }

      if IsNan(arg) then begin
          %_dck2 := true;
          result := arg;   {NaN -> NaN}
      end else if IsInf(arg) or (abs(arg) > limit) then begin
          %_dck2 := true;
          case outside of
              zero:    result := 0d0;    {atan/tanh turn this into pi/2 or 1}
             {2,3 not used}
              inf:     result := %gen_inf;   {exp, sinh, cosh}
              NaN:     result := %gen_nan(11);
                               {NaN #11 for sin/cos/tan/asin/acos}
          end;
      end else
          %_dck2 := false;
end {%_dck2};

{--------------------------------------------------------------UNP }

PROCEDURE %_dunp(value: double;
                 var mant: double;  var expon: integer);
{            --- }
var u: unp_real;

begin
{  Routine to unpack a double precision number, returning a signed }
{  mantissa in the range (-2,-1] or [1,2), and an exponent in the }
{  range [-1024,+1023] }

      %pd_unpk(value, u);
      expon := u.exp + 52;
      u.exp := -52;
      %pd_pack(mant, u);
end;
{--------------------------------------------------------------LGB }

FUNCTION %_dlgb(value: double ): integer;
{                  --- }
var u: unp_real;

begin
{ Routine to fetch the exponent of a real number. }
{ If abs(value) is in [1,2), the exponent is zero. }

      %pd_unpk(value, u);
      %_dlgb := u.exp + 52;
end;
{--------------------------------------------------------------PCK }

PROCEDURE %_dpck(value: double;  power2: integer;  var result: double);
{            --- }
var u: unp_real;

begin
{  Routine to multiply a double precision number by a power of two. }
{  If the parameters are the output of "%_dunp", }
{  the original value is recreated. }

      %pd_unpk(value, u);
      u.exp := u.exp + power2;
      %pd_pack(result, u);
end;
{-------------------------------------------------------------- }
{ Double precision internal routines for Pascal. }

FUNCTION  %_epre(arg: double;  var result: double;
                               var mult: integer): boolean;
{            ---                        (PI-reduce, for trigs) }
const prec=6; halfsz=23;
      maxin=2147483647d0;
var
    lower,rlower : double;
    cutoff,mod4 : integer;
    rtemp,upper,dmult : double;
    i: integer;
    sngl_temp: real;
    pix: array [1..prec] of overlaid;
        {this is used to assure perfect conversion of these numbers}
     
begin
{  Range reduction for SIN, COS, and TAN }

      if not %_dck2(arg, maxin, NaN, result) then begin
          if (abs(arg) < 0.7854d0) then begin
              result := arg;
              mult := 0;
          end else begin
              
 pix[1].i.u:=1071931184;pix[1].i.l:=1073741824;{0.636619687080383301}   
 pix[2].i.u:=1047979236;pix[2].i.l:=1073741824;{8.528719774858473100E-08}
 pix[3].i.u:=1018505720;pix[3].i.l:=0         ;{2.937086744604286430E-16}
 pix[4].i.u:= 999333215;pix[4].i.l:=1073741824;{8.795767286342583120E-22}
 pix[5].i.u:= 973034316;pix[5].i.l:=0         ;{2.471136637518957480E-29}
 pix[6].i.u:= 951959771;pix[6].i.l:=1073741824;{2.238392930622293410E-35}

              sngl_temp := arg;
              upper := sngl_temp;
              lower := arg - upper;
              result := pix[1].d * upper;
              cutoff := %_dlgb(result) - (halfsz - 2);
              dmult := %d_dnint(result);
              result := result - dmult;
              mult := trunc(dmult - %d_dint(dmult/4d0)*4d0);
              rlower := 0d0;
              i := 1;
              while (%_dlgb(result) < cutoff) and
                    (i+2  < prec) do begin
                  rlower := rlower +
                       (lower*pix[i].d + upper*pix[i+1].d);
                  rtemp := result + rlower;
                  if (%_dlgb(rtemp) >= -1) then begin
                      dmult := %d_dnint(rtemp);
                      result := result - dmult;
                      mod4 := trunc(dmult - %d_dint(dmult/4d0)*4d0);
                      mult := mult + mod4;
                      rtemp := result + rlower;
                  end;
                  rlower := (result - rtemp) + rlower;
                  result := rtemp;
                  cutoff := cutoff - halfsz;
                  i := i+1;
              end;

              result := 1.57079632679489662d0 *
                (((lower*pix[i].d +
                   arg*(pix[i+1].d+pix[i+2].d))+rlower)+result);
          end;
          %_epre := false;                 {--EPRE}
      end else %_epre := true;             {--EPRE}
end;

function %_esnc(arg: double; sncsfg: integer): double;
{           ---                          (DSIN, DCOS) }
  var x,xsq,result: double;
      mult: integer;
begin
if %_epre(arg,x,mult)
then %_esnc := x
else begin
     xsq := x*x;
     mult := mult + sncsfg;
     if (not odd(mult)) then begin
         result := x +
                 (((  -26.602364754831d0 ) * xsq
                    +2655.887586118500d0 ) * xsq
                  -102927.006324300873d0 ) * xsq * x /
             ((( xsq +171.560104870728d0 ) * xsq
                   +14942.776380617605d0 ) * xsq
                  +617562.037945803750d0 )  ;

     end else begin
         result := 1d0 +
               (((   -113.318642056143d0 ) * xsq
                    +8156.634106062833d0 ) * xsq
                  -151458.564258496326d0 ) * xsq /
             ((( xsq +129.352967154569d0 ) * xsq
                    +8929.825831108354d0 ) * xsq
                  +302917.128516986641d0 )  ;
         mult := mult - 1;
     end;
     if (odd(mult div 2))
     then %_esnc := -result
     else %_esnc := result;
     end;
end; {%_esnc}

PROCEDURE %_eere(arg: double;  var p,q: double;  var mult: integer);
{            ---                     (E-reduce, for DEXP, DxxxH) }
const p2=0.02309432127295d0;
      p1=20.20170000695313d0;
      p0=1513.86417304653562d0;
      q1=233.17823205143104d0;
      q0=4368.08867006741699d0;
    { log2_u = 1.44269490242004395d0; }
      log2_l = 1.384689194620474d-07;
var
   upper,lower,x,xsq: double;
   sngl_temp: real;
   longmult: longint;
   log2_u: overlaid;

begin

{  Range reduction for EXP, SINH, COSH }

      log2_u.i.u := 1073157447; log2_u.i.l := 1073741824;
      sngl_temp := arg;
      upper := sngl_temp;
      lower := arg - upper;
      x := upper * log2_u.d;
      longmult := round(x);
      if abs(longmult) < 32000
          then mult := longmult
          else if longmult < 0
              then mult := -32000
              else mult :=  32000;
      x := (x - longmult) + (lower*log2_u.d + arg*log2_l);
      xsq := x*x;
      p := x * (p0 + xsq*(p1 + xsq*p2));
      q :=      q0 + xsq*(q1 + xsq);
end;

{ end of internal routines for double precision Pascal functions. }

{ Double precision functions for Pascal. }

FUNCTION %_dsqt{(var arg: double ) : double };
{                           ---       (DSQRT) }
var
    mant,approx,result: double;
    expon  : integer;

begin
      if %_dck1(arg, result) then begin
          if (arg = 0d0) then  result := arg;   {preserve sign of 0}
      end else begin
          %_dunp(arg,mant,expon);
          if (odd(expon)) then begin
              mant := mant * 0.5d0;
              expon := expon + 1;
              approx := 0.5901621 * mant + 0.4173076;
          end else begin
              approx := 0.4173076 * mant + 0.5901621;
          end;
          approx := approx + (mant/approx);
          approx := approx + (4.0*mant/approx);
          approx := approx + (16.0*mant/approx);
          %_dpck(approx, (expon div 2) - 3, result);
      end;
      %_dsqt := result;
end;

FUNCTION %_dsin{(var arg: double ) : double };
{                           ---       (DSIN) }
begin
%_dsin := %_esnc(arg,0);
end;

FUNCTION %_dcos{(var arg: double ) : double };
{                           ---       (DCOS) }
begin
%_dcos := %_esnc(arg,1);
end;

FUNCTION %_datn{(var arg: double ) : double };
{                           ---       (DATAN) }
var
   x,xsq,quad,result: double;
   exp: integer;
   reducd   : boolean;

begin
      if %_dck2(arg, 9d15, zero, result) then begin
          if (result = 0d0) then result := pi_2;
      end else begin
          x := abs(arg);
          if (x >= 2d0) then begin
              x := -1d0 / x;
              quad := pi_2;
              reducd := true;
          end else if (x > 0.5) then begin
              x := (x-1d0) / (x+1d0);
              quad := pi_4;
              reducd := true;
          end else begin
              reducd := false;
          end;
          xsq := x*x;
          result := x -
                    ((((  0.8435039720139102d0 ) * xsq
                         +9.1090581307487438d0 ) * xsq
                        +22.9266838093349465d0 ) * xsq
                        +15.7625272824626173d0 ) * xsq * x /
               (((( xsq +15.8677865626062825d0 ) * xsq
                        +65.3526282413742890d0 ) * xsq
                        +97.1526005357024121d0 ) * xsq
                        +47.2875818473940949d0 )  ;

          if (reducd) then  result := result + quad;
      end;
      %_datn := %d_sign(result,arg);
end;

FUNCTION %_dexp{(var arg: double ) : double };
{           ---       (DEXP) }
var
    p,q,result : double;
    mult: integer;

begin
      if %_dck2(arg, 710d0, inf, result) then begin
          if (not IsNan(result)) and (arg < 0) then
              result := 0d0;
      end else begin
          %_eere(arg, p, q, mult);
          %_dpck(0.5d0 + p/(q-p), mult+1, result);
      end;
      %_dexp := result;
end;

FUNCTION %_dlog{(var arg: double ) : double };
{           ---       (DLOG) }
const log2=0.693147180559945309d0;
var
    x,z,zsq,result: double;
    n: integer;

begin
      if not %_dck1(arg, result) then begin
          %_dunp(arg,x,n);
          if (x > 1.414213) then begin
              z := (x - 2d0) / (x + 2d0);
              n := n + 1;
          end else begin
              z := (x - 1d0) / (x + 1d0);
          end;

          zsq := z*z;
          result := n*log2 + 2*(z +
                   (((  -0.789563581765763543d0 ) * zsq
                        +4.096073272249690401d0 ) * zsq
                        -4.007937950419181027d0 ) * zsq * z /
               ((( zsq  -8.917132545745744675d0 ) * zsq
                       +19.502508127518754879d0 ) * zsq
                       -12.023813851257571257d0 ))  ;
                             { atanhc rational form }
      end;
      %_dlog := result;
end;

FUNCTION %_dupi {(var db: double;   var ie: longint ): double };
{                          ----          (D**I) }
begin
%_dupi := %_dupiv(db,ie);
end; {%_dupi}

{ Double precision exponentiation. }

function %_dupiv{(db: double; ie: longint): double};
  (*****
  var dp,dvb,dve: double;
  *****)
  var answer: double; Invert: Boolean; bit: longint;
begin
{ double precision raised to integer power. }
      (*****
      case ie of
          -1:  %_DUPIv := 1d0/db;

           1:  %_DUPIv := db;
           2:  %_DUPIv := db*db;
           otherwise: begin
                  dvb := abs(db);
                  dve := ie;
                  dp := %_dupd(dvb, dve);
                  IF (db < 0) and (odd(ie)) then dp :=-dp;
                  %_DUPIv := dp;
           end;
      end;
      *****)
Invert := ie < 0;
ie := abs(ie); answer := 1;
if ie > 0
then begin
     if odd(ie) then answer := db;
     bit := 2;
     while ie >= bit do begin
           db := db*db;
           if (ie and bit) <> 0 then answer := answer*db;
           bit := bit + bit;
           end;
     end;
if Invert then answer := 1 / answer;
%_dupiv := answer;
end; {%_dupiv}

FUNCTION %_dupd {(var db: double;   var de: double ): double };
{                          ----          (D**D) }
const NaN_num = 16;
var
   ub, ue: unp_real;
   prd: double;
   l: longint;

      function %three_way ( nan_if, inf_if: boolean ) : double;
      begin
          if      nan_if then %three_way := %gen_nan(NaN_num)
          else if inf_if then %three_way := %gen_inf
          else                %three_way := 0d0;
      end;

begin
{ double precision raised to double precision power. }
      %pd_unpk(db, ub);
      %pd_unpk(de, ue);
      if (ub.knd = NaN) then                  %_dupd := db        {NaN ** x}
      else if (ue.knd = NaN) then             %_dupd := de        {x ** NaN}
      else if (ub.knd = zero) then
          %_dupd := %three_way (ue.knd = zero, ue.sgn < 0)         {0 **  }
      else if (ub.sgn < 0) then begin                             {-y **  }
           l := trunc(de);
           if l = de
           then %_dupd := %_dupiv(db,l)
           else %_dupd := %gen_nan(NaN_num);
           end
      else if (ub.knd = inf) then
          %_dupd := %three_way (ue.knd in [zero,inf], ue.sgn > 0)  {inf **  }
      else if (ue.knd = inf) then
          %_dupd := %three_way (db = 1d0, (db>1d0)=(ue.sgn>0) )    {  ** inf}
      else    begin
          prd := de * %_dlog(db);
          %_DUPD := %_dexp(prd);
      end;
end;

{ The following are internal procedures for Fortran double precision }
{   transcendental functions. }

PROCEDURE %_easc(var result: double;  arg: double;
                  arcsin: boolean);
{            ---                         (DACOS, DASIN) }
var
   x,xsq,temp: double;
   reducd : boolean;

begin
      if not %_dck2(arg, 1d0, NaN, result) then begin
          x := abs(arg);
          if (x < 0.7071d0) then begin
              reducd := false;
          end else begin
              temp := (1d0-x)*(1d0+x);
              x := %_dsqt(temp);
              reducd := true;
          end;
          xsq := x*x;
          result := x +
                ((((((  0.004751624376434203d0 ) * xsq
                       -0.592413491400993952d0 ) * xsq
                       +5.031785600674824666d0 ) * xsq
                      -14.350406308005313946d0 ) * xsq
                      +16.647746863878756233d0 ) * xsq
                       -6.773695835107418535d0 ) * xsq * x /
            ((((( xsq -13.925210930636176476d0 ) * xsq
                      +63.723096250580563708d0 ) * xsq
                     -128.395097929241385370d0 ) * xsq
                     +118.175459937604538160d0 ) * xsq
                      -40.642175010638165982d0 )  ;
          result := %d_sign(result,arg);
          if (reducd) and (arg < 0) then begin
              if (arcsin) then begin
                  result := -(pi_2 + result);
              end else begin
                  result := pi + result;
              end;
          end else if (reducd = arcsin)  then begin
              result := pi_2 - result;
          end;
      end;
end;

PROCEDURE %_esch(var result: double;  arg: double;
               sinh: boolean);
{            ---                       (DSINH, DCOSH) }
var
    x,xsq,p,q : double;
    mult: integer;

begin

      if %_dck2(arg, 710d0, inf, result) then begin
          if (sinh) then result := %d_sign(result,arg);
      end else begin
          x := abs(arg);
          if (x < 0.75d0) then begin
              xsq := x*x;
              if (sinh) then begin
                  result := arg + arg*
                ((((((  0.000000000162095726d0 ) * xsq
                       +0.000000025050948590d0 ) * xsq
                       +0.000002755732364785d0 ) * xsq
                       +0.000198412698326202d0 ) * xsq
                       +0.008333333333341372d0 ) * xsq
                       +0.166666666666666387d0 ) * xsq ;
              end else begin
                  result := 1.0d0 +
                ((((((  0.000000002110133500d0 ) * xsq
                       +0.000000275556007731d0 ) * xsq
                       +0.000024801593801248d0 ) * xsq
                       +0.001388888887631267d0 ) * xsq
                       +0.041666666666782032d0 ) * xsq
                       +0.499999999999996056d0 ) * xsq ;
              end;
          end else begin
              %_eere(x, p, q, mult);
              x := p / (q-p);
              if (mult < 28) then begin
                  %_dpck(0.5/(x+0.5),-2*mult-1,xsq);
                  if (sinh) then begin
                      x := x-xsq;
                  end else begin
                      x := x+xsq;
                  end;
              end;
              %_dpck(0.5+x, mult, result);
              if (sinh) then result := %d_sign(result,arg);
          end;
      end;
end;
{ end of internal double precision routines for fortran. }

{ The following are the double precision functions called from user }
{   programs. }

FUNCTION %_dtan{(var arg: double ) : double };
{          ----     (DTAN) }
var
    x,xsq,result: double;
    mult: integer;

begin
      if %_epre(arg, x, mult) then
          result := x
      else begin
          xsq := x*x;
          result := x +
                      ((( -0.9640986146520d0 ) * xsq
                         +98.5333590598011d0 ) * xsq
                       -1599.1344569510355d0 ) * xsq * x /
               ((( xsq  -111.9944155426216d0 ) * xsq
                       +2214.5614255216536d0 ) * xsq
                       -4797.4033708531468d0 )  ;
          if (odd(mult)) then  result := -1d0 / result;
      end;
      %_dtan := result;
end;

FUNCTION %_dasn{(var arg: double ) : double };
{          ----     (DASIN) }
var result  : double;
begin
      %_easc(result,arg,true);
      %_dasn := result;
end;

FUNCTION %_dacs{(var arg: double ) : double };
{          ----     (DACOS) }
var result  : double;
begin
      %_easc(result,arg,false);
      %_dacs := result;
end;

FUNCTION %_dat2{(var y, x: double ) : double };
{          ----     (DATAN2) }
var result,quot: double;
begin
	if x = 0 then 
	begin
		if y >= 0 then quot := %gen_inf
		else quot := %gen_neginf;
	end else quot := y/x;
      result := %_datn(quot);
      if (x < 0)
          then  %_dat2 := result + %d_sign(pi,y)
          else  %_dat2 := result;
end;

FUNCTION %_dsnh{(var arg: double ) : double };
{          ----     (DSINH) }
var result  : double;
begin
      %_esch(result,arg,true);
      %_dsnh := result;
end;

FUNCTION %_dcsh{(var arg: double ) : double };
{          ----     (DCOSH) }
var result  : double;
begin
      %_esch(result,arg,false);
      %_dcsh := result;
end;

FUNCTION %_dtnh{(var arg: double ) : double };
{          ----     (DTANH) }
var
    x,xsq,p,q,temp,result: double;
    mult: integer;

begin
      if (%_dck2(arg, 19.1d0, zero, result)) then begin
          if (result = 0d0) then result := 1d0;   { > 19.1 }
      end else begin
          x := abs(arg);
          if (x < 0.75d0) then begin
              xsq := x*x;
              result := x +
                       (((-0.96445016240077d0 ) * xsq
                         -99.41819453693073d0 ) * xsq
                       -1617.38247611662225d0 ) * xsq * x /
                ((( xsq +112.95299835936588d0 ) * xsq
                       +2239.11355495022451d0 ) * xsq
                       +4852.14742834988424d0 )  ;
          end else begin
              %_eere(x+x, p, q, mult);
              %_dpck(q+p, mult, temp);
              result := 1d0 - 2d0*(q-p) / (temp+(q-p));
          end;
      end;
      %_dtnh := %d_sign(result,arg);
end;

FUNCTION %_dl10{(var arg: double ) : double };
{          ----     (DLOG10) }
begin
%_dl10 := %_dlog(arg) / 2.302585092994045684d0;
end;
{ end of double precision functions. }

{ Pascal versions (i.e. value parameters) }

function %_pdatn(x: double): double;
begin
%_pdatn := %_datn(x);
end; {%_pdatn}

function %_pdcos(x: double): double;
begin
%_pdcos := %_esnc(x,1);
end; {%_pdcos}

function %_pdsin(x: double): double;
begin
%_pdsin := %_esnc(x,0);
end; {%_pdsin}

function %_pdexp(x: double): double;
begin
%_pdexp := %_dexp(x);
end; {%_pdexp}

function %_pdln(x: double): double;
begin
%_pdln := %_dlog(x);
end; {%_pdln}

function %_pdsqt(x: double): double;
begin
%_pdsqt := %_dsqt(x);
end; {%_pdsqt}

function %d_dprod(a,b: real): double;
  var d: double;
begin
d := a; %d_dprod := d*b;
end; {%d_dprod}

END.
