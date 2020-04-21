(****************************************************************************)
(*                                                                          *)
(*                        ASM68K - MC68000 Assembler                        *)
(*                                                                          *)
(*           (C) Copyright 1982,1985 Silicon Valley Software, Inc.          *)
(*                                                                          *)
(*                            All Rights Reserved.              06-Aug-85   *)
(*                                                                          *)
(****************************************************************************)


{$I-}
{$R-}

program asm68k;

label 999;

const DATE = '06-Aug-85';
      VERSION = '2.4';
      TITLE = 'ASM68K - M68000 Assembler';
      TITLE20 = 'ASM68K - M68020 Assembler';
      COPYRIGHT = '(C) Copyright 1982, 1985 Silicon Valley Software, Inc.';
      M68020 = TRUE;
      MAXCODEW = 2048;
      MAXFILES = 3;
      ESC = '\1b';  TAB = '\09';
      
      MERLIN = 0; UNISOFT = 2; CROMIX = 4; ADVENTURE = 6; CPM = 8;
      
      ENVIRONMENT = UNISOFT;
      
      { Merlin 
      SRCSUFFIX = '.text';
      ARGCBASE = 0;
      {}
      
      { Unisoft }
      SRCSUFFIX = '.asm';
      ARGCBASE = 1;
      {}
      
      { Cromix 
      SRCSUFFIX = '.asm';
      ARGCBASE = 0;
      {}
      
      { Adventure 
      SRCSUFFIX = '.asm'; (* Include files will be .src *)
      ARGCBASE = 0;
      {}

      { CPM 
      SRCSUFFIX = '.asm';
      ARGCBASE = 1;
      {}
      
type string30 = string[30];
     alfa8 = packed array[1..8] of char;
     symbol = (IDENTSY,OPSY,AREGSY,DREGSY,FREGSY,SREGSY,CCREGSY,USPREGSY,PCSY,
               STRINGSY,PAOCSY,INTSY,PLUSSY,MINUSSY,STARSY,SLASHSY,
               ANDSY,ORSY,NOTSY,COMMASY,HASHSY,LPARENSY,RPARENSY,DOTSY,
               EOLNSY,EOFSY,ERRORSY);
     idsymbol = IDENTSY..PCSY;
     valueclass = (ABSOLUTE,RELOCATABLE,EXTERN,STRINGVAL,PAOCVAL);
     identclass = ABSOLUTE..EXTERN;
     dataclass = (ABS1,ABS2,ABS4,REL2,REL4,EXT2,EXT4,CHR1,FILL);
     sizes = (BYTE,WORD,LONG,SNGL,DBLE,XTND,PBCD);
     opsizes = (NONE,B,W,L,S,WL,BWL);
     regrange = 0..7;
     pintlist = ^intlist;
     intlist = record intval: integer;
                      nextint: pintlist;
               end;
     psy = ^sy;
     sy = record name: alfa8; llink,rlink: psy;
                 case sykind: idsymbol of
                   SREGSY,
                   CCREGSY,
                   USPREGSY,
                   PCSY:     ();
                   AREGSY,
                   DREGSY,
                   FREGSY:   (regno: regrange);
                   OPSY:     (opclass: 0..63; opvalue: integer;
                              opsize: opsizes);
                   IDENTSY:  (Global: Boolean; EquNoVal: Boolean;
                               case Defined: Boolean of
                                 TRUE: (Double: Boolean;
                                         case idclass: identclass of
                                           ABSOLUTE,
                                           RELOCATABLE: (value: longint);
                                           EXTERN:      (sreflist,
                                                         lreflist: pintlist)));
                 end;
     datatype = record case cls: dataclass of
                         ABS1,ABS2,ABS4,
                         REL2,REL4,CHR1: (val: longint);
                         EXT2,EXT4:      (sym: psy);
                       end;
     attr = record case akind: valueclass of
                     ABSOLUTE,RELOCATABLE:
                       (avalue: longint);
                     EXTERN:
                       (evalue: psy);
                     STRINGVAL:
                       (svalue: string[64]);
            end;
     effamodes = (ALL,ALT,ALTMEM,DAT,CTR,ALTDAT,
                  DATBUTIMM,CTRPRE,CTRPOST,IMMDATA);
     setof31 = set of 0..31;
     pequvalrec = ^equvalrec;
     equvalrec = record next: pequvalrec;
                        esym: psy;
                        equexpr: packed array[1..64] of char;
                 end;
     
var optree,symboltree: psy;
    pc: longint;
    lineno,pageno,lineonpage,errors,pass,linesperpage,oldlineno: integer;
    objfile: file;
    lfile: text;
    infilename,objfilename,lfilename: string30;
    objbuff: array[0..255] of integer;
    objblock,objword: integer;
    codebuff: array[0..MAXCODEW] of integer;
    codeword,codebase,nextaddress,byteval: integer;
    ByteFlag: Boolean;
    modulename,segmentname: alfa8;
    ListFlag,ListOpen,CodeOpen,SourceOpen,
    PageSetFlag,CommentFlag,LabelFlag,EOFFlag,LongExts,
    MakeUpper,MakeLower,OldMakeUpper,OldMakeLower:Boolean;
    labelname: alfa8;
    ch: char;
    scanname,ucscanname: alfa8;
    scanint: longint;
    scanop,scanreg,scanclass: integer;
    scanstring: string[64];
    scansize: opsizes;
    instsize: sizes;
    StringOK: Boolean;
    extension: char;
    token: symbol;
    line: packed array[1..128] of char;
    maxcolumn,column,lasttokcol,lasterrcol: integer;
    data: array[1..64] of datatype;
    gattr: attr;
    fsizebits: array[sizes] of integer;
    sizebits: array[sizes] of integer;

    { Begin UCSD Dependent I/O }
    
    infile: file;
    topoffilestack: integer;
    filestack: array[0..MAXFILES] of
                 record lastrelblkread: integer;
                        nextchtoread: integer;
                        fname: string30;
                        flineno: integer;
                 end;
    inbufp: integer;
    inbuf: array[0..1025] of -128..127;
    
    { End UCSD Dependent I/O }
    
    NoMore: Boolean;
    StartFlag,StartBefore: Boolean;
    startloc: longint;
    reloclist: pintlist;
    UndefExpr: Boolean;
    undefequlist: pequvalrec;
    curlineno: integer;
    
procedure error(n: integer); forward;

procedure entername(fpsy: psy); forward;

procedure fillinbuf; forward;

procedure suffix(var fname: string30; suf: string30); forward;

procedure lowercase(var fname: string30); forward;

(*$I asm68k.int *)
(*$I asm68k.msc *)

procedure expression;
    var lattr: attr;
        s1,s2: record case Boolean of
                        TRUE:  (s: setof31);
                        FALSE: (i: longint);
                      end;

    procedure factor;
        var lpsy: psy; i: integer;
    begin
    if token = INTSY
    then begin
         gattr.akind := ABSOLUTE;
         gattr.avalue := scanint;
         scan;
         end
    
    else if token = IDENTSY
         then begin
              lpsy := lookupname(symboltree,scanname);
              if lpsy <> nil
              then begin
                   if lpsy^.Defined
                   then begin
                        gattr.akind := lpsy^.idclass;
                        if gattr.akind = EXTERN
                        then gattr.evalue := lpsy
                        else gattr.avalue := lpsy^.value;
                        if lpsy^.EquNoVal then UndefExpr := TRUE;
                        end
                   else begin
                        gattr.akind := ABSOLUTE;
                        gattr.avalue := 0;
                        UndefExpr := TRUE;
                        end;
                   if lpsy^.Double then error(14);
                   end
              else begin
                   gattr.akind := ABSOLUTE;
                   gattr.avalue := 0;
                   UndefExpr := TRUE;
                   error(7);
                   end;
              scan;
              end
         
         else if token = STRINGSY
              then begin
                   if StringOK
                   then begin
                        gattr.akind := STRINGVAL;
                        gattr.svalue := scanstring;
                        end
                   else if length(scanstring) <= 3
                        then begin
                             gattr.akind := ABSOLUTE;
                             gattr.avalue := length(scanstring);
                             for i := 1 to length(scanstring) do
                                 gattr.avalue := gattr.avalue*256 +
                                     ord(scanstring[i]);
                             end
                        else error(22);
                   scan;
                   end
                   
              else if token = PAOCSY
                   then begin
                        if StringOK
                        then begin
                             gattr.akind := PAOCVAL;
                             gattr.svalue := scanstring;
                             end
                        else if length(scanstring) <= 4
                             then begin
                                  gattr.akind := ABSOLUTE;
                                  gattr.avalue := 0;
                                  for i := 1 to length(scanstring) do
                                      gattr.avalue := gattr.avalue*256 +
                                          ord(scanstring[i]);
                                  end
                             else error(22);
                        scan;
                        end
                        
                   else if token = LPARENSY
                        then begin
                             scan;
                             expression;
                             if token = RPARENSY then scan else error(5);
                             end
                        
                        else if token = PCSY
                             then begin
                                  scan;
                                  gattr.akind := RELOCATABLE;
                                  gattr.avalue := pc;
                                  end
                             
                             else begin
                                  error(6);
                                  gattr.akind := ABSOLUTE;
                                  gattr.avalue := 0;
                                  end;
    end; {factor}
    
    procedure term;
        var lattr: attr; ltoken: symbol; Signed,Negative: Boolean;
    begin
    if (token = PLUSSY) or (token = MINUSSY)
    then begin
         Signed := TRUE;
         Negative := token = MINUSSY;
         scan;
         end
    else Signed := FALSE;
    factor;
    while (token = STARSY) or (token = SLASHSY) do
          begin
          ltoken := token;
          lattr := gattr;
          scan;
          factor;
          case ltoken of
               STARSY:  begin
                        gattr.avalue := gattr.avalue * lattr.avalue;
                        if (lattr.akind <> ABSOLUTE) or
                           (gattr.akind <> ABSOLUTE)
                        then error(4);
                        end;
               SLASHSY: begin
                        gattr.avalue := lattr.avalue div gattr.avalue;
                        if (lattr.akind <> ABSOLUTE) or
                           (gattr.akind <> ABSOLUTE)
                        then error(4);
                        end;
          end; {case}
          end;
    if Signed
       then begin
            if gattr.akind <> ABSOLUTE then error(4);
            if Negative then gattr.avalue := -gattr.avalue;
            end;
    end; {term}
    
    procedure simpleexpression;
        var lattr: attr; ltoken: symbol;
    begin
    term;
    while (token = PLUSSY) or (token = MINUSSY) do
          begin
          ltoken := token;
          lattr := gattr;
          scan;
          term;
          case ltoken of
               PLUSSY:  begin
                        gattr.avalue := gattr.avalue + lattr.avalue;
                        case gattr.akind of
                           ABSOLUTE:    if lattr.akind = RELOCATABLE
                                        then gattr.akind := RELOCATABLE
                                        else if lattr.akind <> ABSOLUTE
                                             then error(9);
                           RELOCATABLE: if lattr.akind <> ABSOLUTE
                                        then error(4);
                           EXTERN,
                           STRINGVAL:   error(9);
                        end; {case}
                        end;
               MINUSSY: begin
                        gattr.avalue := lattr.avalue - gattr.avalue;
                        case lattr.akind of
                           ABSOLUTE:    if gattr.akind <> ABSOLUTE
                                        then error(4);
                           RELOCATABLE: if gattr.akind = RELOCATABLE
                                        then gattr.akind := ABSOLUTE
                                        else if gattr.akind = ABSOLUTE
                                             then gattr.akind := RELOCATABLE
                                             else error(4);
                           EXTERN,
                           STRINGVAL:   error(4);
                        end; {case}
                        end;
          end; {case}
          end;
    end; {simpleexpression}
    
    procedure lfactor;
        var NotFlag: Boolean;
    begin
    NotFlag := token = NOTSY;
    if NotFlag then scan;
    simpleexpression;
    if NotFlag
    then if gattr.akind = ABSOLUTE
         then gattr.avalue := -gattr.avalue - 1
         else error(4);
    end; {lfactor}
    
    procedure lterm;
        var lattr: attr;
            s1,s2: record case Boolean of
                            TRUE:  (s: setof31);
                            FALSE: (i: longint);
                          end;
    begin
    lfactor;
    while token = ANDSY do begin
          if gattr.akind <> ABSOLUTE
          then error(4);
          scan; lattr := gattr; lfactor;
          if gattr.akind <> ABSOLUTE
          then error(4);
          s1.i := gattr.avalue; s2.i := lattr.avalue;
          s1.s := s1.s*s2.s;
          gattr.avalue := s1.i; gattr.akind := ABSOLUTE;
          end;
    end; {lterm}
    
begin {expression}
lterm;
while token = ORSY do begin
      if gattr.akind <> ABSOLUTE
      then error(4);
      scan; lattr := gattr; lterm;
      if gattr.akind <> ABSOLUTE
      then error(4);
      s1.i := gattr.avalue; s2.i := lattr.avalue;
      s1.s := s1.s + s2.s;
      gattr.avalue := s1.i; gattr.akind := ABSOLUTE;
      end;
end; {expression}

procedure effaddr(fsize: sizes; var fmode,freg,fisize: integer;
                  legalmodes: effamodes);
    var lisize,lreg,oldcolumn,hi,lo: integer; BadMode: Boolean;
    
    procedure effa;
        var sizebit,offset,hi,lo,lreg: integer; lclass: dataclass;
            lint: pintlist;
    begin
    expression;
    if token = LPARENSY
    then begin
         scan;
         fisize := fisize + 2;
         lisize := fisize div 2;
         if gattr.akind = RELOCATABLE
         then begin
              if token = AREGSY
              then lreg := scanreg + 8
              else if token = DREGSY
                   then lreg := scanreg
                   else error(19);
              scan;
              sizebit := 0;
              if token = DOTSY
              then begin
                   scan;
                   if token = IDENTSY
                   then begin
                        if ucscanname <> 'W       '
                        then if ucscanname = 'L       '
                             then sizebit := 2048
                             else error(15);
                        scan;
                        end
                   else error(15);
                   end;
              fmode := 7; freg := 3;
              gethilo(gattr.avalue - pc - fisize + 2,hi,lo);
              offset := lo;
              if (offset < -128) or (offset > 127)
              then error(20);
              if offset < 0
              then offset := offset + 256;
              data[lisize].val := lreg*4096 + sizebit + offset;
              data[lisize].cls := REL2;
              end
         else begin
              if token = AREGSY
              then freg := scanreg
              else error(16);
              scan;
              if token = DOTSY
              then begin
                   { Caused by semantic controlled parsing }
                   { Required so pass 1 = pass 2 }
                   error(25);
                   scan;
                   if token = IDENTSY then scan;
                   end;
              if token = COMMASY
              then begin
                   scan;
                   if token = DREGSY
                   then begin
                        lreg := scanreg;
                        scan;
                        end
                   else if token = AREGSY
                        then begin
                             lreg := scanreg + 8;
                             scan;
                             end
                        else error(19);
                   sizebit := 0;
                   if token = DOTSY
                   then begin
                        scan;
                        if token = IDENTSY
                        then begin
                             if ucscanname <> 'W       '
                             then if ucscanname = 'L       '
                                  then sizebit := 2048
                                  else error(15);
                             scan;
                             end
                        else error(15);
                        end;
                   fmode := 6;
                   if gattr.akind <> ABSOLUTE
                   then error(4);
                   gethilo(gattr.avalue,hi,lo);
                   offset := lo;
                   if (offset < -128) or (offset > 127)
                   then error(20);
                   if offset < 0
                   then offset := offset + 256;
                   data[lisize].val := lreg*4096 + sizebit + offset;
                   data[lisize].cls := ABS2;
                   end
              else begin
                   fmode := 5;
                   gethilo(gattr.avalue,hi,lo);
                   data[lisize].val := lo;
                   data[lisize].cls := getdatacls(gattr.akind);
                   end;
              end;
         if token = RPARENSY
         then scan
         else error(17);
         end
    else if token = DOTSY
         then begin
              fmode := 7;
              scan;
              if token = IDENTSY
              then begin
                   gethilo(gattr.avalue,hi,lo);
                   if ucscanname = 'L       '
                   then begin
                        freg := 1;
                        fisize := fisize + 2;
                        lisize := fisize div 2;
                        data[lisize].val := hi;
                        data[lisize].cls := ABS2;
                        end
                   else freg := 0;
                   fisize := fisize + 2;
                   lisize := fisize div 2;
                   data[lisize].val := lo;
                   if gattr.akind = RELOCATABLE
                   then if freg = 0
                        then error(38)
                        else if pass = 2
                             then begin
                                  new(lint);
                                  lint^.intval := pc + fisize - 4;
                                  lint^.nextint := reloclist;
                                  reloclist := lint;
                                  end;
                   lclass := getdatacls(gattr.akind);
                   data[lisize].cls := lclass;
                   {!!!} if lclass = EXT2 then error(8);
                   if (ucscanname <> 'W       ') and
                      (ucscanname <> 'L       ')
                   then error(15);
                   scan;
                   end
              else error(15);
              end
         else begin
              if gattr.akind = ABSOLUTE
              then error(40)
              else if (gattr.akind <> RELOCATABLE) and (gattr.akind <> EXTERN)
                   then error(21);
              fmode := 7;
              if LongExts and (gattr.akind = EXTERN)
              then freg := 1 else freg := 2;
              fisize := fisize + 2;
              lisize := fisize div 2;
              lclass := getdatacls(gattr.akind);
              data[lisize].cls := lclass;
              if lclass = EXT2
              then data[lisize].sym := gattr.evalue
              else begin
                   gethilo(gattr.avalue - (pc + fisize - 2),hi,lo);
                   data[lisize].val := lo;
                   end;
              if LongExts and (gattr.akind = EXTERN)
              then begin
                   fisize := fisize + 2;
                   lisize := fisize div 2;
                   data[lisize].val := 0;
                   data[lisize].cls := ABS2;
                   end;
              end;
    end; {effa}
    
begin {effaddr}
BadMode := FALSE;
if (token = DREGSY) or (token = AREGSY)
then begin
     if token = DREGSY
     then fmode := 0
     else begin
          fmode := 1;
          if fsize = BYTE then BadMode := TRUE;
          end;
     freg := scanreg;
     scan;
     end
else if token = HASHSY
     then begin
          scan;
          expression;
          if gattr.akind <> ABSOLUTE then error(4);
          gethilo(gattr.avalue,hi,lo);
          {::} if (fsize = DBLE) or (fsize = PBCD) or (fsize = XTND)
          {::} then error(99);
          if (fsize = LONG) or (fsize = SNGL)
          then begin
               fisize := fisize + 2;
               lisize := fisize div 2;
               data[lisize].val := hi;
               data[lisize].cls := ABS2;
               end;
          fisize := fisize + 2;
          lisize := fisize div 2;
          data[lisize].val := lo;
          data[lisize].cls := ABS2;
          fmode := 7;
          freg := 4;
          end
     else if token = LPARENSY
          then begin
               oldcolumn := column;
               scan;
               if token = AREGSY
               then begin
                    freg := scanreg;
                    scan;
                    if token = RPARENSY then scan else error(17);
                    if token = PLUSSY
                    then begin scan; fmode := 3; end
                    else fmode := 2;
                    end
               else begin
                    token := LPARENSY;
                    column := oldcolumn;
                    effa;
                    end;
               end
          else if token = MINUSSY
               then begin
                    oldcolumn := column;
                    scan;
                    if token = LPARENSY
                    then begin
                         scan;
                         if token = AREGSY
                         then begin
                              freg := scanreg;
                              fmode := 4;
                              scan;
                              if token = RPARENSY then scan else error(17);
                              end
                         else begin
                              token := MINUSSY;
                              column := oldcolumn;
                              effa;
                              end;
                         end
                    else begin
                         token := MINUSSY;
                         column := oldcolumn;
                         effa;
                         end;
                    end
               else effa;
if not BadMode
then case fmode of
       0: BadMode := legalmodes in [ALTMEM,CTR,CTRPRE,CTRPOST,IMMDATA];
       1: BadMode := legalmodes >= ALTMEM;
       2,
       5,
       6: BadMode := legalmodes = IMMDATA;
       3: BadMode := legalmodes in [CTR,CTRPRE,IMMDATA];
       4: BadMode := legalmodes in [CTR,CTRPOST,IMMDATA];
       7: case freg of
            0,1: BadMode := legalmodes = IMMDATA;
            2,3: BadMode := legalmodes in [ALT,ALTMEM,ALTDAT,IMMDATA];
            4:   BadMode := legalmodes in [ALT,ALTMEM,CTR..CTRPOST];
          end; {case}
     end; {case}
if BadMode then error(25);
end; {effaddr}

procedure middleize;
    var hi,lo: integer; NewEqu: Boolean; lequ,mequ,nequ: pequvalrec;
    
    procedure globblocks(fpsy: psy);
        var lpint: pintlist; count,hi,lo: integer;
    begin
    if fpsy <> nil
    then with fpsy^ do
              begin
              globblocks(llink);
              if sykind = IDENTSY
              then if Defined
                   then if Global
                        then begin { 82 }
                             outword(-32256); outword(24);
                             outname(name); outname(name);
                             gethilo(value,hi,lo);
                             outword(hi); outword(lo);
                             end;
              globblocks(rlink);
              end;
    end; {globblocks}
    
begin {middleize}
EOFFlag := FALSE;
repeat
       NewEqu := FALSE;
       lequ := undefequlist; nequ := nil;
       while lequ <> nil do begin
             moveleft(lequ^.equexpr,line,64);
             maxcolumn := 64; column := 1; scan;
             UndefExpr := FALSE; expression;
             if UndefExpr or (token <> EOLNSY)
                then begin
                     mequ := lequ; lequ := lequ^.next;
                     mequ^.next := nequ; nequ := mequ;
                     end
                else begin
                     with lequ^.esym^ do begin
                          idclass := gattr.akind;
                          value := gattr.avalue;
                          EquNoVal := FALSE;
                          end;
                     NewEqu := TRUE; lequ := lequ^.next;
                     end;
             end;
       undefequlist := nequ;
until not NewEqu;

close(infile);
reset(infile,infilename);
if ioresult <> 0
then begin
     writeln(chr(7),'*** Unable to re-open file ',infilename);
     goto 999;
     end;

(* Module Block *)

outword(-32767-1); outword(24);
outname(modulename); outname(segmentname); outword(0); outword(pc);

(* Starting Address Block *)

if StartFlag
   then begin
        outword(-31744); outword(12);
        gethilo(startloc,hi,lo);
        outword(hi); outword(lo);
        outword(0); outword(0);
        end;

(* Global Names Blocks *)

globblocks(symboltree);
errors := 0; pc := 0; lineno := oldlineno; EOFFlag := FALSE;
end; {middleize}

procedure dumprefs;
  var hi,lo,count: integer; lint: pintlist;
  
  procedure outextrefs(fsy: psy);
    var i,j,count: integer; lint: pintlist;
  begin
  if fsy <> nil
  then with fsy^ do
            begin
            outextrefs(llink);
            if sykind = IDENTSY
            then if Defined and (idclass = EXTERN)
                 then begin
                      if sreflist <> nil
                      then begin
                           count := 0; lint := sreflist;
                           while lint <> nil do
                                 begin
                                 count := count + 2;
                                 lint := lint^.nextint;
                                 end;
                           outword(-30464); outword(20 + count);
                           for i := 0 to 1 do
                               for j := 1 to 4 do
                                   outword(ord(name[j*2-1])*256 +
                                           ord(name[j*2]));
                           lint := sreflist;
                           while lint <> nil do
                                 begin
                                 outword(lint^.intval);
                                 lint := lint^.nextint;
                                 end;
                           end;
                      if lreflist <> nil
                      then begin
                           count := 0; lint := lreflist;
                           while lint <> nil do
                                 begin
                                 count := count + 4;
                                 lint := lint^.nextint;
                                 end;
                           outword(-32000); outword(20 + count);
                           for i := 0 to 1 do
                               for j := 1 to 4 do
                                   outword(ord(name[j*2-1])*256 +
                                           ord(name[j*2]));
                           lint := lreflist;
                           while lint <> nil do
                                 begin
                                 outword(0); outword(lint^.intval);
                                 lint := lint^.nextint;
                                 end;
                           end;
                      end;
            outextrefs(rlink);
            end;
  end; {outextrefs}
  
begin {dumprefs}
if (codeword > 0) or ByteFlag
   then outcode;

(* Relocation List *)

if reloclist <> nil
then begin
     count := 0; lint := reloclist;
     while lint <> nil do
           begin
           count := count + 4;
           lint := lint^.nextint;
           end;
     outword(-31232); outword(4 + count);
     lint := reloclist;
     while lint <> nil do
           begin
           outword(0); outword(lint^.intval);
           lint := lint^.nextint;
           end;
     end;
   
(* External References *)

outextrefs(symboltree);
   
(* End Block *)

if odd(pc) then pc := pc + 1;
gethilo(pc,hi,lo);
outword(-32512); outword(8); outword(hi); outword(lo);
end; {dumprefs}

procedure finalize;
begin
if CodeOpen
   then begin
        if errors = 0
           then begin (* End-of-file Mark *)
                outword(0); flushobj;
                close(objfile,LOCK);
                end
           else close(objfile,PURGE);
        end;
if ListOpen
   then begin
        room(2); writeln(lfile);
        writeln(lfile,errors,' errors.  ',lineno,' lines.  File ',
                infilename);
        finishpage;
        close(lfile,LOCK);
        end;
writeln(errors,' errors.  ',lineno,' lines.  File ',infilename);
if errors > 0 then halt(-1);
end; {finalize}

(*$I asm68k.p1 *)
(*$S PASS2 *)
(*$I asm68k.p2 *)


begin {asm68k}
initialize;
pass1;
repeat
       middleize;
       pass2;
       if ListOpen
       then printtree(symboltree);
       pass1;
until NoMore;
999: finalize;
end. {asm68k}

