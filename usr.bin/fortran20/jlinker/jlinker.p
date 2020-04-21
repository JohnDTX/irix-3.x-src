(*****************************************************************************)
(*                                                                           *)
(*                              PROGRAM JLINKER                              *)
(*                                                                           *)
(*           (C) Copyright 1984, 1985 Silicon Valley Software, Inc.          *)
(*                                                                           *)
(*                           All Rights Reserved.                30-Oct-85   *)
(*                                                                           *)
(*****************************************************************************)


{$I-}
{$R-}
{$%+}

program jlinker;

label 999;

const MAXOUTBYTE = 4095;
      DATE = '30-Oct-85';
      VERSION = 'V2.4';
      
      MALE = 0; FEMALE = 1; M68000 = 0; N32000 = 1; M68020 = 2;

      MERLIN = 0; UNISOFT = 1; CT = 2; ARETE = 3; PLEXUS = 4;
      SILGRAPH = 5; HITACHI = 6; SYS5_2 = 7; TEK = 8; SEQ = 9;
      OPUS = 10; CT5_2 = 11; UNOS = 12;

      { Target Operating System }

          TARGETOS = SILGRAPH;
          TARGUSCORE = TRUE;
          AUSTIN = FALSE;  { "TRUE" only works with SYS5_2 }
        
      { Host Operating System }

          HOSTOS = SILGRAPH;
          HOSTUSCORE = TRUE;
        
          { Unix Hosts }
            OBJSUFFIX = '.obj';
            ARGCBASE = 1;
            BINSUFFIX = '.o';
          {}
      
          { Merlin 
            OBJSUFFIX = '.obj';
            ARGCBASE = 0;
            BINSUFFIX = '.o';
          {}
      
      { Target Machine Description: }
    
          { M68020 }
            CHIP = M68020; TARGSEX = MALE;
#ifdef juniper
            TITLE = 'MC68020 Object Code Formatter';
#else
            TITLE = 'MC68010 Object Code Formatter';
#endif
          {}
    
          { N32000 
            CHIP = N32000; TARGSEX = FEMALE;
            TITLE = 'NS32000 Object Code Formatter';
          {}
    
      { Host Machine Description: }
    
          { M68000, M68020 }
            HOSTSEX = MALE;
          {}
    
          { N32000 
            HOSTSEX = FEMALE;
          {}
    
type string31 = string[31];
     string63 = string[63];
     alfa8 = packed array[1..8] of char;
     alfa4 = packed array[1..4] of char;
     byte = -128..127;
     pbyte = ^byte;
     
     pmodlist = ^modulerec;
     modulerec = record modblock,modbyte: integer;
                        nextmodule: pmodlist;
                 end;
     
     palfas = ^alfas;
     alfas = record
               case integer of
                 0: (a4: alfa4);
                 1: (a8: alfa8);
                 2: (a12: packed array[1..12] of char);
                 3: (a16: packed array[1..16] of char);
                 4: (a20: packed array[1..20] of char);
                 5: (a24: packed array[1..24] of char);
                 6: (a28: packed array[1..28] of char);
                 7: (a32: packed array[1..32] of char); {glinkid only}
             end;
     
     linkid = record linkno: integer;
                     nmlen: integer;
                     n4: alfa4;
                     tail: palfas;
              end;
              
     psymbol = ^symbol;
     symbol = record symname: linkid;
                     llink,rlink: psymbol;
                     symtabnum: integer;
                     case Defined: Boolean of
                          TRUE: (loc: longint);
              end;
     
     pdatainit = ^datainit;
     datainit = record datamodule: pmodlist;
                       dataloc: longint;
                       datanext: pdatainit;
                end;
     
     pcommon = ^common;
     common = record datname: linkid; 
                     llink,rlink: pcommon;
                     symtabnum: integer;
                     initlist,lastinit: pdatainit;
                     loc: longint;  {data or bss relative; -1 common}
                     size: longint; {-1 if not Defined}
              end;
     
     preflist = ^reflist;
     reflist = record nextref: preflist;
                      refloc: longint;
                      case Boolean of
                        TRUE:  (srefto: psymbol);  { symtabnum }
                        FALSE: (crefto: pcommon);
               end;
     
     pfdreloc = ^fdreloc;
     fdreloc = record dname,rname: pcommon;
                      daddr: longint;
                      next: pfdreloc;
               end;
     
var infile: file;
    inbyte,inblock,nextinblock,lastinbyte: integer;
    inbuff: packed array[0..4095] of 0..255; {better than byte!}
    listfile: text;
    ListOpen: Boolean;
    ifname,fname,ofname,sfname: string63;
    largc: integer;
    
    outfile: file;
    outbuffer: array[0..4095] of byte;
    outbyte,outblock: integer;
    OutOpen: Boolean;

    modulelist: pmodlist;       { List of all code modules }
    lastmodule: pmodlist;       { Last module on above list }

    moduleloc: longint;
    nxttaddr,                   { Text size }
    textpadding,                { Included in nxttaddr }
    nxtdaddr,                   { Data size }
    nxtbaddr: longint;          { BSS size }
    dataic: longint;

    nxtstnum: integer;          { Symbol table entry numbers }
    textstnum, datastnum, bssstnum: integer;

    symtabbytes: longint;
    strtabbytes: longint;
    trelocbytes: longint;
    drelocbytes: longint;

    StartFlag: Boolean;

    nametree: psymbol;          { Entry point tree }
    ftndatatree: pcommon;       { Tree of FTN Data Areas }
    
    ComFlag: Boolean;
    commandfile: text;
    
    DebugFlag,OK,SysFlag,MapFlag,AlfaFlag,EscapeFlag: Boolean;
    errors: integer;
    
    vartypelist,                { List of Variable/Type defs $A0 }
    lastvartype,
    bpointlist,                 { List of Statement Offset defs $A1 }
    lastbpoint: pmodlist;

    symrefs: preflist;          { List of all symbol references }
    symrefcount: longint;
    comrefs: preflist;          { List of all common references }
    comrefcount: longint;

    symrefhead, comrefhead: preflist;
	
    relocval: longint;
    trickpos: integer;
    trickrec: record case Boolean of
                          TRUE:  (l: longint);
                          FALSE: (a: array[0..3] of byte);
              end;
    
    glinkid: linkid;            { Set by readlinkid }

    CConstructs: Boolean;
    fdreloclist: pfdreloc;      { Head of FTN Data relocation list }
    fdfreloclist: pfdreloc;     { Head of FTN Data Function relocation list }
    
function %%write(cnt: longint; buf: pbyte; fd: longint): longint; cexternal;

function %%read(cnt: longint; buf: pbyte; fd: longint): longint; cexternal;
procedure _tstc(jfile: string63; ofile: string63); cexternal;

function time(null: longint): longint; cexternal;
function _time(null: longint): longint; cexternal;

function getpid: longint; cexternal;
function _getpid: longint; cexternal;

procedure warning(s: string63);
begin
writeln(chr(7),'*** Warning - ',s,' ***');
end; {warning}

procedure error(s: string63);
begin
writeln(chr(7),'*** Error - ',s,' ***');
errors := errors + 1;
end; {error}

procedure fatal_error(s: string63);
begin
writeln(chr(7),'*** Fatal Error - ',s,' ***');
errors := errors + 1;
goto 999;
end; {fatal_error}

procedure prompt(message: string63; var s: string63);
  var ch: char;
begin
write(message);
if ComFlag
then begin
     write(' : ');
     if eof(commandfile)
     then begin
          writeln('*** Eof ***');
          close(commandfile);
          ComFlag := FALSE;
          prompt(message,s);
          end
     else begin
          readln(commandfile,s);
          writeln(s);
          end;
     end
else begin
     write(' - ');
     readln(s);
     if length(s) > 0
     then if s[1] = '<'
          then begin
               delete(s,1,1);
               reset(commandfile,s);
               if ioresult = 0
               then ComFlag := TRUE
               else begin
                    s := concat(s,'.TEXT');
                    reset(commandfile,s);
                    if ioresult = 0
                    then ComFlag := TRUE
                    else warning('Unable to open file');
                    end;
               prompt(message,s);
               end
          else if s[1] = chr(27) { ESCAPE }
               then begin EscapeFlag := TRUE; goto 999; end;
     end;
if length(s) > 0
then if s[1] in ['+','-','?']
     then begin
          if length(s) >= 2
          then ch := s[2]
          else ch := ' ';
          case s[1] of
               '?': begin
                    writeln('Options are:');
                    writeln;
                    writeln('Option Value Description:');
                    writeln('------ ----- ------------');
                    if AlfaFlag
                       then ch := '+'
                       else ch := '-';
                    writeln('+A -A   ''',ch,'''  Alphabetical list');
                    if MapFlag
                       then ch := '+'
                       else ch := '-';
                    writeln('+M -M   ''',ch,'''  Memory map');
                    if SysFlag
                       then ch := '+'
                       else ch := '-';
                    writeln('+S -S   ''',ch,'''  System entries (%)');
                    writeln;
                    end;
               '+','-':
                    case ch of
                      'A','a': AlfaFlag := s[1] = '+';
                      'D','d': DebugFlag := s[1] = '+';
                      'M','m': MapFlag := s[1] = '+';
                      'S','s': SysFlag := s[1] = '+';
                    otherwise: warning(concat('Bad option: ',s));
                    end; {case}
          end;
          prompt(message,s);
          end;
end; {prompt}

procedure lowercase(var s: string63);
  var i: integer; ch: char;
begin
for i := 1 to length(s) do begin
    ch := s[i];
    if (ch >= 'A') and (ch <= 'Z')
    then s[i] := chr(ord(ch) + 32);
    end;
end; {lowercase}

procedure suffix(var fname: string63; suf: string63);
begin
if (length(suf) >= length(fname)) or
   (pos(suf,fname) < length(fname) - length(suf) + 1)
then fname := concat(fname,suf);
end; {suffix}

procedure initialize;
  var i,len: integer; BadOption: Boolean; ch1,ch2: char; firstname: string63;
begin
writeln(TITLE,'  ',VERSION,' ':10,DATE);
writeln('(C) Copyright 1984, 1985 Silicon Valley Software, Inc.');
ComFlag := FALSE; DebugFlag := FALSE;
SysFlag := FALSE; MapFlag := FALSE; EscapeFlag := FALSE;
ListOpen := FALSE; OutOpen := FALSE; AlfaFlag := FALSE;
nxttaddr := 0; nxtdaddr := 0; nxtbaddr := 0;
symtabbytes := 0; strtabbytes := 4; trelocbytes := 0; drelocbytes := 0;
errors := 0; StartFlag := FALSE; new(glinkid.tail);
outbyte := 0; outblock := 0; ftndatatree := nil; nametree := nil;
symrefs := nil; comrefs := nil; symrefcount := 0; comrefcount := 0;
modulelist := nil; lastmodule := nil; vartypelist := nil; bpointlist := nil;
CConstructs := FALSE; fdreloclist := nil; fdfreloclist := nil;
sfname := '';
if argc > ARGCBASE
then begin
     fname := ''; ofname := ''; firstname := '';
     for i := ARGCBASE + 1 to argc do begin
         BadOption := FALSE; len := length(argv[i]^); ch1 := argv[i]^[1];
         if (ch1 = '+') or (ch1 = '-')
         then if len > 1
              then begin
                   ch2 := argv[i]^[2];
                   if ch2 >= 'a' then ch2 := chr(ord(ch2) - 32);
                   if len > 2
                   then begin
                        case ch2 of
                          'L': if length(fname) = 0
                               then begin
                                    fname := copy(argv[i]^,3,len - 2);
                                    MapFlag := TRUE;
                                    end
                               else error('Multiple list files');
                          'O': if length(ofname) = 0
                               then ofname := copy(argv[i]^,3,len - 2)
                               else error('Multiple output files');
                          'D': if length(sfname) = 0
                               then sfname := copy(argv[i]^,3,len - 2)
                               else error('Multiple debug files');
                        otherwise: BadOption := TRUE;
                        end; {case}
                        end
                   else begin
                        case ch2 of
                          'A': AlfaFlag := ch1 = '+';
                          'D': DebugFlag := ch1 = '+';
                          'M': MapFlag := ch1 = '+';
                          'S': SysFlag := ch1 = '+';
                        otherwise: BadOption := TRUE;
                        end; {case}
                        end;
                   end
              else BadOption := TRUE
         else if length(firstname) = 0
              then begin
                   moveleft(argv[i]^,firstname,len + 1);
                   (*** lowercase(firstname); ***)
                   if HOSTOS <> SILGRAPH
                   then suffix(firstname,OBJSUFFIX);
                   firstname := copy(firstname,1,length(firstname)-4);
                   end;
         if BadOption
         then error(concat('Bad option ',argv[i]^));
         end;
     if errors > 0 then goto 999;
     if length(ofname) = 0 then ofname := firstname;
     end;
if argc = ARGCBASE 
then begin writeln; prompt('Listing file',fname); end;
if length(fname) <> 0
then begin
     if argc = ARGCBASE then MapFlag := TRUE;
     rewrite(listfile,fname);
     if ioresult <> 0
     then fatal_error(concat('Can''t open list file ''',fname,''''));
     ListOpen := TRUE;
     end;
if argc = ARGCBASE then prompt('Output file ',ofname);
(*** lowercase(ofname); ***)
if HOSTOS <> SILGRAPH
then suffix(ofname,BINSUFFIX);
rewrite(outfile,ofname);
if ioresult <> 0
then fatal_error(concat('Can''t open output file ''',ofname,''''));
OutOpen := TRUE;
end; {initialize}

procedure readinfile(message: string63);
  var n: integer;
begin
n := blockread(infile,inbuff,8,inblock);
if DebugFlag
then writeln('Read ',n:1,' blocks at block ',inblock,'.');
if n = 0
then fatal_error(concat('Unable to read ',message));
nextinblock := inblock + n; lastinbyte := n*512 - 1;
end; {readinfile}

function nextbyte: integer;
begin
if inbyte > lastinbyte
then begin
     inbyte := 0; inblock := nextinblock; readinfile('input file');
     end;
{Best code, don't reorder!}
inbyte := inbyte + 1; nextbyte := inbuff[inbyte - 1];
end; {nextbyte}

function nextword: integer;
begin
nextword := nextbyte*256 + nextbyte;
end;  {nextword}

function next3bytes: longint;
  var i,j,k: longint;
begin
i := nextbyte; j := nextbyte; k := nextbyte;
next3bytes := (i*256 + j)*256 + k;
end; {next3bytes}

function next4bytes: longint;
  var i,j,k,l: longint;
begin
i := nextbyte; j := nextbyte; k := nextbyte; l := nextbyte;
next4bytes := ((i*256 + j)*256 + k)*256 + l;
end; {next4bytes}

procedure nexts(var fstr: string31);
  var i: integer;
begin
i := nextbyte; fstr[0] := chr(i);
if i > 31 then i := 31;
for i := 1 to i do fstr[i] := chr(nextbyte);
end; {nexts}

procedure skip(fbytes: longint);
begin
if odd(fbytes)
then error('Skip(fbytes odd)');
if (inbyte + fbytes) > lastinbyte
then begin
     inblock := inblock + (inbyte + fbytes) div 512;
     inbyte := (inbyte + fbytes) mod 512;
     readinfile('infile');
     end
else inbyte := inbyte + fbytes;
end; {skip}

procedure seekmodule(fmodule: pmodlist);
begin
with fmodule^ do
     begin
     if (modblock < inblock) or (modblock >= nextinblock)
     then begin
          inblock := modblock;
          readinfile('file');
          end;
     inbyte := (modblock - inblock)*512 + modbyte;
     end;
end; {seekmodule}

procedure reada8(var fname: alfa8);
  var i: integer;
begin
for i := 1 to 8 do
    fname[i] := chr(nextbyte);
end; {reada8}

procedure makestr(flinkid: linkid; var fstr: string31);
  var i: integer;
begin {makestr}
fstr[0] := chr(flinkid.nmlen);
for i := 1 to flinkid.nmlen do
    if i <= 4
    then fstr[i] := flinkid.n4[i]
    else fstr[i] := flinkid.tail^.a28[i - 4];
end; {makestr}

procedure makelname(flinkid: linkid; var fstr: string63);
  var i,j: integer; lstring: string63;
begin {makelname}
if flinkid.linkno < 0
then begin
     lstring[0] := chr(flinkid.nmlen);
     for j := 1 to flinkid.nmlen do
         if j <= 4
         then lstring[j] := flinkid.n4[j]
         else lstring[j] := flinkid.tail^.a28[j - 4];
     end
else begin {Not -1}
     lstring[0] := chr(8); lstring[1] := '$';
     i := flinkid.linkno;
     for j := 2 to 8 do begin
         lstring[j] := chr(i mod 10 + ord('0'));
         i := i div 10;
         end;
     end;
fstr := lstring;
end; {makelname}

procedure makealfa8(flinkid: linkid; var falfa8: alfa8);
  var i: integer;
begin {makealfa8}
for i := 1 to 8 do
    if i <= flinkid.nmlen
    then if i <= 4
         then falfa8[i] := flinkid.n4[i]
         else falfa8[i] := flinkid.tail^.a28[i - 4]
    else falfa8[i] := ' ';
end; {makealfa8}

procedure readlinkid;
  var i,garbage: integer;
begin {readlinkid}
glinkid.linkno := nextword; glinkid.nmlen := nextbyte;
for i := 1 to glinkid.nmlen + 4 do
  if i <= 4
  then 
    if i <= glinkid.nmlen
    then glinkid.n4[i] := chr(nextbyte)
    else glinkid.n4[i] := ' '
  else 
    if i <= glinkid.nmlen
    then glinkid.tail^.a32[i - 4] := chr(nextbyte)
    else glinkid.tail^.a32[i - 4] := ' ';
if not odd(glinkid.nmlen) then garbage := nextbyte;
end; {readlinkid}

procedure reads63(var fstr: string63; len: integer);
  var i,j: integer;
begin
fstr := '';
for i := 1 to len do
    begin
    j := nextbyte;
    if i <= 25
    then begin
         fstr := concat(fstr,' ');
         fstr[i] := chr(j);
         end;
    end;
end; {reads63}

procedure flushout;
  var blocks: integer; ptr: ^longint;
begin
if HOSTOS = MERLIN
then begin
     for blocks := outbyte to MAXOUTBYTE do
         outbuffer[blocks] := 0;
     blocks := (outbyte + 511) div 512;
     if blockwrite(outfile,outbuffer,blocks,outblock) <> blocks
     then fatal_error('Can''t write output file!');
     outblock := outblock + blocks;
     end
else begin
     if (HOSTOS = TEK) or (HOSTOS = SEQ) or (HOSTOS = OPUS)
     then ptr := pointer(ord(@outfile) + 14)  { fd }
     else ptr := pointer(ord(@outfile) + 12); { fd }
     if %%write(outbyte,@outbuffer,ptr^) <> outbyte
     then fatal_error('Can''t write output file!');
     end;
end; {flushout}
 
procedure outone(a: integer);
begin {outone}
if outbyte > MAXOUTBYTE
then begin flushout; outbyte := 0; end;
outbuffer[outbyte] := a; outbyte := outbyte + 1;
end; {outone}
 
{ out: Output an int most significant byte first, may be called with n=0 }
 
procedure out(a: longint; n: integer);
  var i: integer; c: array[0..3] of byte;
begin {out}
moveleft(a,c,4);
if HOSTSEX = MALE
then for i := 4 - n to 3 do outone(c[i])
else for i := n-1 downto 0 do outone(c[i]);
end; {out}

{ outl: Output an int least significant byte first, may be called with n=0 }
 
procedure outl(a: longint; n: integer);
  var i: integer; c: array[0..3] of byte;
begin {outl}
moveleft(a,c,4);
if HOSTSEX = MALE
then for i := 3 downto 4 - n do outone(c[i])
else for i := 0 to n-1 do outone(c[i]);
end; {outl}

procedure outstr(fstr: string31);
  var i: integer;
begin
for i := 1 to length(fstr) do
    outone(ord(fstr[i]));
outone(0); { Null terminator }
end; {outstr}

procedure outname(fname: alfa8);
  var i: integer;
begin {outname}
for i := 1 to 8 do
    if fname[i] <> ' ' then outone(ord(fname[i])) else outone(0);
end; {outname}

function idEQid(var a,b: linkid): Boolean;
begin
if a.linkno = b.linkno
then
  if a.linkno < 0
  then
    if (a.nmlen = b.nmlen) and (a.n4 = b.n4)
    then case a.nmlen of
           0,1,2,3,4:   idEQid := TRUE;
           5,6,7,8:     idEQid := a.tail^.a4 = b.tail^.a4;
           9,10,11,12:  idEQid := a.tail^.a8 = b.tail^.a8;
           13,14,15,16: idEQid := a.tail^.a12 = b.tail^.a12;
           17,18,19,20: idEQid := a.tail^.a16 = b.tail^.a16;
           21,22,23,24: idEQid := a.tail^.a20 = b.tail^.a20;
           25,26,27,28: idEQid := a.tail^.a24 = b.tail^.a24;
           29,30,31,32: idEQid := a.tail^.a28 = b.tail^.a28;
         end {case}
    else idEQid := FALSE
  else idEQid := TRUE
else idEQid := FALSE;
end; {idEQid}

function idLTid(var a,b: linkid): Boolean;
  var shorter: integer;
begin
if a.linkno = b.linkno
then if a.linkno < 0
     then if a.n4 < b.n4
          then idLTid := TRUE
          else if a.n4 = b.n4
               then begin
                    if a.nmlen < b.nmlen
                    then shorter := a.nmlen
                    else shorter := b.nmlen;
                    case shorter of
                      0,1,2,3,4:   idLTid := a.nmlen < b.nmlen;
                      5,6,7,8:     if a.tail^.a4 = b.tail^.a4
                                   then idLTid := a.nmlen < b.nmlen
                                   else idLTid := a.tail^.a4 < b.tail^.a4;
                      9,10,11,12:  if a.tail^.a8 = b.tail^.a8
                                   then idLTid := a.nmlen < b.nmlen
                                   else idLTid := a.tail^.a8 < b.tail^.a8;
                      13,14,15,16: if a.tail^.a12 = b.tail^.a12
                                   then idLTid := a.nmlen < b.nmlen
                                   else idLTid := a.tail^.a12 < b.tail^.a12;
                      17,18,19,20: if a.tail^.a16 = b.tail^.a16
                                   then idLTid := a.nmlen < b.nmlen
                                   else idLTid := a.tail^.a16 < b.tail^.a16;
                      21,22,23,24: if a.tail^.a20 = b.tail^.a20
                                   then idLTid := a.nmlen < b.nmlen
                                   else idLTid := a.tail^.a20 < b.tail^.a20;
                      25,26,27,28: if a.tail^.a24 = b.tail^.a24
                                   then idLTid := a.nmlen < b.nmlen
                                   else idLTid := a.tail^.a24 < b.tail^.a24;
                      29,30,31,32: if a.tail^.a28 = b.tail^.a28
                                   then idLTid := a.nmlen < b.nmlen
                                   else idLTid := a.tail^.a28 < b.tail^.a28;
                    end; {case}
                    end
               else idLTid := FALSE
     else idLTid := FALSE
else idLTid := a.linkno < b.linkno;
end; {idLTid}

procedure entername(fsymbol: psymbol);
  var lsymbol: psymbol;
begin
if nametree = nil
then nametree := fsymbol
else begin
     lsymbol := nametree;
     while lsymbol <> nil do
           with lsymbol^ do
                if idEQid(symname,fsymbol^.symname)
                then begin
                     writeln(chr(7),'Duplicate name');
                     lsymbol := nil;
                     end
                else if idLTid(symname,fsymbol^.symname)
                     then if llink = nil
                          then begin
                               llink := fsymbol;
                               lsymbol := nil;
                               end
                          else lsymbol := llink
                     else if rlink = nil
                          then begin
                               rlink := fsymbol;
                               lsymbol := nil;
                               end
                          else lsymbol := rlink;
     end;
end; {entername}

function lookupname: psymbol;
  var lsymbol: psymbol; ExitFlag: Boolean;
begin
lsymbol := nametree;
ExitFlag := FALSE;
while not ExitFlag and (lsymbol <> nil) do
      with lsymbol^ do
           if idEQid(glinkid,symname)
           then ExitFlag := TRUE
           else if idLTid(glinkid,symname)
                then lsymbol := rlink
                else lsymbol := llink;
lookupname := lsymbol;
end; {lookupname}

function lookupcommon(var ftree: pcommon): pcommon;
  var lcommon: pcommon;
  
  function newcommon: pcommon;
    var lcommon: pcommon;
  begin
  new(lcommon);
  with lcommon^ do begin
       datname := glinkid;
       case glinkid.nmlen of
         0,1,2,3,4:   datname.tail := nil;
         5,6,7,8:     begin
                      new(datname.tail,0);
                      datname.tail^.a4 := glinkid.tail^.a4;
                      end;
         9,10,11,12:  begin
                      new(datname.tail,1);
                      datname.tail^.a8 := glinkid.tail^.a8;
                      end;
         13,14,15,16: begin
                      new(datname.tail,2);
                      datname.tail^.a12 := glinkid.tail^.a12;
                      end;
         17,18,19,20: begin
                      new(datname.tail,3);
                      datname.tail^.a16 := glinkid.tail^.a16;
                      end;
         21,22,23,24: begin
                      new(datname.tail,4);
                      datname.tail^.a20 := glinkid.tail^.a20;
                      end;
         25,26,27,28: begin
                      new(datname.tail,5);
                      datname.tail^.a24 := glinkid.tail^.a24;
                      end;
         29,30,31,32: begin
                      new(datname.tail,6);
                      datname.tail^.a28 := glinkid.tail^.a28;
                      end;
       end; {case}
       llink := nil; rlink := nil;
       initlist := nil; lastinit := nil; size := -1;
       end;
  newcommon := lcommon;
  end; {newcommon}
  
begin {lookupcommon}
if ftree = nil
then begin ftree := newcommon; lookupcommon := ftree; end
else begin
     lcommon := ftree;
     while lcommon <> nil do
           with lcommon^ do
                if idEQid(datname,glinkid)
                then begin
                     lookupcommon := lcommon; lcommon := nil;
                     end
                else if idLTid(datname,glinkid)
                     then if llink = nil
                          then begin
                               llink := newcommon;
                               lookupcommon := llink; lcommon :=  nil;
                               end
                          else lcommon := llink
                     else if rlink = nil
                          then begin
                               rlink := newcommon;
                               lookupcommon := rlink; lcommon :=  nil;
                               end
                          else lcommon := rlink;
     end;
end; {lookupcommon}

function chksex4(l: longint): longint;
  var c: array[0..3] of byte; t: integer;
begin
if HOSTSEX <> TARGSEX
then begin
     moveleft(l,c,4);
     t := c[0]; c[0] := c[3]; c[3] := t;
     t := c[1]; c[1] := c[2]; c[2] := t;
     moveleft(c,l,4);
     end;
chksex4 := l;
end; {chksex4}

{$I jmisc}
{$I jphase.1}
{$I jphase.2a}
{$I jphase.2b}

procedure wrsymbol(var f: text; fsymbol: psymbol);
  var j: integer; lstring: string63;
begin
with fsymbol^ do begin
     if Defined then write(f,loc:6 hex) else write(f,'******');
     write(f,'  ');
     if symname.linkno <> -1
     then begin makelname(symname,lstring); write(f,lstring,' - '); end;
     for j := 1 to symname.nmlen do
         if j <= 4
         then write(f,symname.n4[j])
         else write(f,symname.tail^.a28[j - 4]);
     end;
writeln(f);
end; {wrsymbol}

procedure printsymbols(var f: text; fsymbol: psymbol);
begin
if fsymbol <> NIL
then with fsymbol^ do
     begin
     printsymbols(f,rlink);
     if ((symname.n4[1] <> '%') or SysFlag) or not Defined
     then wrsymbol(f,fsymbol);
     printsymbols(f,llink);
     end;
end; {printsymbols}

procedure sortbyloc(fsymbol: psymbol);
 var lsymbol: psymbol;
begin
if fsymbol <> nil
then with fsymbol^ do
     begin
     sortbyloc(llink); llink := nil;
     sortbyloc(rlink); rlink := nil;
     if Defined
     then if nametree = nil
          then nametree := fsymbol
          else begin
               lsymbol := nametree;
               while lsymbol <> nil do
                     if loc < lsymbol^.loc
                     then if lsymbol^.rlink = nil
                          then begin
                               lsymbol^.rlink := fsymbol;
                               lsymbol := nil;
                               end
                          else lsymbol := lsymbol^.rlink
                     else if lsymbol^.llink = nil
                          then begin
                               lsymbol^.llink := fsymbol;
                               lsymbol := nil;
                               end
                          else lsymbol := lsymbol^.llink;
               end;
     end;
end; {sortbyloc}

procedure printftndata(var f: text; fftndata: pcommon);
  var lstring: string63;
begin
if fftndata <> nil
then with fftndata^ do
     begin
     printftndata(f,rlink);
     if size <> -1
     then begin
          write(f,size:6 hex,'  (');
          if initlist <> nil
          then write(f,loc:6 hex,' - Data)  ')
          else if datname.linkno >= 0
               then write(f,loc:6 hex,' - BSS)   ')
               else write(f,'common)         ');
          end
     else write(f,'******                   ');
     makelname(datname,lstring);
     writeln(f,lstring);
     printftndata(f,llink);
     end;
end; {printftndata}

procedure finalize;
  var i: integer; lsymbol: psymbol;
  
  procedure printinfo(var f: text);
  begin
  if AlfaFlag or MapFlag
  then begin
       writeln(f,TITLE,' ':2,DATE);
       writeln(f);
       writeln(f,'File: ',ofname);
       writeln(f);
       end;
  if AlfaFlag 
  then printsymbols(f,nametree);
  if MapFlag
  then begin
       lsymbol := nametree;
       nametree := nil;
       sortbyloc(lsymbol);
       writeln(f);
       writeln(f,'Memory map:');
       writeln(f);
       printsymbols(f,nametree);
       writeln(f);
       end;
  if AlfaFlag or MapFlag
  then begin
       if ftndatatree <> nil
       then begin
            writeln(f);
            writeln(f,'Data Areas:');
            writeln(f);
            printftndata(f,ftndatatree);
            end;
       writeln(f);
       writeln(f,'Code Size = ',nxttaddr:6 hex);
       writeln(f,'Data Size = ',nxtdaddr:6 hex);
       writeln(f,'BSS Size =  ',nxtbaddr:6 hex);
       end
  else if not StartFlag
       then writeln(f,'No Starting Location');
  writeln(f);
  end; {printinfo}
  
begin {finalize}
if OutOpen
then if ListOpen
     then begin
          printinfo(listfile);
          close(listfile,LOCK);
          end;
writeln;
if errors > 0
then writeln(errors:1,' errors.');
if (errors > 0) or EscapeFlag
then writeln('The output is not saved.');
if OutOpen
then if (errors > 0) or EscapeFlag
     then close(outfile,PURGE)
     else close(outfile,LOCK);
if errors > 0 then halt(-1);
end; {finalize}


procedure getarg(var fname: string63);
  var ch: char;
begin
if argc = ARGCBASE
then if HOSTOS = SILGRAPH
     then prompt('Input file ',fname)
     else prompt(concat('Input file [',OBJSUFFIX,']'),fname)
else begin
     fname := '';
     while (length(fname) = 0) and (largc < argc) do begin
           largc := largc + 1; ch := argv[largc]^[1];
           if (ch <> '-') and (ch <> '+')
           then moveleft(argv[largc]^,fname,length(argv[largc]^) + 1);
           end;
     end;
end; {getarg}

begin {jlinker}
initialize;
largc := ARGCBASE; getarg(ifname);
(*** lowercase(ifname); ***)
if HOSTOS <> SILGRAPH
then suffix(ifname,OBJSUFFIX);
reset(infile,ifname);
OK := ioresult = 0;
if OK
then begin
     nextinblock := 0; lastinbyte := -1; inbyte := 0; phase1;
     end
else fatal_error(concat('Can''t open input file ',ifname));
phase2;
{ Remove because we no longer use dbg
if (bpointlist <> nil) or (vartypelist <> nil) then outsymfile;
}
999: finalize;
close(outfile);_tstc(ifname, ofname);
end. {jlinker}

