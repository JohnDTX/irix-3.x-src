{ File: pasrio.text }
{ Date: 18-Jul-85 }

{$I-}
{$R-}
{$%+}
{$M-}

program pasrio;

type alfa30 = array[1..30] of -128..127;
     
function %_dpwr10(n: integer): double; external;
procedure %_setior(n: integer); external;
function %_sgnbt(val: real): Boolean; external;
function %_dsgnbt(val: double): Boolean; external;

procedure %a_2_f(var a: alfa30; ndigits,nscale: integer; var r: real);
  var i,limit: integer; lr: real; l: longint;
begin
l := 0;
if ndigits < 8 then limit := ndigits else limit := 8;
for i := 1 to limit do
    l := l*10 + a[i] - ord('0');
lr := l;
for i := limit + 1 to ndigits do
    lr := lr*10 + (a[i] - ord('0'));
if nscale < 0
then if nscale < -37
     then lr := (lr/pwroften(-6-nscale))/1e6
     else lr := lr/pwroften(-nscale)
else if nscale > 0
     then lr := lr*pwroften(nscale);
r := lr;
end; {%a_2_f}

procedure %a_2_d(var a: alfa30; ndigits,nscale: integer; var d: double);
  var i,limit: integer; ld: double; l: longint;
begin
l := 0;
if ndigits < 8 then limit := ndigits else limit := 8;
for i := 1 to limit do
    l := l*10 + a[i] - ord('0');
ld := l;
for i := limit + 1 to ndigits do
    ld := ld*10 + (a[i] - ord('0'));
if nscale < 0
then if nscale < -307
     then ld := (ld/%_dpwr10(-16-nscale))/1d16
     else ld := ld/%_dpwr10(-nscale)
else if nscale > 0
     then ld := ld*%_dpwr10(nscale);
d := ld;
end; {%a_2_d}

procedure %d_2_a(val: double; ndigits: integer;
                var a: alfa30; var nscale: integer; var Negative: Boolean);
  var i,exp,truncval,ldigits,addexp,j,index,temp: integer; normval: double;
      sign: char; ZeroFlag: Boolean;
begin
if IsNum(val)
then begin
     Negative := val < 0;
     if Negative then val := -val;
     
     { Compute:  1.0 <= normalized value < 10.0, and exponent. }
     
     exp := 0; normval := val;
     if normval > 10
     then begin
          i := 16;  { Must be a power of 2 }
          repeat
                 if normval > %_dpwr10(i)
                    then begin
                         exp := exp + i;
                         normval := val/%_dpwr10(exp);
                         end
                    else i := i div 2;
          until i = 0;
          end;
     if normval <> 0
     then begin
          if normval < 1
          then begin
               if normval < 1d-307
               then begin val := val*1d16; addexp := -16; end
               else addexp := 0;
               i := 16; { Must be a power of 2 }
               repeat
                      if normval < (1 / %_dpwr10(i - 1))
                      then begin
                           exp := exp - i;
                           normval := val*%_dpwr10(-exp);
                           end
                      else i := i div 2;
               until i = 0;
               exp := exp + addexp;
               end;
          end;
     
     { Round by adding 5 to digit just beyond printing decimal places. }
     
     if ndigits > 16 then ldigits := 16 else ldigits := ndigits;
     normval := normval + 5/%_dpwr10(ldigits);
     
     { Fix normalization if rounding messed it up. }
     
     if normval >= 10
     then begin
          exp := exp  + 1;
          normval := normval/10;
          end;
     
     { Fill in the characters of buf. }
     
     ZeroFlag := TRUE; index := 1; normval := normval*1000;
     for i := (ldigits + 3) div 4 downto 1 do begin
         truncval := trunc(normval);
         if i > 1 then normval := (normval - truncval)*10000;
         for j := 3 downto 0 do begin
             if j + index <= ldigits
             then begin
                  temp := truncval mod 10;
                  a[index+j] := temp + ord('0');
                  if temp <> 0 then ZeroFlag := FALSE;
                  end;
             truncval := truncval div 10;
             end;
         index := index + 4;
         end;
     for i := ldigits + 1 to ndigits do a[i] := ord('0');
     if ZeroFlag then begin exp := 0; Negative := False; end;
     
     nscale := exp;
     end
