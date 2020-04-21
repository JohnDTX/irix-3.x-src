(*****************************************************************************)
(*                                                                           *)
(*                            File: FLIBINQ.TEXT                             *)
(*                                                                           *)
(*           (C) Copyright 1982, 1985 Silicon Valley Software, Inc.          *)
(*                                                                           *)
(*                            All Rights Reserved.               21-Aug-85   *)
(*                                                                           *)
(*****************************************************************************)


{$%+} {$R-} {$I-}

unit %flibinq;

interface

uses {$U flibinit} %flibinit,
     {$U flibrec}  %flibrec;

implementation

procedure %_fseek(fib: ptext; targetpos,sense: longint); external;
function _lseek(sense: longint; 
                targetposition: longint;
                fd: longint): longint; cexternal; {UNISOFT external}
function lseek(sense: longint; 
                targetposition: longint;
                fd: longint): longint; cexternal; {UNISOFT external}
function _Jfilepos(fd: longint): longint; cexternal; {UNOS external}

procedure %_inq(unitnum: longint;
                name: ppac; namelen: longint;
                fiostat: plongint;
                exist: plongint;  { really plogical*4 }
                opened: plongint; { really plogical*4 }
                number: plongint;
                named: plongint;  { really plogical*4 }
                recl: plongint;
                nextrec: plongint;
                errexit: pcodearray;
                nm: ppac; nmlen: longint;
                access: ppac; accesslen: longint;
                seq: ppac; seqlen: longint;
                dir: ppac; dirlen: longint;
                form: ppac; formlen: longint;
                ftted: ppac; fttedlen: longint;
                unftted: ppac; unfttedlen: longint;
                blank: ppac; blanklen: longint);
  
  var Found: Boolean; lname: filenamestring; lfile: file;
      orgblk, currec: longint;
  
  procedure %setint4(fplongint: plongint; fval: longint);
  begin {%setint4}
  if fplongint <> nil then fplongint^ := fval;
  end; {%setint4}
  
  procedure %stuffchar(fpac: ppac; fpaclen: integer; fstring: filenamestring);
    var i: integer;
  begin {%stuffchar}
  if fpac <> nil
  then
    for i := 1 to fpaclen do
      if i <= length(fstring) 
      then fpac^[i] := fstring[i] 
      else fpac^[i] := ' ';
  end; {%stuffchar}
  
  procedure %booloption(fpac: ppac; fpaclen: integer; fbool: Boolean;
                       tstring, fstring: filenamestring);
  begin {%booloption}
  if fbool 
  then %stuffchar(fpac,fpaclen,tstring)
  else %stuffchar(fpac,fpaclen,fstring);
  end; {%booloption}
  
begin {%_inq}
errornumber := 0; 
if name = nil
then curunit := %findunit(unitnum) { inquire by unit }
else begin { inquire by name }
  { set curunit to named unit if it can be found, nil otherwise }
  %makelname(lname,name,namelen);
  curunit := units; Found := FALSE;
  while (curunit <> nil) and (not Found) do begin
    if curunit^.fname = lname 
    then Found := TRUE 
    else curunit := curunit^.nextunit;
    end;
  end;
if (name <> nil) and (curunit = nil)
then begin 
  { inquire by name, file not connected, just a chance that it exists }
  reset(lfile,lname);
  if ioresult = 0
  then begin %setint4(exist,1); close(lfile); end
  else %setint4(exist,0);
  %setint4(opened,0); %setint4(named,1); %stuffchar(nm,nmlen,lname);
  %stuffchar(seq,seqlen,'UNKNOWN'); 
  %stuffchar(dir,dirlen,'UNKNOWN');
  %stuffchar(ftted,fttedlen,'UNKNOWN'); 
  %stuffchar(unftted,unfttedlen,'UNKNOWN');
  end
