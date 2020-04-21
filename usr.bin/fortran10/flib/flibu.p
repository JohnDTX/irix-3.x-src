(*****************************************************************************)
(*                                                                           *)
(*                             File: FLIBU.TEXT                              *)
(*                                                                           *)
(*           (C) Copyright 1982, 1985 Silicon Valley Software, Inc.          *)
(*                                                                           *)
(*                            All Rights Reserved.               16-Jun-85   *)
(*                                                                           *)
(*****************************************************************************)


{$%+} {$R-} {$I-}

unit %flibu;

interface

uses {$u flibinit} %flibinit,
     {$u flibrec}  %flibrec;

implementation

procedure %_fwrite(count: integer; buff: pbyte; fib: ptext); external;
procedure %_fread(buff: pbyte; fib: ptext; var count: integer); external;
function %_freadn(buff: pbyte; fib: ptext; count: longint): longint; external;

{ For UNFORMATTED files, the following conventions are used: }
{ ESC ESC => ESC, ESC FEOR => EOR, EOR is true end of record }

procedure %luput(fval: int1); {Never called with curunit = nil}
  var i, nummoved: integer;
begin {%luput}
reclenctr := reclenctr + 1;
with curunit^ do
  if ENVIRONMENT = MERLIN
  then
    if osfib^.FIsBlocked
    then begin { FIsBlocked }
      if osfib^.fnextbyte >= FBLKSIZE { no room in existing buffer }
      then begin 
        if osfib^.FBufChanged then %pageoutfbuffer; 
        %pageinfbuffer(osfib^.fnextblock); 
        end;
      osfib^.fbuffer[osfib^.fnextbyte] := fval; 
      osfib^.fnextbyte := osfib^.fnextbyte + 1; osfib^.FBufChanged := TRUE;
      end
    else { not FIsBlocked }
      unitwrite(osfib^.funit,fval,1)
  else begin {ENVIRONMENT <> MERLIN}
    %_fwrite(1,@fval,ostextfile);
    if ioresult <> 0 then %error(48);
    end;
end; {%luput}

procedure %uputn(ptr: pbyte; n: longint);
  var i: longint; last: integer; larr: array[1..4096] of byte;

  procedure %flushlarr;
  begin {%flushlarr}
  if ENVIRONMENT <> MERLIN
  then begin
    if last <> 0
    then begin
      %_fwrite(last,@larr,curunit^.ostextfile);
      if ioresult <> 0 then %error(48);
      last := 0;
      end;
    end;
  end; {%flushlarr}

begin {%uputn}
if curunit = nil
then reclenctr := reclenctr + n
else
  if curunit^.Formted = UNFORMATTED
  then
    if ENVIRONMENT = MERLIN
    then
      for i := 1 to n do begin
        if ptr^ = ESCBYTE
        then begin %luput(ESCBYTE); %luput(ESCBYTE); end
        else
          if ptr^ = EORBYTE
          then begin %luput(ESCBYTE); %luput(FEORBYTE); end
          else %luput(ptr^);
        ptr := pointer(ord(ptr) + 1);
        end
    else begin {not MERLIN}
      reclenctr := reclenctr + n;
      last := 0;
      for i := 1 to n do begin
        if ptr^ = ESCBYTE
        then begin
          if last = 4096 then %flushlarr;
          last := last + 1; larr[last] := ESCBYTE;
          if last = 4096 then %flushlarr;
          last := last + 1; larr[last] := ESCBYTE;
          end
        else
          if ptr^ = EORBYTE
          then begin
            if last = 4096 then %flushlarr;
            last := last + 1; larr[last] := ESCBYTE;
            if last = 4096 then %flushlarr;
            last := last + 1; larr[last] := FEORBYTE;
            end
          else begin
            if last = 4096 then %flushlarr;
            last := last + 1; larr[last] := ptr^;
            end;
        ptr := pointer(ord(ptr) + 1);
        end;
      %flushlarr;
      end
  else {BINARY}
    if ENVIRONMENT = MERLIN
    then
      for i := 1 to n do begin
        %luput(ptr^); ptr := pointer(ord(ptr) + 1);
        end
    else begin
      reclenctr := reclenctr + n;
      while n >= 32256 do begin
        %_fwrite(32256,ptr,curunit^.ostextfile);
        if ioresult <> 0 then %error(48);
        n := n - 32256; ptr := pointer(ord(ptr) + 32256);
        end;
      if n <> 0
      then begin
        %_fwrite(n,ptr,curunit^.ostextfile);
        if ioresult <> 0 then %error(48);
        end;
      end;
