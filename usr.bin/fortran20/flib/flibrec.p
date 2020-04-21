(*****************************************************************************)
(*                                                                           *)
(*                            File: FLIBREC.TEXT                             *)
(*                                                                           *)
(*           (C) Copyright 1982, 1985 Silicon Valley Software, Inc.          *)
(*                                                                           *)
(*                            All Rights Reserved.               10-May-85   *)
(*                                                                           *)
(*****************************************************************************)


{$%+} {$R-} {$I-}

unit %flibrec;

interface

uses {$u flibinit} %flibinit;

procedure %putch(fch: char);
procedure %getch(var fch: char);
procedure %nextrec;
procedure %foutputrecbuf;
procedure %finputrecbuf;
procedure %termiostmt(fiostat: plongint; errexit: pcodearray);
procedure %putblanks(count: integer);
procedure %putstars;
procedure %fillfield(fch: char);

implementation

procedure %_fwrite(count: integer; buff: plongint; fib: ptext); external;
procedure %_fread(buff: plongint; fib: ptext; var count: integer); external;
procedure %_fseek(fib: ptext; targetpos,sense: longint); external;

procedure %putch{*fch: char*};
begin {%putch}
if ENVIRONMENT = CPM
then
  if recbufp < (MAXRECLEN - 1) { Must leave room for final <cr> }
  then begin
    lastwritten := recbufp;
    recbuf[recbufp] := ord(fch); recbufp := recbufp + 1;
    end
  else %error(82)
else 
  if recbufp < MAXRECLEN { Must leave room for final <cr> }
  then begin
    lastwritten := recbufp;
    recbuf[recbufp] := ord(fch); recbufp := recbufp + 1;
    end
  else %error(82);
end; {%putch}

procedure %getch{*var fch: char*};
begin {%getch}
if recbufp <= recbuflen then fch := chr(recbuf[recbufp]) else fch := ' ';
recbufp := recbufp + 1; col := col + 1;
end; {%getch}

procedure %nextrec;
  var i: integer; inline: string[255];
begin {%nextrec}
if InternalIO
then 
  if Reading
  then begin { Reading from internal file }
    moveleft(intfile^[intbufp],recbuf,intreclen);
    intbufp := intbufp + intreclen; recbuflen := intreclen;
    end
  else begin { Writing to internal file }
    if lastwritten > maxlastwritten then maxlastwritten := lastwritten;
    while maxlastwritten < intreclen do begin
      maxlastwritten := maxlastwritten + 1;
      recbuf[maxlastwritten] := 32; { blank }
      end;
    if maxlastwritten > intreclen then %error(83);
    moveleft(recbuf,intfile^[intbufp],intreclen);
    intbufp := intbufp + intreclen;
    lastwritten := 0; maxlastwritten := 0;
    end
else
  if Reading
  then 
    if curunit = consoleunit
    then begin { Reading the console }
      readln(inline); recbuflen := length(inline);
      for i := 1 to recbuflen do
        recbuf[i] := ord(inline[i]);
      if (recbuflen = 0) and eof then EofFlag := TRUE;
      end
    else begin { Reading external, other than consoleunit }
      %finputrecbuf;
      if curunit <> nil
      then begin
        if (not curunit^.Seqen) and (recbuflen <> curunit^.reclen)
        then %error(84);
        end;
      end
  else begin { Writing external }
    if lastwritten > maxlastwritten then maxlastwritten := lastwritten;
    if curunit <> nil
    then
      if (curunit^.Formted = PRINTER) and (maxlastwritten >= 1)
      then begin
        if recbuf[1] = ord('1')
        then recbuf[1] := 12 {^L - formfeed}
        else
          if recbuf[1] = ord('+')
          then recbuf[1] := 11 {^K - upcursor}
          else
            if recbuf[1] = ord('0')
            then
              if ENVIRONMENT = CPM
              then begin
                moveright(recbuf[1],recbuf[2],maxlastwritten);
                maxlastwritten := maxlastwritten + 1;
                recbuf[1] := 13 {<cr>}; recbuf[2] := ord(EOLCH);
                end
              else recbuf[1] := ord(EOLCH)
            else begin {Presumably blank}
              moveleft(recbuf[2],recbuf[1],maxlastwritten-1);
              maxlastwritten := maxlastwritten - 1;
              end;
        end;
    if curunit <> nil
    then
      if not curunit^.Seqen
      then begin
        while maxlastwritten < curunit^.reclen do begin
          maxlastwritten := maxlastwritten + 1;
          recbuf[maxlastwritten] := ord(' ');
          end;
        if maxlastwritten > curunit^.reclen
        then begin %error(85); maxlastwritten := curunit^.reclen; end;
        end;
    if ENVIRONMENT = CPM
    then begin
      maxlastwritten := maxlastwritten + 1;
      recbuf[maxlastwritten] := 13 {<cr>};
      end;
    maxlastwritten := maxlastwritten + 1;
    recbuf[maxlastwritten] := ord(EOLCH);
    %foutputrecbuf;
    lastwritten := 0; maxlastwritten := 0;
    end;
