(*****************************************************************************)
(*                                                                           *)
(*                             File: LIBRARY.TEXT                            *)
(*                                                                           *)
(*          (C) Copyright 1982, 1985 Silicon Valley Software, Inc.           *)
(*                                                                           *)
(*                            All Rights Reserved.               30-Oct-85   *)
(*                                                                           *)
(*****************************************************************************)


{$I-}
{$R-}

program library;

label 999;

const DATE = '30-Oct-85';
      VERSION = 'V2.4';
      
      OTHER = 0; UNISOFT = 2; ELITE = 9; ENVIRONMENT = UNISOFT;
  
type string45 = string[45];
     alfa8 = packed array[1..8] of char;
     
     pintlist = ^intlist;
     intlist = record int: integer;
                      next: pintlist;
               end;
     
     psymbol = ^symbol;
     symbol = record linkname,username: alfa8;
                     llink,rlink,next: psymbol;
                     reflist: pintlist;
                     case Defined: Boolean of
                          TRUE: (loc,modnum: integer);
              end;
     
     ppatchlist = ^patchlist;
     patchlist = record patchloc: integer;
                        patchval: integer;
                        nextpatch: ppatchlist;
                 end;
     
     pmod = ^module;
     module = record modname: alfa8;
                     next: pmod;
                     modno,codesize,
                     idiskaddr,itextaddr,textsize: integer;
                     otextaddr,odiskaddr,iinitaddr: longint;
                     othermods: pintlist;
              end;
     
     pfilerec = ^filerec;
     filerec = record name: string45;
                      lbaseno,ltypeno: integer;
                      next: pfilerec;
               end;
     
     linkid = record linkno: integer; nam: string[8]; end;
     
var infile: file;
    inbuff: array[0..255] of integer;
    inword,inblock: integer;
    mapfile: text;
    ListFlag,HalfFlag: Boolean;
    LastByte: integer;
    fname: string45;
    FlipBytes,EofFlag: Boolean;
    
    reloclist,lastreloc: pintlist;
    
    outfile: file;
    outbuffer: packed array[0..511] of char;
    outbyte,outblock: integer;
    
    files,thisfile,lfile: pfilerec;
    StartFlag: Boolean;
    startmod: integer;
    lprocbase,lprocmax,ltypebase,ltypemax: integer;
    
    nametree: psymbol;
    
    headpatchlist: ppatchlist;
    
    modlist: pmod;
    nummods: integer;
    
    SaveUText: Boolean;
    ComFlag: Boolean;
    commandfile: text;
    
    IsATerminal: Boolean; {Always TRUE except under UNISOFT}
    largc: integer;
    
    function _isatty(fd: longint): longint; cexternal; {UNISOFT}
    
{$I sglib.misc}

function rdlnkno: integer;
  var i: integer;
begin
i := nextword;
if i >= 0
then begin
     i := i + lprocbase;
     if lprocmax <= i
     then lprocmax := i + 1;
     end;
rdlnkno := i;
end; {rdlnkno}

procedure readlnkid(var fid: linkid);
  var len,i,junk: integer;
begin
fid.linkno := rdlnkno;
len := nextbyte;
if len <= 8 then fid.nam[0] := chr(len) else fid.nam[0] := chr(8);
for i := 1 to len do
    if i <= 8
    then fid.nam[i] := chr(nextbyte)
    else junk := nextbyte;
end; {readlnkid}