end; {%uputn}


procedure %luget(var fval: int1);
  var count: integer;
begin {%luget}
reclenctr := reclenctr + 1;
if curunit <> nil
then
  with curunit^ do begin
    if ENVIRONMENT = MERLIN
    then
      if osfib^.FIsBlocked
      then
        if (osfib^.fnextbyte < FBLKSIZE) and
          ((osfib^.fnextbyte < osfib^.fheader.dlastbyte) or 
           (osfib^.fnextblock < osfib^.fmaxblock))
        then begin { Character in fbuffer }
          fval := osfib^.fbuffer[osfib^.fnextbyte]; 
          osfib^.fnextbyte := osfib^.fnextbyte + 1;
          end
        else { No more characters in fbuffer }
          if osfib^.fnextblock >= osfib^.fmaxblock
          then begin EofFlag := TRUE; fval := 0; end
          else begin { Pick up next block }
            if osfib^.FBufChanged then %pageoutfbuffer;
            %pageinfbuffer(osfib^.fnextblock);
            fval := osfib^.fbuffer[osfib^.fnextbyte]; 
            osfib^.fnextbyte := osfib^.fnextbyte + 1;
            end
      else unitread(osfib^.funit,fval,1)
    else begin {ENVIRONMENT <> MERLIN}
      %_fread(@fval,ostextfile,count);
      if count = 0
      then begin
        if ioresult <> 0 then %error(49);
        EofFlag := TRUE; fval := 0;
        end;
      end;
    end;
end; {%luget}

procedure %ugetn(ptr: pbyte; n: longint);
  var i: longint; count: integer;
      next, last: integer; larr: array[1..4096] of byte;

  procedure %filllarr;
    var amountwanted: integer;
  begin {%filllarr}
  if ENVIRONMENT <> MERLIN
  then begin
    if n <= 4096 then amountwanted := n else amountwanted := 4096;
    last := %_freadn(@larr,curunit^.ostextfile,amountwanted);
    if last <> amountwanted
    then begin
      if ioresult <> 0 then %error(49);
      EofFlag := TRUE;
      while last < amountwanted do begin
        last := last + 1; larr[last] := 0;
        end;
      end;
    next := 1;
    end;
  end; {%filllarr}

begin {%ugetn}
if curunit = nil
then reclenctr := reclenctr + n
else
  if curunit^.Formted = UNFORMATTED
  then
    if ENVIRONMENT = MERLIN
    then
      for i := 1 to n do begin
        %luget(ptr^);
        if ptr^ = ESCBYTE
        then begin 
          %luget(ptr^); 
          if ptr^ = FEORBYTE then ptr^ := EORBYTE;
          end;
        ptr := pointer(ord(ptr) + 1);
        end
    else begin {not MERLIN}
      reclenctr := reclenctr + n; next := 1; last := 0;
      while n > 0 do begin
        if next > last then %filllarr;
        ptr^ := larr[next]; next := next + 1;
        if ptr^ = ESCBYTE
        then begin 
          if next > last then %filllarr;
          ptr^ := larr[next]; next := next + 1;
          if ptr^ = FEORBYTE then ptr^ := EORBYTE;
          end;
        ptr := pointer(ord(ptr) + 1); n := n - 1;
        end;
      end
  else {BINARY}
    if ENVIRONMENT = MERLIN
    then
      for i := 1 to n do begin
        %luget(ptr^);
        ptr := pointer(ord(ptr) + 1);
        end
    else begin
      reclenctr := reclenctr + n;
      if %_freadn(ptr,curunit^.ostextfile,n) <> n
      then begin
        if ioresult <> 0 then %error(49);
        EofFlag := TRUE;
        end;
      end;