recbufp := 1;
end; {%nextrec}

procedure %foutputrecbuf;
  var i, nummoved: integer;
begin {%foutputrecbuf}
if curunit <> nil
then
  with curunit^ do
    if ENVIRONMENT = MERLIN
    then
      if osfib^.FIsBlocked
      then { FIsBlocked }
        if (osfib^.fnextbyte + maxlastwritten) > FBLKSIZE
        then { no room in existing buffer }
          if odd(osfib^.fnextblock)
          then begin { Even block, fill it up and move on to next }
            nummoved := FBLKSIZE - osfib^.fnextbyte;
            moveleft(recbuf,osfib^.fbuffer[osfib^.fnextbyte],
                     FBLKSIZE - osfib^.fnextbyte);
            %pageoutfbuffer; %pageinfbuffer(osfib^.fnextblock);
            moveleft(recbuf[nummoved+1],osfib^.fbuffer,
                     maxlastwritten - nummoved);
            osfib^.fnextbyte := maxlastwritten - nummoved; 
            osfib^.FBufChanged := TRUE;
            end
          else begin { Finishing odd block, zero pad and go on }
            for i := osfib^.fnextbyte to FBLKSIZE - 1 do
              osfib^.fbuffer[i] := 0;
            if osfib^.FBufChanged then %pageoutfbuffer; 
            %pageinfbuffer(osfib^.fnextblock);
            moveleft(recbuf,osfib^.fbuffer,maxlastwritten);
            osfib^.fnextbyte := maxlastwritten; osfib^.FBufChanged := TRUE;
            end
        else begin { Room in fbuffer }
          moveleft(recbuf,osfib^.fbuffer[osfib^.fnextbyte],maxlastwritten);
          osfib^.fnextbyte := osfib^.fnextbyte + maxlastwritten; 
          osfib^.FBufChanged := TRUE;
          end
      else { not FIsBlocked }
        unitwrite(osfib^.funit,recbuf,maxlastwritten)
    else begin { ENVIRONMENT <> MERLIN }
      %_fwrite(maxlastwritten,@recbuf,ostextfile);
      if ioresult <> 0 then %error(48);
      end;
end; {%foutputrecbuf}


{ For unitread input, %finputrecbuf expects the eof indicator to be }
{ after completed lines.                                            }

procedure %finputrecbuf;
  var lint1: int1; Done: Boolean; count: integer;
  
  { Getrawch automatically skips forward off ends of odd blocks }
  
  procedure %getrawch;
  begin {%getrawch}
  if ENVIRONMENT = MERLIN
  then
    with curunit^ do begin
      if osfib^.fnextbyte >= FBLKSIZE
      then begin 
        if osfib^.FBufChanged then %pageoutfbuffer;
        %pageinfbuffer(osfib^.fnextblock);
        end;
      lint1 := osfib^.fbuffer[osfib^.fnextbyte]; 
      osfib^.fnextbyte := osfib^.fnextbyte + 1;
      end;
  end; {%getrawch}
  
