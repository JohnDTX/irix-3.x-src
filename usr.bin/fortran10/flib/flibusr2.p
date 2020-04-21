(*****************************************************************************)
(*                                                                           *)
(*                            File: FLIBUSR.TEXT                             *)
(*                                                                           *)
(*           (C) Copyright 1982, 1985 Silicon Valley Software, Inc.          *)
(*                                                                           *)
(*                            All Rights Reserved.               21-Aug-85   *)
(*                                                                           *)
(*****************************************************************************)


{$%+} {$R-} {$I-} {$E-}

unit %flibus2;

interface

uses {$u flibinit} %flibinit;

function iargc: longint;
procedure getarg(var farg: longint; t: ppac; len: longint);
function peek(var loc: longint): byte;
procedure poke(var loc: longint; var val: byte);
function ifd(var unitnum: longint): longint;
procedure vers;
function ran(var fa: longint): real;

implementation

function iargc{*: longint*};
begin {iargc}
iargc := argc;
end; {iargc}

procedure getarg{*var farg: longint; t: ppac; len: longint*};
  var i: integer;
begin {getarg}
for i := 1 to len do t^[i] := ' ';
if farg <= argc
then
  for i := 1 to length(argv[farg]^) do
    if i <= len then t^[i] := argv[farg]^[i];
end; {getarg}

function peek{*var loc: longint): byte*};
  var p: ^byte;
begin {peek}
p := pointer(loc); peek := p^;
end; {peek}

procedure poke{*var loc: longint; var val: byte*};
  var p: ^byte;
begin {poke}
p := pointer(loc); p^ := val;
end; {poke}

function ifd{*var unitnum: longint): longint*};
begin {ifd}
ifd := 0;
curunit := %findunit(unitnum);
if (ENVIRONMENT = IDRIS) or
   (ENVIRONMENT = UNISOFT) or
   (ENVIRONMENT = UNOS) or
   (ENVIRONMENT = REGULUS) or
   (ENVIRONMENT = CPM) or
   (ENVIRONMENT = ADVENTURE)
then begin
  if curunit <> nil then ifd := curunit^.idosfib^.fd;
  end
else
  if ENVIRONMENT = CROMIX
  then begin
    if curunit <> nil then ifd := curunit^.crosfib^.fd;
    end
  else
    if (ENVIRONMENT = ELITE) or (ENVIRONMENT = GENIX) or
       (ENVIRONMENT = TEK)
    then begin
      if curunit <> nil then ifd := curunit^.eltpfib^.fd;
      end;
end; {ifd}

procedure vers;
begin
writeln('SVS FORTRAN 77 Run Time Library Version ',VERSION,' of ',DATE);
end; {vers}

function ran{*var fa: longint): real*};
  const A = 16807; P = 2147483647; B15 = 32768; B16 = 65536;
  var xhi,xalo,leftlo,fhi,k: longint; r: array[0..3] of byte;
begin
if fa < 0 then ranseed := ranseed*171771;
if fa <> 0
then begin
     xhi := ranseed div B16;
     xalo := (ranseed - xhi*B16)*A;
     leftlo := xalo div B16;
     fhi := ranseed*A + leftlo;
     k := fhi div B15;
     ranseed := (((xalo - leftlo*B16) - P) + (fhi - k*B15)*B16) + k;
     k := (ranseed and $007fffff) or $3f800000;
     moveleft(k,lastran,4);
     lastran := lastran - 1.0;
     end;
ran := lastran;
end; {ran}

end. {%flibus2}