procedure phase1;
  var i,unitnumber: integer; thismod,unitmod: pmod;
      UnitFlag,InitFlag: Boolean; lint: pintlist;
  
  (* +----+ *)
  (* | 80 | *)
  (* +----+ *)
  
  procedure head1block;
    var blocksize,size: integer; lname,sname: alfa8; comment: string45;
        lmod: pmod; lint: pintlist;
  begin
  blocksize := next3bytes;
  reada8(lname);
  reada8(sname);
  size := next4bytes;
  reads45(comment,blocksize - 24);
  new(lmod);
  with lmod^ do begin
       modname := lname; next := nil; codesize := 0;
       idiskaddr := (inblock*256 + inword)*2 - blocksize;
       odiskaddr := 0; otextaddr := 0;
       itextaddr := 0; textsize := 0; iinitaddr := 0;
       othermods := nil;
       end;
  if UnitFlag
  then begin
       new(lint);
       with lint^ do
            begin int := unitnumber; next := nil; end;
       lmod^.othermods := lint;
       end;
  entermod(lmod);
  thismod := lmod;
  end; {head1block}
  
  (* +----+ *)
  (* | 81 | *)
  (* +----+ *)
  
  procedure end1block;
    var blocksize,codesize: integer;
  begin
  blocksize := next3bytes;
  codesize := next4bytes;
  end; {end1block}
  
  (* +----+ *)
  (* | 82 | *)
  (* +----+ *)
  
  procedure entry1block;
    var blocksize,lloc: integer; lname,uname: alfa8; comment: string45;
        lsymbol: psymbol;
  begin
  blocksize := next3bytes;
  readlnkname(lname); reada8(uname);
  lloc := next4bytes;
  reads45(comment,blocksize - 24);
  lsymbol := lookupname(lname);
  if lsymbol = nil
  then begin
       new(lsymbol);
       with lsymbol^ do
            begin linkname := lname; username := uname;
            llink := nil; rlink := nil; next := nil;
            Defined := TRUE; loc := lloc; reflist := nil;
            modnum := thismod^.modno;
            end;
       entername(lsymbol);
       end
  else with lsymbol^ do
            if Defined
            then writeln('\07Double defined ',lname)
            else begin
                 Defined := TRUE; loc := lloc;
                 modnum := thismod^.modno;
                 end;
  end; {entry1block}
  
  (* +----+ *)
  (* | 83 | *)
  (* +----+ *)
  
  procedure extern1block(ShortCall: Boolean);
    var blocksize,bytesleft,loc: integer; lname,uname: alfa8;
        lsymbol: psymbol; lref: pintlist;
  begin
  blocksize := next3bytes;
  readlnkname(lname); reada8(uname);
  lsymbol := lookupname(lname);
  if lsymbol = nil
  then begin
       new(lsymbol);
       with lsymbol^ do begin
            linkname := lname; username := uname;
            llink := nil; rlink := nil; next := nil;
            reflist := nil; Defined := FALSE;
            end;
       entername(lsymbol);
       end;
  bytesleft := blocksize - 20;
  while bytesleft > 0 do begin
        if ShortCall
        then begin loc := nextword; bytesleft := bytesleft - 2; end
        else begin loc := next4bytes; bytesleft := bytesleft - 4; end;
        end;
  new(lref);
  with lref^ do begin
       int := thismod^.modno;
       next := lsymbol^.reflist;
       lsymbol^.reflist := lref;
       end;
  end; {extern1block}
  
  (* +----+ *)
  (* | 84 | *)
  (* +----+ *)
  
  procedure start1block;
    var lloc,blocksize: integer; comment: string45;
  begin
  blocksize := next3bytes;
  lloc := next4bytes;
  reads45(comment,blocksize - 8);
  if StartFlag
  then writeln('\07*** Error - two start locations')
  else begin
       StartFlag := TRUE;
       startmod := thismod^.modno;
       end;
  end; {start1block}
  
  (* +----+ *)
  (* | 85 | *) (* Self-relocating *)
  (* +----+ *)
  
  procedure code1block;
    var blocksize,address: integer;
  begin
  blocksize := next3bytes;
  skip(blocksize - 4);
  end; {code1block}
  
  (* +----+ *)
  (* | 86 | *) (* Relocation Block *)
  (* +----+ *)
  
  procedure reloc1block;
    var blocksize,loc,i: integer; lpint: pintlist;
  begin
  blocksize := next3bytes;
  for i := 1 to (blocksize - 4) div 4 do begin
      loc := next4bytes;
      new(lpint); lpint^.next := nil; lpint^.int := loc;
      if reloclist = nil
      then reloclist := lpint
      else lastreloc^.next := lpint;
      lastreloc := lpint;
      end;
  end; {reloc1block}
  
  (* +----+ *)
  (* | 92 | *)
  (* +----+ *)
  
  procedure unit1block;
    var blocksize,caddr,taddr,tsize,iaddr: integer; uname: alfa8;
  begin
  blocksize := next3bytes;
  reada8(uname);
  caddr := next4bytes;
  taddr := next4bytes;
  tsize := next4bytes;
  iaddr := next4bytes;
  new(unitmod);
  with unitmod^ do begin
       modname := uname; next := nil; codesize := 0;
       idiskaddr := (inblock*256 + inword)*2 - blocksize;
       odiskaddr := 0; otextaddr := 0;
       itextaddr := taddr; textsize := tsize;
       iinitaddr := iaddr; othermods := nil;
       end;
  entermod(unitmod);
  UnitFlag := TRUE;
  unitnumber := unitmod^.modno;
  if iaddr <> 0 then InitFlag := TRUE;
  end; {unit1block}
  
  procedure typvar1block;
    var i,kind,ltypeno,len: integer; l: longint;
        PackedFlag: Boolean; lid: linkid;
  begin {typvar1block}
  l := next3bytes;
  
  { L.V.S.L LinkName PrntName }
  
  ignore(4); readlnkid(lid); readlnkid(lid);
  
  { UserName }
  
  len := nextbyte; ignore(len);
  
  { Types }
  
  ltypeno := nextword;
  while ltypeno <> 0 do begin
        incltype(ltypeno);
        kind := nextbyte; PackedFlag := kind > 15;
        case kind mod 16 of
          0: {SCALAR}
             ignore(2);
          1: {SUBRANGE}
             begin
             i := readtypeno; ignore(8);
             end;
          2, {POINTER}
          3: {SET}
             i := readtypeno;
          4: {ARRAY}
             begin
             i := readtypeno; i := readtypeno;
             if PackedFlag then i := nextbyte;
             end;
          5: {STRING}
             i := nextbyte;
          6: {FILE}
             i := readtypeno;
          7: {RECORD}
             begin
             ignore(4);
             len := nextbyte;
             while len > 0 do begin
                   ignore(len);
                   i := readtypeno; ignore(2);
                   if PackedFlag then ignore(2);
                   len := nextbyte;
                   end;
             end;
          9: {FCHAR}
             i := nextword;
         10: {FARRAY}
             begin
             len := nextbyte; i := readtypeno;
             ignore(len*13);
             end;
        otherwise: writeln('*** Bad $A0 Block ***');
        end; {case}
        ltypeno := nextword;
        end;
  
  { Variables }
  
  len := nextbyte;
  while len <> 0 do begin
        ignore(len);
        i := readtypeno;
        kind := nextbyte;
        case kind mod 16 of
          0,1: i := nextword;
          2,3: ;
          4:   begin
               i := nextword; len := nextbyte; ignore(len+4);
               end;
        end; {case}
        len := nextbyte;
        end;
  if odd(l) then i := nextbyte;
  end; {typvar1block}
  