begin {%finputrecbuf}
recbuflen := 0;
if (curunit <> nil) and (not EofFlag)
then
  with curunit^ do
    if ENVIRONMENT = MERLIN
    then
      if osfib^.FIsBlocked
      then begin { FIsBlocked }
        %getrawch;
        if (lint1 = 0) and (osfib^.fnextblock < osfib^.fmaxblock)
        then begin
          if osfib^.FBufChanged then %pageoutfbuffer;
          %pageinfbuffer(osfib^.fnextblock);
          %getrawch;
          end;
        if lint1 = 0
        then EofFlag := TRUE
        else begin
          if lint1 = 16
          then begin
            if not Seqen then %error(68); { DLE not allowed in direct files }
            %getrawch;
            while lint1 > 32 do begin
              recbuflen := recbuflen + 1; recbuf[recbuflen] := 32;
              lint1 := lint1 - 1;
              end;
            %getrawch;
            end;
          while lint1 <> 13 do begin
            if recbuflen < MAXRECLEN
            then begin
              recbuflen := recbuflen + 1; recbuf[recbuflen] := lint1;
              end;
            %getrawch;
            end;
          end;
        end
      else begin { not FIsBlocked }
        Done := FALSE;
        unitread(osfib^.funit,lint1,1);
        if lint1 = 13 {<cr>}
        then Done := TRUE
        else 
          if lint1 = 3 {<ctrl-c>, end of file indicator}
          then begin EofFlag := TRUE; Done := TRUE; end
          else 
            if lint1 = 16 {DLE}
            then begin
              if not Seqen then %error(68); { DLE not allowed in direct files }
              unitread(osfib^.funit,lint1,1);
              while lint1 > 32 do begin
                recbuflen := recbuflen + 1; recbuf[recbuflen] := 32;
                lint1 := lint1 - 1;
                end;
              end
            else begin recbuflen := 1; recbuf[1] := lint1; end;
        while not Done do begin
          unitread(osfib^.funit,lint1,1);
          if lint1 = 13 {<cr>}
          then Done := TRUE
          else 
            if recbuflen < MAXRECLEN
            then begin
              recbuflen := recbuflen + 1; recbuf[recbuflen] := lint1;
              end;
          end;
        end { not FIsBlocked }
    else begin { ENVIRONMENT <> MERLIN }
      Done := FALSE;
      %_fread(@lint1,ostextfile,count);
      if ioresult <> 0 then %error(49);
      if (ENVIRONMENT = CPM) and (lint1 = 26 {<ctrl-Z>})
      then begin
        {Discovered text file eof, set filesiz accurately.}
        {Insure next read will once again be ctrl-Z.      }
        %_fseek(ostextfile,-1,SEEK_RELATIVE);
        {Has side effect of setting filepos accurately}
        idosfib^.filesiz := idosfib^.filepos;
        end;
      if ((ENVIRONMENT = CPM) and (lint1 = 26 {<ctrl-Z>})) or
         (count = 0) {Zero read count end of file}
      then begin EofFlag := TRUE; Done := TRUE; end
      else
        if lint1 = ord(EOLCH)
        then Done := TRUE
        else begin recbuflen := 1; recbuf[1] := lint1; end;
      while not Done do begin
        if (ENVIRONMENT = CROMIX) or (ENVIRONMENT = CPM)
        then
          if lint1 = $0d then recbuflen := recbuflen - 1;
        %_fread(@lint1,ostextfile,count);
        if ioresult <> 0 then %error(49);
        if (lint1 = ord(EOLCH)) or 
           (count = 0) or 
           ((ENVIRONMENT = CPM) and (lint1 = 26 {<ctrl-Z>}))
        then Done := TRUE
        else 
          if recbuflen < MAXRECLEN
          then begin
            recbuflen := recbuflen + 1; recbuf[recbuflen] := lint1;
            end;
        end;
      end;
end; {%finputrecbuf}


{ Set fiostat based on whether an error has occurred.  Exit          }
{ to user FORTRAN program at location errexit by patching return     }
{ location of calling procedure if there has been a run time error.  }
{ If run time error has occurred and the user failed to trap it then }
{ abort program with a run time error and routine does not return.   }

procedure %termiostmt{*fiostat: plongint; errexit: pcodearray*};
begin {%termiostmt}
if fiostat <> nil then fiostat^ := errornumber;
if errornumber <> 0
then begin
  if errexit[1] <> nil
  then
    if ENVIRONMENT = GENIX
    then errexit[-2]^[2] := errexit[1]
    else errexit[-1]^[2] := errexit[1]
  else
    if fiostat = nil
    then { User failed to trap error using errexit or iostat }
      if ENVIRONMENT = GENIX
      then %_ferror(ord(errexit[-2]^[2]),errornumber)
      else %_ferror(ord(errexit[-1]^[2]),errornumber);
  end;
end; {%termiostmt}

procedure %fillfield{*fch: char*};
  var i: integer;
begin {%fillfield}
for i := 1 to edw do
  %putch(fch);
end; {%fillfield}

procedure %putstars;
begin {%putstars}
%fillfield('*');
end; {%putstars}

procedure %putblanks{*count: integer*};
  var i: integer;
begin {%putblanks}
for i := 1 to count do
  %putch(' ');
end; {%putblanks}

end. {%flibrec}

                                                                                                                                                                        