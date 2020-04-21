(*****************************************************************************)
(*                                                                           *)
(*                            File: FLIBBACK.TEXT                            *)
(*                                                                           *)
(*           (C) Copyright 1982, 1985 Silicon Valley Software, Inc.          *)
(*                                                                           *)
(*                            All Rights Reserved.               10-May-85   *)
(*                                                                           *)
(*****************************************************************************)


{$%+} {$R-} {$I-}

unit %flibback;

interface

uses {$U flibinit} %flibinit,
     {$U flibrec}  %flibrec;

implementation

procedure %_fread(buff: plongint; fib: ptext; var count: integer); external;
procedure %_fseek(fib: ptext; targetpos,sense: longint); external;

procedure %_rewd(unitnum: longint; fiostat: plongint; errexit: pcodearray);
begin {%_rewd}
errornumber := 0; 
curunit := %findunit(unitnum);
if curunit <> nil 
then 
  with curunit^ do
    if Seqen 
    then begin
      %forceeofiflastwritten;
      if ENVIRONMENT = MERLIN
      then begin
        if osfib^.FIsBlocked 
        then begin
          if osfib^.FBufChanged then %pageoutfbuffer;
          if osfib^.fheader.fkind = TEXTFILE 
          then osfib^.fnextblock := 2 
          else osfib^.fnextblock := 0;
          osfib^.fnextbyte := FBLKSIZE; osfib^.FBufChanged := FALSE;
          end;
        end
      else { ENVIRONMENT <> MERLIN }
        %sysseek(0);
      lastop := READOP; PastEndFile := FALSE;
      end
    else %error(79);
%termiostmt(fiostat,errexit);
end; {%_rewd}

procedure %_endf(unitnum: longint; fiostat: plongint; errexit: pcodearray);
begin {%_endf}
errornumber := 0; 
curunit := %findunit(unitnum);
if curunit <> nil 
then 
  with curunit^ do
    if Seqen 
    then begin
      lastop := WRITEOP; %forceeofiflastwritten; lastop := READOP;
      PastEndFile := TRUE;
      end
    else %error(80);
%termiostmt(fiostat,errexit);
end; {%_endf}

procedure %_back(unitnum: longint; fiostat: plongint; errexit: pcodearray);
  var leorbyte, lint1: int1;
  
  { Is it possible to back up the file one char }
  
  function %AtStartOfFile: Boolean;
  begin {%AtStartOfFile}
  if ENVIRONMENT = MERLIN
  then
    with curunit^ do
      if osfib^.fheader.fkind = TEXTFILE
      then 
        %AtStartOfFile := ((osfib^.fnextbyte = 0) and (osfib^.fnextblock = 3)) or
                         ((osfib^.fnextbyte = 512) and (osfib^.fnextblock = 2))
      else
        %AtStartOfFile := ((osfib^.fnextbyte = 0) and (osfib^.fnextblock = 1)) or
                         ((osfib^.fnextbyte = 512) and (osfib^.fnextblock = 0));
  end; {%AtStartOfFile}
  
  
  { Peekatprevch returns the character value of the previous character      }
  { written to the file.  It does not change the file curser.               }
  { Under MERLIN, if it is necessary to fetch a previous block to get the   }
  { value, fnextbyte is set to FBLKSIZE (512) to show that the curser is    }
  { off the end of the block.                                               }
  
  function %peekatprevch(useaseorbyte: int1): int1;
    var count: integer; lint1: int1;
  begin {%peekatprevch}
  with curunit^ do
    if ENVIRONMENT = MERLIN
    then
      if %AtStartOfFile
      then %peekatprevch := useaseorbyte
      else begin
        if osfib^.fnextbyte = 0
        then begin { Need to look at previous block }
          if osfib^.FBufChanged then %pageoutfbuffer;
          %pageinfbuffer(osfib^.fnextblock - 2);
          osfib^.fnextbyte := FBLKSIZE;
          if not odd(osfib^.fnextblock)
          then
            while osfib^.fbuffer[osfib^.fnextbyte - 1] = 0 do 
              osfib^.fnextbyte := osfib^.fnextbyte - 1;
          end;
        %peekatprevch := osfib^.fbuffer[osfib^.fnextbyte - 1];
        end
    else begin {ENVIRONMENT <> MERLIN}
      %_fseek(ostextfile,-1,SEEK_RELATIVE);
      if ioresult <> 0
      then begin
        %peekatprevch := useaseorbyte; {AtStartOfFile}
        %sysseek(0);
        end
      else begin
        %_fread(@lint1,ostextfile,count);
        if count = 0 
        then %peekatprevch := useaseorbyte
        else %peekatprevch := lint1;
        end;
      end;
  end; {%peekatprevch}
  
  { Backuponech must not be called except after a call to %peekatprevch. }
  
  procedure %backuponech;
  begin {%backuponech}
  if ENVIRONMENT = MERLIN
  then begin
    if not %AtStartOfFile
    then curunit^.osfib^.fnextbyte := curunit^.osfib^.fnextbyte - 1;
    end
  else begin {ENVIRONMENT <> MERLIN}
    %_fseek(curunit^.ostextfile,-1,SEEK_RELATIVE);
    end;
  end; {%backuponech}
  
begin {%_back}
errornumber := 0; 
curunit := %findunit(unitnum);
if curunit <> nil
then 
  with curunit^ do 
    if Seqen
    then
      if PastEndFile
      then PastEndFile := FALSE
      else
        if ENVIRONMENT = MERLIN
        then
          if osfib^.FIsBlocked and (Formted <= UNFORMATTED)
          then begin { Sequential, not binary, blocked }
            if Formted <= FORMATTED 
            then leorbyte := 13
            else leorbyte := EORBYTE;
            %forceeofiflastwritten;
            lint1 := %peekatprevch(leorbyte); { call to set up %backuponech }
            repeat
              %backuponech;
            until %peekatprevch(leorbyte) = leorbyte; { <CR> }
            lastop := READOP;
            end
          else %error(60)
        else begin {ENVIRONMENT <> MERLIN}
          if Formted <= UNFORMATTED
          then begin { Sequential, formatted or unformatted, not binary }
            if Formted <= FORMATTED 
            then leorbyte := ord(EOLCH) 
            else leorbyte := EORBYTE;
            %forceeofiflastwritten;
            repeat
              %backuponech;
            until %peekatprevch(leorbyte) = leorbyte;
            lastop := READOP;
            end
          else %error(60)
          end
    else %error(81);
%termiostmt(fiostat,errexit);
end; {%_back}

end. {%flibback}