else begin { not IsNum }
     if IsNan(val)
     then sign := '?'
     else if %_dsgnbt(val)
          then sign := '-'
          else sign := '+';
     for i := 1 to ndigits do a[i] := ord(sign);
     nscale := 0; Negative := sign = '-';
     end;
end; {%d_2_a}

procedure %w_e(var f: text; val: real; len: integer);
  var Negative,ZeroFlag: Boolean;
      normval: real;
      i,exp,truncval,addexp: integer;
      sign: char;
      buf: array[1..7] of char;
begin
if len < 8
then len := 8;
if IsNum(val)
then begin
     len := len - 1;
     Negative := val < 0;
     if Negative then val := -val;
     
     { Compute:  1.0 <= normalized value < 10.0, and exponent. }
     
     exp := 0; normval := val;
     if normval > 10
     then begin
          i := 8;  { Must be a power of 2 }
          repeat
                 if normval > pwroften(i)
                 then begin
                      exp := exp + i;
                      normval := val/pwroften(exp);
                      end
                 else i := i div 2;
          until i = 0;
          end;
     if normval <> 0
     then begin
          if normval < 1
          then begin
               if val < 1e-37
               then begin val := val*1e8; addexp := -8; end
               else addexp := 0;
               i := 8; { Must be a power of 2 }
               repeat
                      if normval < (1 / pwroften(i - 1))
                      then begin
                           exp := exp - i;
                           normval := val*pwroften(-exp);
                           end
                      else i := i div 2;
               until i = 0;
               exp := exp + addexp;
               end;
          end;
     
     { Round by adding 5 to digit just beyond printing decimal places. }
     
     if len > 11
     then normval := normval + 5/pwroften(7)
     else normval := normval + 5/pwroften(len - 5);
     
     { Fix normalization if rounding messed it up. }
     
     if normval >= 10
     then begin
          exp := exp  + 1;
          normval := normval/10;
          end;
     
     { Fill in the characters of buf. }
     
     ZeroFlag := TRUE;
     for i := 1 to 7 do
         begin
         truncval := trunc(normval);
         if (truncval <> 0) and (i <= len - 5) then ZeroFlag := FALSE;
         buf[i] := chr(truncval + ord('0'));
         normval := (normval - truncval)*10;
         end;
     
     if Negative and (not ZeroFlag) then sign := '-' else sign := ' ';
     if ZeroFlag then exp := 0;
     write(f,sign,buf[1],'.');
     for i := 2 to len - 5 do
         if i <= 7 then write(f,buf[i]) else write(f,'0');
     if exp < 0
     then begin sign := '-'; exp := -exp; end
     else sign := '+';
     write(f,'E',sign,chr((exp div 10) + ord('0')),
                     chr((exp mod 10) + ord('0')));
     end
else begin { not IsNum }
     if IsNan(val)
     then sign := '?'
     else if %_sgnbt(val)
          then sign := '-'
          else sign := '+';
     for i := 1 to len do write(f,sign);
     end;
end; {%w_e}

procedure %w_f(var f: text; val: real; totalwidth,fractdigits: integer);
  var Negative,ZeroFlag: Boolean;
      normval: real;
      i,exp,truncval,lower,addexp: integer;
      sign,ch: char;
      buf: array[1..7] of char;