else { either inquire by unit or unit connected to inquired name }
  if curunit = nil
  then begin %setint4(exist,1); %setint4(opened,0); end
  else 
    with curunit^ do begin
      %setint4(exist,1); %setint4(opened,1); %setint4(named,ord(not Scratch));
      %stuffchar(nm,nmlen,fname);
      %setint4(number,unitid); %setint4(recl,curunit^.reclen);
      if ENVIRONMENT = MERLIN
      then begin
        if (not Seqen) and (nextrec <> nil)
        then begin
          if osfib^.fheader.fkind = TEXTFILE then orgblk := 2 else orgblk := 0;
          if Formted <= FORMATTED
          then 
            currec :=
              { Number of records in prev bigblocks }
              (1024 div (reclen+1)) * ((osfib^.fnextblock-1-orgblk) div 2) +
              
              { Number of records in current bigblock }
              ((((osfib^.fnextblock - 1) mod 2) * FBLKSIZE) + osfib^.fnextbyte) 
              div (reclen+1)
            
          else { not formatted }
            currec :=   (
              ((osfib^.fnextblock-1-orgblk) * FBLKSIZE) + {#bytes in prev blks}
              osfib^.fnextbyte  )                         {#bytes in cur blk  }
                 div reclen;
          nextrec^ := currec + 1;
          end;
        end
      else 
        if (ENVIRONMENT = UNISOFT) or (ENVIRONMENT = REGULUS) or
           (ENVIRONMENT = GENIX) or (ENVIRONMENT = TEK)
        then begin
          { *** Destroys register A2 under REGULUS *** }
          if (not Seqen) and (nextrec <> nil)
          then begin
            %_fseek(ostextfile,0,SEEK_RELATIVE); {Force flush}
            if UNDERSCORE
            then
              if Formted <= FORMATTED
              then
                if (ENVIRONMENT = GENIX) or (ENVIRONMENT = TEK)
                then nextrec^ := (_lseek(1,0,eltpfib^.fd) div (reclen+1)) + 1
                else nextrec^ := (_lseek(1,0,idosfib^.fd) div (reclen+1)) + 1
              else
                if (ENVIRONMENT = GENIX) or (ENVIRONMENT = TEK)
                then nextrec^ := (_lseek(1,0,eltpfib^.fd) div reclen) + 1
                else nextrec^ := (_lseek(1,0,idosfib^.fd) div reclen) + 1
            else
              if Formted <= FORMATTED
              then
                if (ENVIRONMENT = GENIX) or (ENVIRONMENT = TEK)
                then nextrec^ := (lseek(1,0,eltpfib^.fd) div (reclen+1)) + 1
                else nextrec^ := (lseek(1,0,idosfib^.fd) div (reclen+1)) + 1
              else
                if (ENVIRONMENT = GENIX) or (ENVIRONMENT = TEK)
                then nextrec^ := (lseek(1,0,eltpfib^.fd) div reclen) + 1
                else nextrec^ := (lseek(1,0,idosfib^.fd) div reclen) + 1;
            end;
          end
        else
          if ENVIRONMENT = UNOS
          then begin
            if (not Seqen) and (nextrec <> nil)
            then begin
              %_fseek(ostextfile,0,SEEK_RELATIVE); {Force Flush}
              if Formted <= FORMATTED
              then nextrec^ := (_Jfilepos(idosfib^.fd) div (reclen+1)) + 1
              else nextrec^ := (_Jfilepos(idosfib^.fd) div reclen) + 1;
              end;
            end
          else
            if (ENVIRONMENT = ADVENTURE) or (ENVIRONMENT = CPM)
            then begin
              if (not Seqen) and (nextrec <> nil)
              then begin
                %_fseek(ostextfile,0,SEEK_RELATIVE); {Force Flush}
                if Formted <= FORMATTED
                then
                  if ENVIRONMENT = CPM
                  then nextrec^ := (idosfib^.filepos div (reclen+2)) + 1
                  else nextrec^ := (idosfib^.filepos div (reclen+1)) + 1
                else nextrec^ := (idosfib^.filepos div reclen) + 1;
                end;
              end
            else
              if ENVIRONMENT = ELITE
              then begin
                if (not Seqen) and (nextrec <> nil)
                then begin
                  %_fseek(ostextfile,0,SEEK_RELATIVE); {Force Flush}
                  if Formted <= FORMATTED
                  then nextrec^ := (eltpfib^.filesyspos div (reclen+1)) + 1
                  else nextrec^ := (eltpfib^.filesyspos div reclen) + 1;
                  end;
                end
              else begin {Temporary until tell system call}
                if (not Seqen) and (nextrec <> nil)
                then nextrec^ := 0;
                end;
      %booloption(access,accesslen,Seqen,'SEQUENTIAL','DIRECT');
      %booloption(seq,seqlen,Seqen,'YES','NO');
      %booloption(dir,dirlen,Seqen,'NO','YES');
      %booloption(form,formlen,Formted<=FORMATTED,'FORMATTED','UNFORMATTED');
      %booloption(ftted,fttedlen,Formted<=FORMATTED,'YES','NO');
      %booloption(unftted,unfttedlen,Formted<=FORMATTED,'NO','YES');
      %booloption(blank,blanklen,BlankNull,'NULL','ZERO');
      end;
%termiostmt(fiostat,errexit);
end; {%_inq}

end. {%flibauxio}