begin {phase1}
HalfFlag := FALSE; UnitFlag := FALSE; InitFlag := FALSE;
i := nextbyte;
while i <> 0 do begin
      case i of
       $80: head1block;
       $81: end1block;
       $82: entry1block;
       $83: extern1block(FALSE);
       $84: start1block;
       $85: code1block;
       $86: reloc1block;
       $87, { comrefblock }
       $88, { comdefblock }
       $8A, { FDATAdefblock }
       $8B, { FDATAinitblock }
       $8C, { FDATArefblock }
       $8D, { FDATArelblock }
       $A1: { BPTblock }
            begin
            i := next3bytes;
            if odd(i) then i := i + 1;
            skip(i - 4);
            end;
       $89: extern1block(TRUE);
       $92: unit1block;
       $A0: typvar1block;
      otherwise:
           begin
           writeln('\07*** Object Code Format Error: $',i:2 hex,' ***');
           exit(phase1);
           end;
      end; {case}
      i := nextbyte;
      end;
if InitFlag
then begin
     new(lint); lint^.int := nummods - 1;
     lint^.next := unitmod^.othermods;
     unitmod^.othermods := lint;
     end;
end; {phase1}

procedure phase2;
  var i,count: integer; lname: alfa8; lastmod: longint;
      thismod,lmod,unitmod: pmod; UnitFlag: Boolean;
  
  
  { Add module A to module B's reference list }
  
  procedure modref(a,b: integer);
    var lmod: pmod; lpint: pintlist; MatchFlag: Boolean;
  begin
  lmod := modlist;
  while lmod^.modno <> b do
        lmod := lmod^.next;
  lpint := lmod^.othermods;
  MatchFlag := FALSE;
  while (lpint <> nil) and not MatchFlag do
        if lpint^.int = a
        then MatchFlag := TRUE
        else lpint := lpint^.next;
  if not MatchFlag
  then begin
       new(lpint);
       with lpint^ do
            begin int := a; next := lmod^.othermods; end;
       lmod^.othermods := lpint;
       end;
  end; {modref}
  
  
  { Compute the module reference relation }
  
  procedure walktree(fsymbol: psymbol);
    var lpint: pintlist;
  begin
  if fsymbol <> nil
  then with fsymbol^ do begin
       walktree(llink);
       if Defined
       then begin
            lpint := reflist;
            while lpint <> nil do
                  with lpint^ do begin
                       modref(modnum,int);
                       lpint := next;
                       end;
            end;
       walktree(rlink);
       end;
  end; {walktree}
  
  
  { Compute the transative closure of the module reference relation }
  
  procedure closure;
    var lmod,mmod: pmod; lpint,mpint: pintlist;
  begin
  lmod := modlist;
  while lmod <> nil do begin
        mmod := modlist;
        while mmod <> nil do begin
              mpint := mmod^.othermods;
              while mpint <> nil do
                    if mpint^.int = lmod^.modno
                    then begin
                         lpint := lmod^.othermods;
                         while lpint <> nil do begin
                               modref(lpint^.int,mmod^.modno);
                               lpint := lpint^.next;
                               end;
                         mpint := nil;
                         end
                    else mpint := mpint^.next;
              mmod := mmod^.next;
              end;
        lmod := lmod^.next;
        end;
  end; {closure}
  
  procedure out(a: integer);
  begin
  if outbyte > 511
  then begin
       if blockwrite(outfile,outbuffer,1,outblock) <> 1
       then writeln('\07Can''t write output file!');
       outblock := outblock + 1; outbyte := 0;
       end;
  outbuffer[outbyte] := chr(a);
  outbyte := outbyte + 1;
  end; {out}
  
  procedure out2(a: integer);
    var c: array[0..1] of -128..127;
  begin
  moveleft(a,c,2);
  if FlipBytes
  then begin out(c[1]); out(c[0]); end
  else begin out(c[0]); out(c[1]); end;
  end; {out2}
  
  procedure out3(a: longint);
    var c: array[0..3] of -128..127;
  begin
  moveleft(a,c,4);
  if FlipBytes
  then begin out(c[2]); out(c[1]); out(c[0]); end
  else begin out(c[1]); out(c[2]); out(c[3]); end;
  end; {out3}
  
  procedure out4(a: longint);
    var c: array[0..3] of -128..127;
  begin
  moveleft(a,c,4);
  if FlipBytes
  then begin out(c[3]); out(c[2]); out(c[1]); out(c[0]); end
  else begin out(c[0]); out(c[1]); out(c[2]); out(c[3]); end;
  end; {out4}
  
  procedure outalfa(fname: alfa8);
    var i: integer;
  begin
  for i := 1 to 8 do
      out(ord(fname[i]));
  end; {outalfa}
  
  procedure copyout(fbytes: integer);
    var i: integer;
  begin
  for i := 1 to fbytes do out(nextbyte);
  end; {copyout}
  
  procedure outentries(fsymbol: psymbol);
    var i: integer;
  begin
  if fsymbol <> nil
  then with fsymbol^ do begin
       outentries(llink);
       if Defined and not localname(linkname)
          then begin
               
               { library entry record }
               
               out(-111); out3(18); outalfa(linkname);
               out2(modnum); out4(loc);
               end;
       outentries(rlink);
       end;
  end; {outentries}
  
  procedure outmods(fmod: pmod);
    var i,count: integer; lpint: pintlist; lmod: pmod;
  begin
  lmod := fmod;
  while lmod <> nil do
        with lmod^ do begin
             count := 0; lpint := othermods;
             while lpint <> nil do
                   begin count := count + 1; lpint := lpint^.next; end;
             
             { library module record }
             
             out(-112); out3(count*2 + 30); outalfa(modname);
             out4(codesize); out4(odiskaddr); out4(otextaddr);
             out4(textsize); out2(count);
             lpint := othermods;
             while lpint <> nil do
                   begin out2(lpint^.int); lpint := lpint^.next; end;
             
             lmod := next;
             end;
  end; {outmods}
  
  procedure copymods;
    var i,blocksize,codesize: integer;
    
    procedure copyblock(tag,size: integer);
      var i: integer;
    begin
    out(tag); out3(size);
    for i := 5 to size do out(nextbyte);
    if odd(size) then out(nextbyte);
    end; {copyblock}
    
    (* +----+ *)
    (* | 82 | *)
    (* +----+ *)
    
    procedure entry2block;
      var blocksize,i: integer; lname: alfa8;
    begin
    blocksize := next3bytes;
    readlnkname(lname);
    out(130); out3(blocksize); outalfa(lname);
    for i := 13 to blocksize do
        out(nextbyte);
    end; {entry2block}
    
    (* +----+ *)
    (* | 83 | *)
    (* +----+ *)
    
    procedure extern2block(ShortCall: Boolean);
      var blocksize,i: integer; lname: alfa8;
    begin
    blocksize := next3bytes;
    readlnkname(lname);
    if ShortCall then out(137) else out(131);
    out3(blocksize); outalfa(lname);
    for i := 13 to blocksize do
        out(nextbyte);
    end; {extern2block}
    
    procedure outlid(fid: linkid);
      var i: integer;
    begin
    out2(fid.linkno);
    for i := 0 to length(fid.nam) do out(ord(fid.nam[i]));
    end; {outlid}
    
    procedure typvarcopy;
      var i,kind,ltypeno,len: integer; l: longint;
          PackedFlag: Boolean; lid: linkid;
    begin
    l := next3bytes;
    out($A0); out3(l);
    
    { L.V.S.L LinkName PrntName }
    
    out(nextbyte); i := nextbyte; len := nextbyte;
    if i*100 + len < 203
    then begin
         writeln('*** .DBG data for V',i,'.',len,' was encountered ***');
         writeln('*** but this library requires V2.3 ***');
         halt(-1);
         end;
    out(i); out(len); out(nextbyte);
    out2(rdlnkno); len := nextbyte; out(len); copyout(len);
    out2(rdlnkno); len := nextbyte; out(len); copyout(len);
    
    { UserName }
    
    len := nextbyte; out(len); copyout(len);
    
    { Types }
    
    ltypeno := nextword;
    while ltypeno <> 0 do begin
          incltype(ltypeno); out2(ltypeno);
          kind := nextbyte; out(kind); PackedFlag := kind > 15;
          case kind mod 16 of
            0: {SCALAR}
               out2(nextword);
            1: {SUBRANGE}
               begin
               out2(readtypeno); out4(next4bytes); out4(next4bytes);
               end;
            2, {POINTER}
            3: {SET}
               out2(readtypeno);
            4: {ARRAY}
               begin
               out2(readtypeno); out2(readtypeno);
               if PackedFlag then out(nextbyte);
               end;
            5: {STRING}
               out(nextbyte);
            6: {FILE}
               out2(readtypeno);
            7: {RECORD}
               begin
               out4(next4bytes);
               len := nextbyte;
               while len > 0 do begin
                     out(len); copyout(len);
                     out2(readtypeno); i := nextword; out2(i);
                     if i < 0 then out2(nextword);
                     if PackedFlag then out2(nextword);
                     len := nextbyte;
                     end;
               out(0);
               end;
            9: {FCHAR}
               out2(nextword);
           10: {FARRAY}
               begin
               len := nextbyte; out(len); out2(readtypeno);
               copyout(len*13);
               end;
          otherwise: writeln('*** Bad $A0 Block ***');
          end; {case}
          ltypeno := nextword;
          end;
    out2(0);
    
    { Variables }
    
    len := nextbyte;
    while len <> 0 do begin
          out(len); copyout(len);
          out2(readtypeno);
          kind := nextbyte; out(kind);
          case kind mod 16 of
            0,1,
            6,7: out2(nextword);
            2,3: ;
            4:   begin
                 out2(nextword); len := nextbyte; out(len); copyout(len);
                 out4(next4bytes);
                 end;
          end; {case}
          len := nextbyte;
          end;
    out(0);
    if odd(l) then out(nextbyte);
    end; {typvarcopy}
    
    procedure copybpts;
      var i,n: integer; lid: linkid; l: longint;
    begin
    l := next3bytes;
    out($A1); out3(l);
    n := nextword; out2(n);
    i := nextbyte; out(i);
    case i of
      0:   ;
      1,2: begin out2(nextword); out4(next4bytes); end;
    end; {case}
    for i := 1 to n do begin
        readlnkid(lid); outlid(lid);
        out4(next4bytes); out4(next4bytes); {Entry/Exit}
        end;
    n := nextword; out2(n);
    for i := 1 to n do out2(nextword);
    if odd(l) then out(nextbyte);
    end; {copybpts}
    
  begin {copymods}
  HalfFlag := FALSE;
  i := nextbyte;
  while i <> 0 do begin
        case i of
          $82: entry2block;
          $83: extern2block(FALSE);
          $89: extern2block(TRUE);
          $84, { startblock  }
          $85, { codeblock   }
          $86, { relocblock  }
          $87, { comrefblock }
          $88, { comdefblock }
          $8A, { FDATAdefblock }
          $8B, { FDATAinitblock }
          $8C, { FDATArefblock }
          $8D: { FDATArelblock }
               begin
               blocksize := next3bytes;
               copyblock(i,blocksize);
               end;
          $80: { headblock }
               begin
               blocksize := next3bytes;
               lastmod := ord4(outblock)*512 + outbyte;
               thismod^.odiskaddr := lastmod;
               copyblock(i,blocksize);
               end;
          $81: { endblock }
               begin
               blocksize := next3bytes;
               codesize := next4bytes;
               thismod^.codesize := codesize;
               thismod := thismod^.next;
               out(129); out3(8); out4(codesize);
               end;
          $92: { unitblock }
               begin
               UnitFlag := TRUE; unitmod := thismod;
               blocksize := next3bytes;
               skip(24);
            {--if thismod^.iinitaddr = 0
               then--} begin
                    thismod^.odiskaddr := ord4(outblock)*512 + outbyte;
                    { Module Name Block }
                    out(128); out3(24);
                    outalfa(thismod^.modname); outalfa('        ');
                    out4(0);
                    { End Block with no code }
                    out(129); out3(8); out4(0);
                    end;
               thismod := thismod^.next;
               end;
          $A0: typvarcopy;
          $A1: copybpts;
        otherwise:
               begin
               writeln('\07*** Object Code Format Error: $',i:2 hex,' ***');
               exit(copymods);
               end;
        end; {case}
        i := nextbyte;
        end;
  end; {copymods}
  
  procedure copytext;
    var i,blocksize,codesize,textblock: integer;
        lbuff: packed array[0..511] of char;
  begin
  HalfFlag := FALSE;
  i := nextbyte;
  while i <> 0 do begin
        case i of
          $80, { headblock }
          $82, { entryblock  }
          $83, { externblock }
          $84, { startblock  }
          $85, { codeblock   }
          $86, { relocblock  }
          $87, { comrefblock }
          $88, { comdefblock }
          $89, { externblock }
          $8A, { FDATAdefblock }
          $8B, { FDATAinitblock }
          $8C, { FDATArefblock }
          $8D, { FDATArelblock }
          $A0, { TYPVARblock }
          $A1: { BPTblock }
               begin
               blocksize := next3bytes;
               if odd(blocksize) then blocksize := blocksize + 1;
               skip(blocksize - 4);
               end;
          $81: { endblock }
               begin
               blocksize := next3bytes;
               codesize := next4bytes;
               thismod := thismod^.next;
               end;
          $92: { unitblock }
               begin
               blocksize := next3bytes;
               skip(24);
               textblock := thismod^.itextaddr div 512;
               thismod^.otextaddr := ord4(outblock)*512;
               for i := 1 to thismod^.textsize div 512 do begin
                   if blockread(infile,lbuff,1,textblock) <> 1
                   then begin
                        writeln('*** Can''t read text block ***');
                        exit(copytext);
                        end;
                   if blockwrite(outfile,lbuff,1,outblock) <> 1
                   then begin
                        writeln('*** Can''t write text block ***');
                        exit(copytext);
                        end;
                   outblock := outblock + 1;
                   textblock := textblock + 1;
                   end;
               thismod := thismod^.next;
               end;
          otherwise:
               begin
               writeln('\07*** Object Code Format Error: $',i:2 hex,' ***');
               exit(copytext);
               end;
        end; {case}
        i := nextbyte;
        end;
  end; {copytext}
  
  procedure patchmods(fmod: pmod);
    var lbyte: longint; i,count: integer; lpint: pintlist; lmod: pmod;
    
    procedure patchlong(fbyte: longint; fvalue: longint);
      
      procedure patchword(fbyte: longint; fvalue: integer);
        var k,hi,lo: integer;
      begin
      { fbyte must be even. }
      k :=  fbyte div 512;
      if k <> outblock
      then begin
           if blockwrite(outfile,outbuffer,1,outblock) <> 1 then ;
           outblock := k;
           if blockread(outfile,outbuffer,1,outblock) <> 1 then ;
           end;
      k := fbyte mod 512;
      splitbytes(fvalue,hi,lo);
      outbuffer[k] := chr(hi);
      outbuffer[k + 1] := chr(lo);
      end; {patchword}
      
    begin {patchlong}
    patchword(fbyte,fvalue div 65536);
    patchword(fbyte + 2,fvalue);
    end; {patchlong}
    
  begin {patchmods}
  lbyte := 0; outbyte := 0; outblock := 0;
  if blockread(outfile,outbuffer,1,outblock) <> 1 then ;
  lmod := fmod;
  while lmod <> nil do
        with lmod^ do begin
             
             { Patch code size, disk address, and text }
             { address in library module record. }
             
             patchlong(lbyte + 12,codesize);
             patchlong(lbyte + 16,odiskaddr);
             patchlong(lbyte + 20,otextaddr);
             count := 0;
             lpint := othermods;
             while lpint <> nil do
                   begin count := count + 1; lpint := lpint^.next; end;
             lbyte := lbyte + 30 + count*2;
             lmod := next;
             end;
  if blockwrite(outfile,outbuffer,1,outblock) <> 1 then ;
  end; {patchmods}
  
begin {phase2}
walktree(nametree);
closure;
outmods(modlist);
if StartFlag
then begin out(150); out3(6); out2(startmod); end;
outentries(nametree);

{ Copy module }

thismod := modlist; thisfile := files;
while thisfile <> nil do
      with thisfile^ do begin
           UnitFlag := FALSE;
           reset(infile,name);
           lprocbase := lbaseno;
           ltypebase := ltypeno;
           (* writeln('Reading ',name);*)
           inblock := -1; inword := 256;
           copymods;
           if UnitFlag
           then if unitmod^.odiskaddr = 0
                then unitmod^.odiskaddr := lastmod;
           close(infile);
           thisfile := next;
           end;

{ End-Of-File Mark: }

out(0);

while outbyte <= 511 do
      out(0);
out(0);

{ Copy Unit TEXT blocks }

if SaveUText
then begin
     thismod := modlist; thisfile := files;
     while thisfile <> nil do
           with thisfile^ do begin
                reset(infile,name);
                { ... only if unit text is there ??? ... }
                (*writeln('Copying interface text of ',name);*)
                inblock := -1; inword := 256;
                copytext;
                close(infile);
                thisfile := next;
                end;
     end;

patchmods(modlist);
end; {phase2}

procedure finalize;
begin
if ListFlag
then begin
     printsymbols(nametree);
     writeln(mapfile);
     if StartFlag
     then writeln(mapfile,'Starting Location in module ',startmod)
     else writeln(mapfile,'No Starting Location');
     close(mapfile,lock);
     end;
close(outfile,lock);
end; {finalize}

begin {library}
if ENVIRONMENT = UNISOFT 
then IsATerminal := _isatty(0) <> 0 
else IsATerminal := TRUE; 
initialize;
if (ENVIRONMENT = ELITE)
then prompt('Input file [#obj]',fname)
else prompt('Input file ',fname);
while length(fname) > 0 do begin
      reset(infile,fname);
      if ioresult = 0
      then begin
           new(lfile);
           with lfile^ do begin
                name := fname; lbaseno := lprocbase;
                ltypeno := ltypebase; next := nil;
                end;
           if files = nil
           then files := lfile
           else thisfile^.next := lfile;
           thisfile := lfile;
           inblock := -1; inword := 256;
           lprocmax := lprocbase; ltypemax := ltypebase;
           phase1;
           lprocbase := lprocmax; ltypebase := ltypemax;
           close(infile);
           end
      else writeln('\07Can''t open ''',fname,'''');
      if (ENVIRONMENT = ELITE)
      then prompt('Input file [#obj]',fname)
      else prompt('Input file ',fname);
      end;
phase2;
999: finalize;
end. {library}