begin
if IsNum(val)
then begin
     Negative := val < 0;
     if Negative then val := -val;
     
     { Compute:  1.0 <= normalized value < 10.0, and exponent. }
     
     exp := 0; normval := val;
     if normval > 10
     then begin
          i := 8;  { Must be a power of 2 }
          repeat
                 if normval > pwroften(i)
                 then begin
                      exp := exp + i;
                      normval := val/pwroften(exp);
                      end
                 else i := i div 2;
          until i = 0;
          end;
     if normval <> 0
     then begin
          if normval < 1
          then begin
               if val < 1e-37
               then begin val := val*1e8; addexp := -8; end
               else addexp := 0;
               i := 8; { Must be a power of 2 }
               repeat
                      if normval < (1 / pwroften(i - 1))
                      then begin
                           exp := exp - i;
                           normval := val*pwroften(-exp);
                           end
                      else i := i div 2;
               until i = 0;
               exp := exp + addexp;
               end;
          end;
     
     { Round by adding 5 to digit just beyond printing decimal places. }
     
     i := exp + 1 + fractdigits;
     if i > 6
     then normval := normval + 5/pwroften(7)
     else if i >= 0
          then normval := normval + 5/pwroften(i)
          else normval := normval + 5*pwroften(-i);
     
     { Fix normalization if rounding messed it up. }
     
     if normval >= 10
     then begin exp := exp + 1; normval := normval/10; end;
     
     { Fill in the characters of buf. }
     
     ZeroFlag := TRUE;
     for i := 1 to 7 do
         begin
         truncval := trunc(normval);
         if (truncval <> 0) and (i <= exp + 1 + fractdigits)
         then ZeroFlag := FALSE;
         buf[i] := chr(truncval + ord('0'));
         normval := (normval - truncval)*10;
         end;
     
     if ZeroFlag then Negative := FALSE;
     
     lower := ord(Negative) + 3 + fractdigits;
     if exp >= 0
     then lower := lower + exp
     else if fractdigits > 0
          then lower := lower - 1;
     for i := lower to totalwidth do
         write(f,' ');
     
     if Negative then write(f,'-');
     for i := 1 to exp + 1 do begin
         if ZeroFlag and (i <> exp + 1)
         then ch := ' '
         else if i <= 7
              then ch := buf[i]
              else ch := '0';
         write(f,ch);
         end;
     {if (fractdigits = 0) and (exp < 0) then write(f,'0');}
     if (exp < 0) then write(f,'0');
     write(f,'.');
     for i := exp + 2 to exp + 1 + fractdigits do begin
         if (i <= 7) and (i >= 1)
         then ch := buf[i]
         else ch := '0';
         write(f,ch);
         end;
     end
else begin { not IsNum }
     if IsNan(val)
     then sign := '?'
     else if %_sgnbt(val)
          then sign := '-'
          else sign := '+';
     for i := 1 to totalwidth do write(f,sign);
     end;
end; {%w_f}

procedure %_rdfp(var f: text; var a: alfa30; var ErrorFlag,Negative: Boolean;
                 var ndigits,exp: integer);
  var lch: char; ExpNeg: Boolean; bias: integer;
begin
Negative := FALSE; ErrorFlag := FALSE; ExpNeg := FALSE;
bias := 0; exp := 0; ndigits := 0;
while f^ = ' ' do begin
      if eof(f) then exit(%_rdfp);
      get(f);
      end;
lch := f^;
if lch = '+'
then begin get(f); lch := f^; end
else if lch = '-'
     then begin
          Negative := TRUE;
          get(f); lch := f^;
          end;
if (lch >= '0') and (lch <= '9')
then repeat
            if ndigits < 30
            then begin
                 ndigits := ndigits + 1;
                 a[ndigits] := ord(lch);
                 end
            else bias := bias + 1;
            get(f); lch := f^;
     until (lch < '0') or (lch > '9')
else ErrorFlag := TRUE;
if lch = '.'
then begin
     get(f); lch := f^;
     if (lch >= '0') and (lch <= '9')
     then repeat
                 if ndigits < 30
                 then begin
                      ndigits := ndigits + 1;
                      a[ndigits] := ord(lch);
                      bias := bias - 1;
                      end;
                 get(f); lch := f^;
          until (lch < '0') or (lch > '9')
     else ErrorFlag := TRUE;
     end;
if not ErrorFlag
then begin
     if (lch = 'E') or (lch = 'e') or (lch = 'D') or (lch = 'd')
     then begin
          get(f); lch := f^;
          if lch = '+'
          then begin get(f); lch := f^; end
          else if lch = '-'
               then begin
                    ExpNeg := TRUE;
                    get(f); lch := f^;
                    end;
          if (lch >= '0') and (lch <= '9')
          then repeat
                      if exp < 1000
                      then exp := exp*10 + ord(lch) - ord('0');
                      get(f); lch := f^;
               until (lch < '0') or (lch > '9')
          else ErrorFlag := TRUE;
          if ExpNeg then exp := -exp;
          end;
     end;