end; {%ugetn}

procedure %unextrec;
  var lval: int1;
begin {%unextrec}
if curunit <> nil
then
  if curunit^.Seqen
  then begin
    if curunit^.Formted = UNFORMATTED
    then
      if Reading 
      then
        repeat
          %luget(lval);
        until (lval = EORBYTE) or EofFlag
      else %luput(EORBYTE);
    end
  else begin {Direct Access}
    lval := 0;
    while reclenctr < curunit^.reclen do
      if Reading then %ugetn(@lval,1) else %uputn(@lval,1);
    if reclenctr <> curunit^.reclen then %error(106);
    end;
end; {%unextrec}

procedure %uposition(rec: longint);
  var targetposition: longint;
begin {%uposition}
if curunit <> nil
then 
  with curunit^ do begin
    rec := rec - 1; { Prefer to work with zero origin indexing }
    if rec < 0 
    then %error(56)
    else
      if Seqen 
      then %error(57)
      else begin
        %updatehighwatermark; targetposition := rec * reclen;
        %sysseek(targetposition);
        end;
    end;
end; {%uposition}


{ Does the work for %_ixuwr and %_ixurd }

procedure %ixu(unitnum: longint);
begin {%ixu}
errornumber := 0; reclenctr := 0;
curunit := %findunit(unitnum);
if curunit = nil
then %error(30)
else begin
  if curunit^.PastEndFile then %error(55);
  if curunit^.Formted <= FORMATTED then %error(47);
  end;
end; {%ixu}


{ Initialize external write or print statement (without seek) }

procedure %_ixuwr(unitnum: longint);
begin {%_ixuwr}
Reading := FALSE; %ixu(unitnum);
end; {%_ixuwr}


{ Initialize external write or print statement (without seek), UNIT=* }

procedure %_ixuwrd;
begin {%_ixuwrd}
%_ixuwr(0);
end; {%_ixuwrd}


{ Initialize external write or print statement (with seek) }

procedure %_pxuwr(unitnum: longint; rec: longint);
begin {%_pxuwr}
Reading := FALSE; %ixu(unitnum); %uposition(rec);
end; {%_pxuwr}


{ Terminate write or print statement }

procedure %_tuwr(fiostat: plongint; errexit: pcodearray);
begin {%_tuwr}
%unextrec; 
if curunit <> nil then curunit^.lastop := WRITEOP;
%termiostmt(fiostat,errexit);
end; {%_tuwr}


{ Terminate write or print statement, no error checking }

procedure %_tuwrd;
  var errexit: pcodearray;
begin {%_tuwrd}
%unextrec; 
if curunit <> nil then curunit^.lastop := WRITEOP;
errexit[1] := nil; %termiostmt(nil,errexit);
end; {%_tuwrd}


{ Initialize external read statement (without seek) }

procedure %_ixurd(unitnum: longint);
begin {%_ixurd}
Reading := TRUE; EofFlag := FALSE; %ixu(unitnum); 
if curunit <> nil
then
  if curunit^.Seqen then %forceeofiflastwritten;
end; {%_ixurd}


{ Initialize external read statement (without seek), UNIT=* }

procedure %_ixurdd;
begin {%_ixurdd}
%_ixurd(0);
end; {%_ixurdd}


{ Initialize external read statement (with seek) }

