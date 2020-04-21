(*****************************************************************************)
(*                                                                           *)
(*                            File: FLIBFMT.TEXT                             *)
(*                                                                           *)
(*           (C) Copyright 1982, 1985 Silicon Valley Software, Inc.          *)
(*                                                                           *)
(*                            All Rights Reserved.               10-May-85   *)
(*                                                                           *)
(*****************************************************************************)


{$%+} {$R-} {$I-}

unit %flibfmt;

interface

uses {$u flibinit} %flibinit,
     {$u flibrec}  %flibrec;

procedure %scanfmt;
procedure %getfmt;
procedure %puthex(fval: integer);
procedure %%wrfch(fpac: ppac; paclen: integer);

implementation

{ Sets fmttok and, if appropriate fmtintval }

procedure %scanfmt;
  var lfmtp: integer; fmtch: char;
  
  { Sets fmtch to next char out of the format character string, skips blnks }
  
  procedure %getfmtnonblank;
  begin {%getfmtnonblank}
  repeat
    fmtch := curfmt^[fmtp];
    fmtp := fmtp + 1;
  until fmtch <> ' ';
  if (fmtch >= 'a') and (fmtch <= 'z') then fmtch := chr(ord(fmtch) - 32);
  end; {%getfmtnonblank}
  
begin {%scanfmt}
%getfmtnonblank;
if fmtch in ['I','F','E','D','G','L','A','H','X','P','Z',
                     '(',')',',','.','''',':','/','\\','$','+','-']
then fmttok := fmtch
else
  if fmtch = 'B'
  then begin {Return 'N' for BN, 'B' for BZ}
    %getfmtnonblank;
    if fmtch = 'N'
    then fmttok := 'N'
    else
      if fmtch = 'Z' then fmttok := 'B' else %error(4);
    end
  else
    if fmtch = 'T'
    then begin
      lfmtp := fmtp; %getfmtnonblank;
      if (fmtch = 'L') or (fmtch = 'R')
      then fmttok := chr(ord(fmtch) - 1) { Translate L => K and R => Q }
      else begin fmttok := 'T'; fmtp := lfmtp; end;
      end
    else
      if fmtch = 'S'
      then begin
        lfmtp := fmtp; %getfmtnonblank;
        if (fmtch = 'P') or (fmtch = 'S')
        then fmttok := chr(ord(fmtch) - 1) { Translate P => O and S => R }
        else begin fmttok := 'S'; fmtp := lfmtp; end;
        end
      else
        if (fmtch >= '0') and (fmtch <= '9')
        then begin
          fmttok := '0';  { Stands for integer }
          fmtintval := 0;
          repeat
            fmtintval := fmtintval * 10 + ord(fmtch) - ord('0');
            lfmtp := fmtp; %getfmtnonblank;
          until (fmtch < '0') or (fmtch > '9');
          fmtp := lfmtp;
          end
        else begin 
          { Prevent from looping if previous fmttok was nonrepeatable }
          %error(5); fmttok := 'I';
          end;
end; {%scanfmt}


{ Processes all nonrepeatable format edit descriptors up to the first }
{ repeatable format edit descriptor, leabing it in the global ed...   }
{ Wraps around if fieldfound, prevents infinite loops and gives clean }
{ termination when called under termxxx.                              }

procedure %getfmt;
label 1;
  var i, lint, lfmtp: integer; Repeated, Done: Boolean; lch: char;

  function %getfmtch: char;
  begin {%getfmtch}
  %getfmtch := curfmt^[fmtp]; fmtp := fmtp + 1;
  end; {%getfmtch}

begin {%getfmt}
if edcnt > 1
then edcnt := edcnt - 1
else 
  1: begin
  lint := 1; %scanfmt;
  if fmttok = '0'
  then begin
    lint := fmtintval;
    Repeated := TRUE; %scanfmt;
    end
  else Repeated := FALSE;
  edlet := fmttok; AorZFlag := FALSE;
  if (lint <= 0) and (edlet <> 'P') then %error(6);
  case fmttok of
    'I','F','E','D','G','L': 
         begin { All take manditory W field }
         %scanfmt; if fmttok <> '0' then %error(7);
         edcnt := lint; edw := fmtintval;
         if edw <= 0 then %error(8);
         FieldFound := TRUE; GP2FieldFound := TRUE;
         if (edlet >= 'D') and (edlet <= 'G')
         then begin { Manditory . D fields }
           %scanfmt; if fmttok <> '.' then %error(9);
           %scanfmt; if fmttok <> '0' then %error(10);
           edd := fmtintval; EdePresent := FALSE;
           if (edlet = 'E') or (edlet = 'G') then begin { Optional E e field }
             lfmtp := fmtp; %scanfmt;
             if fmttok = 'E' 
             then begin
               %scanfmt; if fmttok <> '0' then %error(11);
               ede := fmtintval; EdePresent := TRUE;
               if ede <= 0 then %error(12);
               end
             else fmtp := lfmtp;
             end;
           end
         else { 'I' or 'L' }
           if edlet = 'I' then begin { Optional . m field }
             lfmtp := fmtp; %scanfmt; edm := 1; { default }
             if fmttok = '.' 
             then begin
               %scanfmt; if fmttok <> '0' then %error(11); edm := fmtintval;
               end
             else fmtp := lfmtp;
             end;
         end;
    
    'A','Z': begin
         AorZFlag := TRUE; edcnt := lint; lfmtp := fmtp; %scanfmt;
         FieldFound := TRUE; GP2FieldFound := TRUE;
         if fmttok = '0' then begin
           EdwPresent := TRUE; edw := fmtintval;
           if edw <= 0 then %error(13);
           end
         else begin Edwpresent := FALSE; fmtp := lfmtp; end;
         end;
    
    '''':begin
         if Reading then %error(25);
         if Repeated then %error(26);
         Done := FALSE;
         repeat
           lch := %getfmtch; { Note: don't skip blanks }
           while lch <> '''' do begin
             %putch(lch);
             { INLINE lch := %getfmtch; }
             lch := curfmt^[fmtp]; fmtp := fmtp + 1;
             end;
           lch := %getfmtch;
           if lch = ''''
           then %putch('''') { Double ' }
           else begin fmtp := fmtp - 1; Done := TRUE; end;
         until Done;
         goto 1;
         end;
    
    'H': begin
         if Reading then %error(14);
         if not Repeated then %error(15);
         for i := 1 to lint do
           %putch(%getfmtch);
         goto 1;
         end;
    
    'T','K' {'K' is really 'L'},'Q' {'Q' is really 'R'}: 
         begin
         if Repeated then %error(52); 
         %scanfmt; if fmttok <> '0' then %error(53);
         if fmtintval <= 0 
         then %error(53)
         else begin
           if edlet = 'T'
           then recbufp := fmtintval
           else
             if edlet = 'K'
             then begin
               recbufp := recbufp - fmtintval;
               if recbufp < 1 then recbufp := 1;
               end
             else {'Q'} recbufp := recbufp + fmtintval;
           if not Reading
           then begin
             if lastwritten > maxlastwritten then maxlastwritten := lastwritten;
             for i := maxlastwritten + 1 to recbufp - 1 do
               if i <= MAXRECLEN then recbuf[i] := 32 {' '};
             end;
           end;
         goto 1;
         end;
 
    'X': begin
         if not Repeated then %error(16);
         recbufp := recbufp + lint;
         if not Reading
         then begin
           if lastwritten > maxlastwritten then maxlastwritten := lastwritten;
           for i := maxlastwritten + 1 to recbufp - 1 do
             if i <= MAXRECLEN then recbuf[i] := 32 {' '};
           end;
         goto 1;
         end;
    
    '/': begin 
         if Repeated then %error(27); %nextrec; goto 1;
         end;
    
    '\\','$': begin 
         if Repeated then %error(28); InhibitFinalCR := TRUE; goto 1;
         end;
    
    ':': begin 
         if Repeated then %error(28); 
         if not OKToExit then goto 1;
         end;
    
    'S','O' {'O' is really 'P'},'R' {'R' is really 'S'}: 
         begin
         if Repeated then %error(28); 
         PrintOptionalPlus := edlet = 'O'; { SP format }
         goto 1;
         end;
 
    'P': begin
         if not Repeated then %error(17); pval := lint; goto 1;
         end;
    
    '-',
    '+': begin
         if Repeated then %error(18);
         %scanfmt; if fmttok <> '0' then %error(19);
         if edlet = '+' then pval := fmtintval else pval := - fmtintval;
         %scanfmt; if fmttok <> 'P' then %error(20);
         goto 1;
         end;
         
    'N',
    'B': begin { BN, BZ } {B is really Z}
         if Repeated then %error(29); BZFlag := edlet = 'B'; goto 1;
         end;
    
    '(': begin
         if gplevel >= 11 then %error(21);
         if gplevel = 1 then GP2FieldFound := FALSE;
         gplevel := gplevel + 1; 
         gpcount[gplevel] := lint; gpbegin[gplevel] := fmtp;
         goto 1;
         end;
    
    ')': begin
         if Repeated then %error(22);
         if (gplevel = 2) and GP2FieldFound
         then 
           if lastgppos <> gpbegin[2]
           then begin
             lastgppos := gpbegin[2]; lastgprep := gpcount[2]; 
             lastgplevel := 2;
             end;
         if gpcount[gplevel] > 1
         then begin
           gpcount[gplevel] := gpcount[gplevel] - 1; 
           fmtp := gpbegin[gplevel];
           goto 1;
           end;
         if gplevel > 1 then begin gplevel := gplevel - 1; goto 1; end;
         if not OKToExit
         then
           if FieldFound
           then begin { Format reversion (wrap around) }
             %nextrec; gplevel := lastgplevel; fmtp := lastgppos;
             gpcount[gplevel] := lastgprep; gpbegin[gplevel] := lastgppos;
             goto 1;
             end
           else %error(86); { Can't wrap around an empty format and can't exit }
         end;
    
    ',': begin 
         if Repeated then %error(23); goto 1;
         end;
 
    '.',
    '0': %error(24);
  end; {case}
  end;
end; {%getfmt}


{ Initialize the format scanner }

procedure %ifmt(ffmt: ppac);
begin {%ifmt}
curfmt := ffmt; fmtp := 1; %scanfmt; 
if fmttok <> '(' then %error(32);
edcnt := 1; gplevel := 1; gpcount[1] := 1; gpbegin[1] := fmtp;
lastgppos := fmtp; lastgprep := 1; lastgplevel := 1;
FieldFound := FALSE; OKToExit := FALSE; InhibitFinalCR := FALSE; 
pval := 0; PrintOptionalPlus := FALSE;
end; {%ifmt}


{ Does the work for %_ixfwr and %_ixfrd }

procedure %ixf(unitnum: longint);
begin {%ixf}
errornumber := 0; InternalIO := FALSE;
curunit := %findunit(unitnum);
if curunit = nil
then %error(30)
else begin
  if curunit^.PastEndFile and (curunit <> consoleunit) then %error(55);
  if curunit^.Formted > FORMATTED then %error(31);
  BZFlag := not curunit^.BlankNull;
  end;
end; {%ixf}


{ Does the work for %_iifwr and %_iifrd }

procedure %iif(fintfile: pint1array; fintreclen: integer);
begin {%iif}
errornumber := 0; InternalIO := TRUE;
intfile := fintfile; intreclen := fintreclen; 
intbufp := 1; { pointer to character position of next internal I/O }
if intreclen > 4096 
then begin %error(82); intreclen := MAXRECLEN; end;
BZFlag := FALSE;
end; {%iif}


procedure %fposition(rec: longint);
  var targetposition,targnxtbyte,recsperbigblock,bigblocknumber: longint;
begin {%fposition}
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
        if ENVIRONMENT = MERLIN
        then begin
          %updatehighwatermark;
          recsperbigblock := 1024 div (reclen+1); {Note, <cr> is allowed last}
          bigblocknumber := rec div recsperbigblock;
          targnxtbyte := (rec mod recsperbigblock)*(reclen+1); {In a bigblock}
          targetposition := bigblocknumber * 1024 + targnxtbyte;
          %sysseek(targetposition);
          end
        else 
          if ENVIRONMENT = CPM
          then %sysseek(ord4(reclen+2) * rec)
          else %sysseek(ord4(reclen+1) * rec);
        end;
    end;
end; {%fposition}


{ Initialize external write or print statement (sequential access) }

procedure %_ixfwr(unitnum: longint; ffmt: ppac; ffmtlen: longint);
begin {%_ixfwr}
Reading := FALSE; %ixf(unitnum); %ifmt(ffmt);
{ Set up output buffer for record }
recbufp := 1; lastwritten := 0; maxlastwritten := 0;
end; {%_ixfwr}


{ Initialize external write or print statement (sequential access), UNIT=* }

procedure %_ixfwrd(ffmt: ppac; ffmtlen: longint);
begin {%_ixfwrd}
%_ixfwr(0,ffmt,ffmtlen);
end; {%_ixfwrd}


{ Initialize external write or print statement (with seek) }

procedure %_pxfwr(unitnum: longint; ffmt: ppac; ffmtlen: longint; rec: longint);
begin {%_pxfwr}
%_ixfwr(unitnum,ffmt,ffmtlen); %fposition(rec);
end; {%_pxfwr}


{ Initialize internal write or print statement (sequential access) }

procedure %_iifwr(fintfile: pint1array; fintreclen: longint; 
                    ffmt: ppac; ffmtlen: longint);
begin {%_iifwr}
Reading := FALSE; %iif(fintfile,fintreclen); %ifmt(ffmt);
{ Set up output buffer for record }
recbufp := 1; lastwritten := 0; maxlastwritten := 0;
end; {%_iifwr}


{ Terminate write or print statement }

procedure %_tfwr(fiostat: plongint; errexit: pcodearray);
begin {%_tfwr}
OKToExit := TRUE; { Inhibit wrap around } %getfmt; 
if (curunit = consoleunit) and InhibitFinalCR
then begin
  if lastwritten > maxlastwritten then maxlastwritten := lastwritten;
  %foutputrecbuf;
  end
else %nextrec; 
if (not InternalIO) and (curunit <> nil) then curunit^.lastop := WRITEOP;
%termiostmt(fiostat,errexit);
end; {%_tfwr}


{ Terminate write or print statement, no error checking }

procedure %_tfwrd;
  var errexit: pcodearray;
begin {%_tfwrd}
OKToExit := TRUE; { Inhibit wrap around } %getfmt; 
if (curunit = consoleunit) and InhibitFinalCR
then begin
  if lastwritten > maxlastwritten then maxlastwritten := lastwritten;
  %foutputrecbuf;
  end
else %nextrec; 
if (not InternalIO) and (curunit <> nil) then curunit^.lastop := WRITEOP;
errexit[1] := nil; %termiostmt(nil,errexit);
end; {%_tfwrd}


{ Initialize external read statement (sequential access) }

procedure %_ixfrd(unitnum: longint; ffmt: ppac; ffmtlen: longint);
begin {%_ixfrd}
Reading := TRUE; EofFlag := FALSE; %ixf(unitnum); %ifmt(ffmt); 
if curunit <> nil
then
  if curunit^.Seqen then %forceeofiflastwritten;
%nextrec;
end; {%_ixfrd}


{ Initialize external read statement (sequential access), UNIT=* }

procedure %_ixfrdd(ffmt: ppac; ffmtlen: longint);
begin {%_ixfrdd}
%_ixfrd(0,ffmt,ffmtlen);
end; {%_ixfrdd}


{ Initialize external read statement (with seek) }

procedure %_pxfrd(unitnum: longint; ffmt: ppac; ffmtlen: longint; rec: longint);
begin {%_pxfrd}
Reading := TRUE; EofFlag := FALSE; %ixf(unitnum); %ifmt(ffmt); 
%fposition(rec); %nextrec;
end; {%_pxfrd}


{ Initialize internal read statement (sequential access) }

procedure %_iifrd(fintfile: pint1array; fintreclen: longint; 
                    ffmt: ppac; ffmtlen: longint);
begin {%_iifrd}
Reading := TRUE; EofFlag := FALSE; 
%iif(fintfile,fintreclen); %ifmt(ffmt); %nextrec;
end; {%_iifrd}


{ Terminate read statement }

procedure %_tfrd(fiostat: plongint; errexit, endexit: pcodearray);
begin {%_tfrd}
OKToExit := TRUE; { Inhibit wrap around } 
%getfmt; { Required if, for example, / edit descriptors are next }
if (not InternalIO) and (curunit <> nil)
then begin
  if curunit^.Seqen and EofFlag then curunit^.PastEndFile := TRUE;
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
end; {%_tfrd}

procedure %puthex{*fval: integer*};
begin {%puthex}
if fval <= 9
then %putch(chr(fval + ord('0')))
else %putch(chr(fval + ord('A') - 10));
end; {%puthex}


{ Write a character or hex numeric expression }

procedure %%wrfch{*fpac: ppac; paclen: integer*};
  var lpaclen,i: integer; larray: array [0..1] of longint;
      buf: array[1..16] of integer; temp: longint;
  
  procedure %rightnibble;
    
    procedure %rightshift4(var fval: longint);
    begin {%rightshift4}
    if fval < 0
    then fval := ((fval and $7FFFFFFF) div 16) or $8000000
    else fval := fval div 16;
    end; {%rightshift4}
    
  begin {%rightnibble}
  %rightshift4(larray[1]);
  larray[1] := larray[1] or (larray[0] * $10000000);
  %rightshift4(larray[0]);
  end; {%rightnibble}
  
begin {%%wrfch}
if edlet = 'A'
then begin
  if not EdwPresent then edw := paclen;
  %putblanks(edw - paclen);
  if edw < paclen then lpaclen := edw else lpaclen := paclen;
  for i := 1 to lpaclen do
    %putch(fpac^[i]);
  end
else begin {edlet = 'Z', paclen = 1,2,4, or 8 (double)}
  temp := 1; moveleft(temp,buf,4);
  moveleft(fpac^,larray,paclen);
  if buf[2] = 1
  then {lsb in higher addresses (68000 byte sex)}
    for i := 1 to (8 - paclen)*2 do
      %rightnibble
  else begin {lsb in lower addresses (16000 byte sex)}
    temp := larray[0]; larray[0] := larray[1]; larray[1] := temp;
    end;
  if not EdwPresent then edw := paclen*2;
  %putblanks(edw - paclen*2);
  if edw < paclen*2 then lpaclen := edw else lpaclen := paclen*2;
  for i := 1 to lpaclen do begin
    buf[i] := larray[1] and $F;
    %rightnibble;
    end;
  for i := lpaclen downto 1 do
    %puthex(buf[i]);
  end;
end; {%%wrfch}

end. {%flibfmt}