exp := exp + bias;
end; {%_rdfp}

procedure %r_r(var f: text; var fr: real);
  var ErrorFlag,Negative: Boolean; ndigits,exp: integer; a: alfa30;
      r: record case Boolean of
                  FALSE: (r: real);
                  TRUE:  (a: longint);
         end;
begin
%_rdfp(f,a,ErrorFlag,Negative,ndigits,exp);
if ErrorFlag
then begin
     %_setior(14); { Numeric Read Error }
     r.a := $7F80000E; { Return NAN }
     end
else begin
     %a_2_f(a,ndigits,exp,r.r);
     if Negative then r.r := -r.r;
     end;
fr := r.r;
end; {%r_r}

procedure %r_d(var f: text; var fd: double);
  var ErrorFlag,Negative: Boolean; ndigits,exp: integer; a: alfa30;
      d: record case Boolean of
                  FALSE: (d: double);
                  TRUE:  (a: longint; b: longint);
         end;
begin
%_rdfp(f,a,ErrorFlag,Negative,ndigits,exp);
if ErrorFlag
then begin
     %_setior(14); { Numeric Read Error }
     d.a := $7FF00000; d.b := $0000000E; { Return NAN }
     end
else begin
     %a_2_d(a,ndigits,exp,d.d);
     if Negative then d.d := -d.d;
     end;
fd := d.d;
end; {%r_d}

procedure %w_d(var f: text; val: double; flen: integer);
  var a: alfa30; i,exp,ldigits,llen: integer; Negative: Boolean;
begin
if flen < 8 then flen := 8;
llen := flen; if llen > 22 then llen := 22;
ldigits := llen - 7;
%d_2_a(val,ldigits,a,exp,Negative);
if flen > llen then write(f,' ':flen-llen);
if Negative then write(f,'-') else write(f,' ');
write(f,chr(a[1]),'.');
for i := 2 to ldigits do write(f,chr(a[i]));
write(f,'D');
if exp < 0 then write(f,'-') else write(f,'+');
exp := abs(exp);
write(f,exp div 100:1,(exp div 10) mod 10:1,exp mod 10:1);
end; {%w_d}

procedure %w_l(var f: text; val: double; totalwidth,fractdigits: integer);
  var a: alfa30; i,exp,digit,llen: integer; Negative: Boolean;
      lch,zeroch: char;
begin
%d_2_a(val,15,a,exp,Negative);
if fractdigits > 0 then llen := fractdigits + 2 else llen := 1;
if Negative then llen := llen + 1;
if exp >= 0 then llen := llen + exp;
if totalwidth > llen then write(f,' ':totalwidth - llen);
if Negative then write(f,'-');
if a[1] > ord('9') then zeroch := chr(a[1]) else zeroch := '0';
digit := 0;
if exp < 0
then begin
     write(f,zeroch,'.');
     for i := 1 to fractdigits do begin
         exp := exp + 1;
         lch := zeroch;
         if exp >= 0
         then begin
              digit := digit + 1;
              if digit <= 15 then lch := chr(a[digit]);
              end;
         write(f,lch);
         end;
     end
else begin
     llen := exp + 1;
     if fractdigits > 0 then llen := llen + fractdigits;
     for i := 1 to llen do begin
         if exp = -1 then write(f,'.');
         exp := exp - 1; digit := digit + 1;
         if digit <= 15 then lch := chr(a[digit]) else lch := '0';
         write(f,lch);
         end;
     end;
end; {%w_l}

procedure %w_x(var f: text; fval: longint; fsize: integer);
  var i,j: integer; c: packed array[0..7] of 0..15;
begin
if fsize > 8 then begin write(f,' ':fsize-8); fsize := 8; end;
moveleft(fval,c,4);
for i := 8 - fsize to 7 do begin
    if odd(i) then j := c[i - 1] else j := c[i + 1];
    if j > 9 then j := j + 7;
    write(f,chr(j + ord('0')));
    end;
end; {%w_x}

{$C- }
begin {pasrio}
end. {pasrio}