procedure %_pxurd(unitnum: longint; rec: longint);
begin {%_pxurd}
%_ixurd(unitnum); %uposition(rec);
end; {%_pxurd}


{ Terminate read statement }

procedure %_turd(fiostat: plongint; errexit, endexit: pcodearray);
begin {%_turd}
%unextrec;
if curunit <> nil
then begin
  If curunit^.Seqen and EofFlag then curunit^.PastEndFile := TRUE;
  curunit^.lastop := READOP;
  end;
if EofFlag and (endexit[1] = nil) then %error(-601);
%termiostmt(fiostat,errexit);
if EofFlag and (endexit[1] <> nil) and (errornumber = 0)
then begin 
  if fiostat <> nil then fiostat^ := -1;
  if ENVIRONMENT = GENIX
  then endexit[-1] := endexit[1]
  else endexit[0] := endexit[1];
  end;
end; {%_turd}


procedure %_wru1(fval: int1);
begin {%_wru1}
%uputn(@fval,1); 
end; {%_wru1}

procedure %_wru2(fval: aint12);
begin {%_wru2}
%uputn(@fval,2);
end; {%_wru2}

procedure %_wru4(fval: aint14);
begin {%_wru4}
%uputn(@fval,4);
end; {%_wru4}

procedure %_wru8(fvalmsbytes,fvallsbytes: aint14);
begin {%_wru8}
%uputn(@fvallsbytes,8);
end; {%_wru8}

procedure %_wruch(var fpac: int1array; paclen: longint);
begin {%_wruch}
%uputn(@fpac,paclen);
end; {%_wruch}


procedure %_rdu1(var fval: int1);
begin {%_rdu1}
%ugetn(@fval,1);
end; {%_rdu1}

procedure %_rdu2(var fval: aint12);
begin {%_rdu2}
%ugetn(@fval,2);
end; {%_rdu2}

procedure %_rdu4(var fval: aint14);
begin {%_rdu4}
%ugetn(@fval,4);
end; {%_rdu4}

procedure %_rdu8(var fval: aint18);
  var i: integer;
begin {%_rdu8}
%ugetn(@fval,8);
end; {%_rdu8}

procedure %_rduch(var fpac: int1array; paclen: longint);
  var i: integer;
begin {%_rduch}
%ugetn(@fpac,paclen);
end; {%_rduch}


procedure %_wau8(var fintarray: int1array; count: int4);
begin {%_wau8}
%uputn(@fintarray,count*8);
end; {%_wau8}

procedure %_wau4(var fintarray: int1array; count: int4);
begin {%_wau4}
%uputn(@fintarray,count*4);
end; {%_wau4}

procedure %_wau2(var fintarray: int1array; count: int4);
begin {%_wau2}
%uputn(@fintarray,count+count);
end; {%_wau2}

procedure %_wau1(var fintarray: int1array; count: int4);
begin {%_wau1}
%uputn(@fintarray,count);
end; {%_wau1}

procedure %_wauch(var fpac: int1array; paclen: longint; count: int4);
begin {%_wauch}
%uputn(@fpac,paclen*count);
end; {%_wauch}


procedure %_rau8(var fintarray: int1array; count: int4);
begin {%_rau8}
%ugetn(@fintarray,count*8);
end; {%_rau8}

procedure %_rau4(var fintarray: int1array; count: int4);
begin {%_rau4}
%ugetn(@fintarray,count*4);
end; {%_rau4}

procedure %_rau2(var fintarray: int1array; count: int4);
begin {%_rau2}
%ugetn(@fintarray,count+count);
end; {%_rau2}

procedure %_rau1(var fintarray: int1array; count: int4);
begin {%_rau1}
%ugetn(@fintarray,count);
end; {%_rau1}

procedure %_rauch(var fpac: int1array; paclen: longint; count: int4);
begin {%_rauch}
%ugetn(@fpac,paclen*count);
end; {%_rauch}

end. {%flibu}

