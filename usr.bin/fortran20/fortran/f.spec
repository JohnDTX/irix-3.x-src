(*****************************************************************************)
(*                                                                           *)
(*                             File: F.SPEC.TEXT                             *)
(*                                                                           *)
(*           (C) Copyright 1982, 1985 Silicon Valley Software, Inc.          *)
(*                                                                           *)
(*                            All Rights Reserved.               29-Jul-85   *)
(*                                                                           *)
(*****************************************************************************)


procedure declstmts;
  var ExitFlag,FirstTime: Boolean;
  
  procedure paramlist(var farglist: pnlist; var faltrets: integer);
    var lpn: pn; newarg,lastarg: pnlist; ErrorFlag: Boolean;
        llabp: plabelrec;
  begin
  farglist := nil; faltrets := 0;
  if token = LPARENSY
  then begin
    fscan;
    if token = RPARENSY
    then fscan
    else begin
      repeat
        ErrorFlag := FALSE;
        if token = IDENTSY
        then begin
          newn(lpn,UNKN,ident); xref(lpn);
          entername(lpn,localtree);
          end
        else
          if token = STARSY
          then begin
            if blockkind = FUNBLOCK then error(244);
            faltrets := faltrets + 1;
            newn(lpn,LABL,'      '); lpn^.idtype.typ := INTEGR;
            new(llabp); lpn^.labp := llabp; llabp^.ilabno := faltrets;
            end
          else ErrorFlag := TRUE;
        if not ErrorFlag
        then begin
          lpn^.Indirect := TRUE; fscan;
          new(newarg); newarg^.next := nil; newarg^.nrec := lpn;
	  {  Added by jimt  at SGI for dbx  }
	  lpn^.isparam := TRUE;
          if farglist = nil
          then farglist := newarg
          else lastarg^.next := newarg;
          lastarg := newarg;
          if token = COMMASY
          then fscan
          else ErrorFlag := token <> RPARENSY;
          end;
        if ErrorFlag
        then begin error(243); skip([RPARENSY,IDENTSY,STARSY]); end;
      until (token = RPARENSY) or (token = EOLNSY);
      if token = RPARENSY then fscan else error(26);
      maxaltreturn := faltrets;
      end;
    end;
  end; {paramlist}
  
  procedure procstmt(funtype: types;
                     funsize: integer;
                     ExplicType: Boolean;
                     prockind: idkind);
    var lpn,lpn2: pn; larglist: pnlist; laltrets: integer;
  begin
  needstate(HEADSTATE,HEADSTATE);
  if prockind = FUNCT
  then blockkind := FUNBLOCK
  else blockkind := SUBBLOCK;
  fscan;
  if token = IDENTSY
  then begin procname := ident; fscan; end
  else begin error(28); procname := 'NONAME'; end;
  lpn2 := lookupname(globaltree,procname);
  if lpn2 = nil
  then
    begin
    newn(lpn,prockind,procname);
    with lpn^ do begin
      if ExplicType
      then begin
        idtype.typ := funtype; idtype.siz := funsize; ExpType := TRUE;
        end;
      end;
    makeglobal(lpn);
    end
  else { Found it in globaltree }
    begin
    if lpn2^.nkind = COMMON
    then
      begin
      error(250); newn(lpn,prockind,procname);
      end
    else { Assume it is FUNCT or SUBR }
      begin
      new(lpn); lpn^ := lpn2^; lpn^.llink := nil; lpn^.rlink := nil;
      if lpn2^.NDefined
      then error(85)
      else makeglobal(lpn);
      if ExplicType
      then
        begin
        lpn^.ExpType := TRUE;
        if (lpn^.idtype.typ <> funtype) or (lpn^.idtype.siz <> funsize)
        then
          if (funtype <> CHARACTER) or (funsize <> 0)
          then error(84);
        end;
      end;
    end;
  lpn^.NDefined := TRUE; lpn^.RtnLocal := TRUE;
  localtree := lpn; xref(lpn);
  if (prockind = FUNCT) and (token <> LPARENSY) then error(24);
  paramlist(larglist,laltrets);
  lpn^.arglist := larglist; lpn^.numaltret := laltrets;
  end; {procstmt}
  
  procedure progstmt;
    var lpn,lpn2: pn;
  begin
  needstate(HEADSTATE,HEADSTATE);
  blockkind := PROGBLOCK;
  fscan;
  if token = IDENTSY
  then begin procname := ident; fscan; end
  else begin error(28); procname := 'NONAME'; end;
  newn(lpn,PROG,procname); lpn^.rtnlev := 1; localtree := lpn; xref(lpn);
  lpn2 := lookupname(globaltree,procname);
  if lpn2 = nil
  then makeglobal(lpn)
  else error(34);
  end; {progstmt}
  
  procedure blockstmt;
    var lpn,lpn2: pn;
  begin
  needstate(HEADSTATE,HEADSTATE);
  blockkind := BLOCKBLOCK;
  fscan;
  if token = IDENTSY
  then begin procname := ident; fscan; end
  else procname := ' -----';
  newn(lpn,BLOCKD,procname); localtree := lpn; xref(lpn);
  lpn2 := lookupname(globaltree,procname);
  if lpn2 = nil
  then makeglobal(lpn)
  else error(34);
  end; {blockstmt}
  
  procedure verifysize(ftype: types; initsize,newsize: integer;
                       var fsize: integer);
  begin
  case ftype of
       INTEGR,
       LOGICAL:   if newsize in [1,2,4]
                  then fsize := newsize
                  else error(204);
       REEL:      if initsize = 8 {DOUBLE}
                  then error(205)
                  else
                    if (newsize = 4) or (newsize = 8)
                    then fsize := newsize
                    else error(206);
       CHARACTER: if (newsize > 0) and (newsize <= MAXCHAR)
                  then fsize := newsize
                  else error(98);
       COMPLEX:   error(207);
  end; {case}
  end; {verifysize}
  
  procedure implstmt;
    var itype: types; vlength,i: integer; Exit2Flag: Boolean;
        firstch,lastch,c: char;
  begin
  needstate(IMPSTATE,IMPSTATE);
  repeat
    if Match('INTEGER')
    then begin itype := INTEGR; vlength := defisize; end
    else
      if Match('REAL')
      then begin itype := REEL; vlength := 4; end
      else
        if Match('LOGICAL')
        then begin itype := LOGICAL; vlength := deflsize; end
        else
          if Match('CHARACTER')
          then begin vlength := 1; itype := CHARACTER; end
          else
            if Match('COMPLEX')
            then begin itype := COMPLEX; vlength := 8; end
            else
              if Match('DOUBLEPRECISION')
              then begin itype := REEL; vlength := 8; end
              else begin error(21); itype := INTEGR; end;
    fscan;
    if token = STARSY
    then begin
      if itype <> CHARACTER then notansi;
      fscan; i := 4;
      if token = LPARENSY
      then begin
        if itype <> CHARACTER then error(208);
        fscan; cstexpression;
        if gattr.atype.typ = INTEGR
        then i := gattr.treeptr^.cstvalu.ival
        else error(209);
        if token = RPARENSY then fscan else error(26);
        end
      else
        if token = ICONSTSY
        then begin i := intval; fscan; end
        else error(210);
      verifysize(itype,vlength,i,vlength);
      end;
    if token = LPARENSY then fscan else error(24);
    Exit2Flag := FALSE;
    repeat
      if token = IDENTSY
      then begin
        firstch := ident[1];
        if length(ident) > 1
        then error(27)
        else
          if (firstch < 'A') or (firstch > 'Z')
          then begin error(27); firstch := 'Z'; end;
        lastch := firstch; fscan;
        if token = MINUSSY
        then begin
          fscan;
          if token = IDENTSY
          then begin
            lastch := ident[1];
            if length(ident) > 1
            then error(27)
            else
              if (lastch < 'A') or (lastch > 'Z')
              then begin error(27); lastch := 'A'; end;
            fscan;
            end
          else error(27);
          end;
        for c := firstch to lastch do
          with implicit[c] do begin
            if Defd then error(25);
            Defd := TRUE; imptype.typ := itype; imptype.siz := vlength;
            end;
        end
      else error(27);
      if token = COMMASY then fscan else Exit2Flag := TRUE;
    until Exit2Flag;
    if token = RPARENSY then fscan else error(26);
  until token <> COMMASY;
  end; {implstmt}
  
  { namedecl parses "NAME [ ([lo:] hi [,[lo:] hi] ... ) ]" }
  
  function namedecl: pn;
    var lpn,lpn2: pn; dim: integer; lpb,lastpb: pbound; AssumedFlag: Boolean;
    
    procedure getbound(var fbound: brec);
    begin
    if token = STARSY
    then begin
      if (not lpn^.Indirect) and (lpn^.nkind <> UNKN) then error(65);
      fbound.bkind := ASSUMEDBOUND; AssumedFlag := TRUE; fscan;
      end
    else begin
      exprkind := ADJUSTEXPR; expression; exprkind := NORMEXPR;
      fold(gattr.treeptr);
      if gattr.atype.typ = INTEGR
      then begin
        if gattr.treeptr <> nil
        then
          if gattr.treeptr^.node = CSTNODE
          then begin
               fbound.bkind := CONSTBOUND;
               fbound.i := gattr.treeptr^.cstvalu.ival;
               end
          else begin
            fbound.bkind := ADJUSTBOUND;
            makeisize(gattr,4); fbound.adjexpr := gattr.treeptr;
            end;
        end
      else error(170);
      end;
    end; {getbound}
    
  begin {namedecl}
  if token = IDENTSY
  then begin
    lpn := lookupname(localtree,ident);
    if lpn = nil
    then 
      begin newn(lpn,UNKN,ident); entername(lpn,localtree); 
{ added bu jim t at SGI for dbx }
	lpn^.isparam := FALSE;
      end;
    xref(lpn); fscan;
    if token = LPARENSY
    then begin
      if (lpn^.nkind = UNKN) or (lpn^.nkind = VARS)
      then begin
        if lpn^.nkind = UNKN
        then begin
          if lpn^.unbounds <> nil then error(30)
          end
        else if lpn^.bounds <> nil then error(30);
        dim := 0; AssumedFlag := FALSE;
        repeat
          if AssumedFlag then error(67);
          dim := dim + 1; fscan; new(lpb);
          with lpb^ do begin
            next := nil; lo.bkind := CONSTBOUND; lo.i := 1;
            hi.bkind := CONSTBOUND; hi.i := 1;
            BigIndex := FALSE; BigElSize := FALSE;
            getbound(hi);
            if token = COLONSY
            then begin
              if hi.bkind = ASSUMEDBOUND then error(67);
              lo := hi; fscan; getbound(hi);
              end;
            if (lo.bkind = CONSTBOUND) and (hi.bkind = CONSTBOUND)
            then
              if lo.i > hi.i then error(171);
            end;
          if dim = 1
          then
            if lpn^.nkind = UNKN
            then lpn^.unbounds := lpb
            else lpn^.bounds := lpb
          else lastpb^.next := lpb;
          lastpb := lpb;
          if dim = 8 then error(31);
        until token <> COMMASY;
        if token = RPARENSY then fscan else error(26);
        end
      else error(36);
      end;
    namedecl := lpn;
    end
  else begin error(28); namedecl := nil; end;
  end; {namedecl}
  
  function findcommon(n: idstring): pn;
    label 1;
    var lpn,lpn2: pn;
  begin
  lpn := commonlist;
  while lpn <> nil do
    with lpn^ do
      if idEQstr(name,n) then goto 1 else lpn := next;
  newn(lpn,COMMON,n); lpn2 := lookupname(globaltree,n);
  if lpn2 <> nil
  then
    begin
    if lpn2^.nkind <> COMMON then error(34);
    end
  else if n <> ' ' then makeglobal(lpn);
  lpn^.next := commonlist; commonlist := lpn;
1:findcommon := lpn;
  end; {findcommon}
  
  procedure commonstmt;
    var lpc,lpn,lpn2: pn; n: idstring; ExitFlag: Boolean;
  begin
  needstate(SPECSTATE,SPECSTATE);
  fscan;
  n := ' ';
  lpc := nil;
  ExitFlag := FALSE;
  repeat
    if token = SLASHSY
    then begin
      fscan;
      if token = IDENTSY
      then begin n := ident; fscan; end
      else n := ' ';
      if length(n) > MAXCOMIDLEN then error(301);
      lpc := findcommon(n); xref(lpc);
      if token = SLASHSY then fscan else error(38);
      end
    else
      if token = CONCATSY
      then begin n := ' '; lpc := findcommon(n); fscan; end;
    if lpc = nil
    then lpc := findcommon(n);
    lpn := namedecl;
    if lpn <> nil
    then begin
      if lpn^.nkind = UNKN then unkntovars(lpn);
      if lpn^.nkind = VARS
      then begin
        if lpn^.Indirect
        then error(63)
        else
          begin
          if lpn^.incommon = nil
          then
            begin
            lpn^.incommon := lpc;
            if lpc^.comellist = nil
            then lpc^.comellist := lpn
            else
              begin
              lpn2 := lpc^.comellist;
              while lpn2^.next <> nil do
                lpn2 := lpn2^.next;
              lpn2^.next := lpn;
              end;
            end
          else error(40);
          end;
        end
      else error(58);
      end; { lpn <> nil }
    if token = COMMASY
    then fscan
    else
      if (token <> SLASHSY) and (token <> CONCATSY)
      then ExitFlag := TRUE;
  until ExitFlag;
  end; {commonstmt}
  
  procedure dimstmt;
    var lpn: pn;
  begin
  needstate(SPECSTATE,SPECSTATE);
  repeat
    fscan;
    lpn := namedecl;
    if lpn <> nil
    then
      if lpn^.nkind = VARS
      then begin
        if lpn^.bounds = nil then error(29);
        end
      else
        if lpn^.nkind = UNKN
        then begin
          if lpn^.unbounds = nil then error(29);
          end
        else error(58);
  until token <> COMMASY;
  end; {dimstmt}
  
  procedure equivstmt;
    var lpn: pn; lpe: pe; lpequivel: pequivel; dim,i: integer;
        ExitFlag,Exit2Flag: Boolean;
    
    procedure eqsubstr(loindex: integer; fpequivel: pequivel);
      var i: integer;
    begin
    with fpequivel^ do begin
      SubStrFlag := TRUE; lo := 1; hi := 0;
      if loindex >= 1 then lo := loindex else error(211);
      fscan;
      if token <> RPARENSY
      then begin
        cstexpression;
        if gattr.atype.typ = INTEGR
        then begin
          i := gattr.treeptr^.cstvalu.ival;
          if (i >= lo) and (i <= MAXCHAR) then hi := i else error(211);
          end
        else error(212);
        end;
      if token = RPARENSY then fscan else error(26);
      end;
    end; {eqsubstr}
    
  begin {equivstmt}
  needstate(SPECSTATE,SPECSTATE);
  repeat
    fscan;
    if token = LPARENSY then fscan else error(24);
    new(lpe);
    with lpe^ do
      begin next := equivlist; equivgroup := nil; end;
    equivlist := lpe;
    ExitFlag := FALSE;
    repeat
      if token = IDENTSY
      then begin
        lpn := lookupname(localtree,ident);
        if lpn = nil
        then
          begin newn(lpn,VARS,ident); entername(lpn,localtree); end;
        if lpn^.nkind = UNKN then unkntovars(lpn);
        if lpn^.nkind <> VARS then error(58);
        if lpn^.Indirect then error(64);
        xref(lpn); fscan;
        new(lpequivel);
        with lpequivel^ do begin
          varname := lpn; next := lpe^.equivgroup; SubStrFlag := FALSE;
          end;
        lpe^.equivgroup := lpequivel;
        dim := 0;
        if token = LPARENSY
        then begin
          fscan;
          if token = COLONSY
          then eqsubstr(1,lpequivel)
          else begin
            cstexpression;
            if gattr.atype.typ <> INTEGR then error(212);
            if token = COLONSY
            then eqsubstr(gattr.treeptr^.cstvalu.ival,lpequivel)
            else begin
              Exit2Flag := FALSE;
              repeat
                dim := dim + 1;
                if dim <= 7
                then lpequivel^.inx[dim] := gattr.treeptr^.cstvalu.ival
                else
                  if dim = 8 then error(31);
                if token = COMMASY
                then begin
                  fscan; cstexpression;
                  if gattr.atype.typ <> INTEGR then error(210);
                  end
                else Exit2Flag := TRUE;
              until Exit2Flag;
              if dim > 7 then dim := 7;
              if token = RPARENSY then fscan else error(26);
              if token = LPARENSY
              then begin
                fscan;
                if token = COLONSY
                then eqsubstr(1,lpequivel)
                else begin
                  cstexpression;
                  if gattr.atype.typ <> INTEGR then error(212);
                  if token = COLONSY
                  then eqsubstr(gattr.treeptr^.cstvalu.ival,lpequivel)
                  else error(213);
                  end;
                end;
              end;
            end;
          end;
        lpequivel^.dimens := dim;
        end
      else error(28);
      if token = COMMASY then fscan else ExitFlag := TRUE;
    until ExitFlag;
    if token = RPARENSY then fscan else error(26);
  until token <> COMMASY;
  end; {equivstmt}
  
  procedure extstmt;
    var lpn,lpn2: pn;
  begin
  needstate(SPECSTATE,SPECSTATE);
  repeat
    fscan;
    if token = IDENTSY
    then begin
      lpn := lookupname(localtree,ident);
      lpn2 := lookupname(globaltree,ident);
      if lpn = nil
      then begin
        if lpn2 = nil
        then newn(lpn,EXTERN,ident)
        else begin
          new(lpn); lpn^ := lpn2^; lpn^.llink := nil;
          lpn^.rlink := nil; lpn^.RtnLocal := FALSE;
          end;
        entername(lpn,localtree);
        end
      else { Already appears in localtree }
        if lpn^.nkind = UNKN
        then begin
          if (lpn2 = nil) or lpn^.Indirect
          then { Doesn't appear in GLOBALTREE or is a formal parameter }
            begin
            with lpn^ do begin
              nkind := EXTERN; rtnno := -1; arglist := nil;
              rtnlev := 2; NDefined := FALSE; RtnLocal := FALSE;
              end;
            end
          else { Appears in GLOBALTREE and is not a formal parameter }
            if (lpn2^.nkind = FUNCT) or (lpn2^.nkind = SUBR) or
               (lpn2^.nkind = BLOCKD)
            then
              with lpn^ do begin
                nkind := lpn2^.nkind; rtnno := lpn2^.rtnno;
                rtnlev := lpn2^.rtnlev; NDefined := lpn2^.NDefined;
                next := lpn2^.next; RtnLocal := FALSE;
                arglist := lpn2^.arglist;
                end
            else error(34);
          end
        else error(34);
      xref(lpn); fscan;
      end
    else error(28);
  until token <> COMMASY;
  end; {extstmt}
  
  procedure intrinstmt;
    var lpn,lpn2: pn;
  begin
  needstate(SPECSTATE,SPECSTATE);
  repeat
    fscan;
    if token = IDENTSY
    then begin
      lpn := lookupname(intrintree,ident);
      if lpn <> nil
      then begin
        lpn2 := lookupname(localtree,ident);
        if lpn2 <> nil
        then begin
          if lpn2^.nkind = UNKN
          then begin
            lpn2^.nkind := INTRIN; lpn2^.key := lpn^.key;
            lpn2^.ifunno := lpn^.ifunno;
            if (lpn^.idtype.typ <> lpn2^.idtype.typ) or
               (lpn^.idtype.siz <> lpn2^.idtype.siz) then error(82);
            end
          else error(34);
          end
        else begin { lpn2 = nil }
          newn(lpn2,INTRIN,ident);
          lpn2^.key := lpn^.key;
          lpn2^.idtype := lpn^.idtype;
          lpn2^.ifunno := lpn^.ifunno;
          entername(lpn2,localtree);
          end;
        xref(lpn2);
        end
      else error(81);
      fscan;
      end
    else error(28);
  until token <> COMMASY;
  end; {intrinstmt}
  
  procedure paramstmt;
    var ExitFlag: Boolean; lpn: pn;
  begin
  needstate(IMPSTATE,SPECSTATE);
  fscan;
  if token = LPARENSY then fscan else error(24);
  ExitFlag := FALSE;
  repeat
    if token = IDENTSY
    then begin
      lpn := lookupname(localtree,ident);
      if lpn = nil
      then begin
        newn(lpn,PARAMETER,ident);
        entername(lpn,localtree);
        end
      else
        if lpn^.nkind = UNKN
        then begin
          lpn^.nkind := PARAMETER;
          lpn^.pvalu.ival := 0;
          end
        else error(34);
      xref(lpn); fscan;
      end
    else begin error(28); lpn := nil; end;
    if token = ASSIGNSY then fscan else error(59);
    cstexpression;
    if lpn <> nil
    then
      with lpn^ do
        if nkind = PARAMETER
        then begin
          ExpType := TRUE;
          if Numeric(lpn^.idtype.typ)
          then cmakentype(gattr,lpn^.idtype)
          else
            if lpn^.idtype.typ = CHARACTER
            then 
              if gattr.atype.typ = CHARACTER
              then
                if lpn^.idtype.siz = 0 {=*(*)}
                then lpn^.idtype.siz := gattr.atype.siz
                else cmakeslen(gattr,lpn^.idtype.siz)
              else error(214)
            else {lpn^.idtype.typ = LOGICAL}
              if gattr.atype.typ <> LOGICAL
              then error(215);
          lpn^.pvalu := gattr.treeptr^.cstvalu;
          end;
    if token = COMMASY then fscan else ExitFlag := TRUE;
  until ExitFlag;
  if token = RPARENSY then fscan else error(26);
  end; {paramstmt}
  
  procedure savestmt;
    var lpn: pn; ExitFlag: Boolean;
  begin
  needstate(SPECSTATE,SPECSTATE);
  fscan;
  if token = EOLNSY
  then GSaveFlag := TRUE
  else
    repeat
      if token = SLASHSY
      then begin
        fscan;
        if token = IDENTSY
        then begin
          lpn := findcommon(ident);
          with lpn^ do
            begin
            if SaveFlag then error(39);
            SaveFlag := TRUE;
            end;
          xref(lpn); fscan;
          end
        else error(28);
        if token = SLASHSY then fscan else error(38);
        end
      else
        if token = IDENTSY
        then begin
          lpn := lookupname(localtree,ident);
          if lpn = nil
          then begin newn(lpn,UNKN,ident); entername(lpn,localtree); end;
          if lpn^.nkind = UNKN then unkntovars(lpn);
          if lpn^.nkind = VARS
          then begin
            if lpn^.SaveFlag then error(201);
            lpn^.SaveFlag := TRUE;
            if lpn^.Indirect then error(202);
            if lpn^.incommon <> nil then error(203);
            end
          else error(200);
          xref(lpn); fscan;
          end
        else error(200);
      ExitFlag := token <> COMMASY;
      if not ExitFlag then fscan;
    until ExitFlag;
  end; {savestmt}
  
  procedure typestmt(ftyp: types; defaultsize: integer);
    var lpn: pn; ExitFlag: Boolean; lsize,newsize: integer;
  
    function getint: integer;
      var lint: integer; lch:  char;
    begin
    lint := 0;
    lch := getnonblank;
    if chclass[ord(lch)] <> DIGITCL 
    then error(18);
    while chclass[ord(lch)] = DIGITCL do begin
      if (lint >= 3277) or
        ((lint  = 3276) and (lch > '7'))
      then
        begin error(11); lint := 0; end { Prevent too many error messages }
      else
        lint := lint * 10 + ord(lch) - ord('0');
      lch := getnonblank;
      end;
    stmtbufp := stmtbufp - 1; { BACKUPCH }
    getint := lint;
    end; {getint}
    
  begin {typestmt}
  if Match('*')
  then begin
    if ftyp <> CHARACTER then notansi;
    if Match('(')
    then begin
      if ftyp <> CHARACTER then error(208);
      if Match('*')
      then begin
        defaultsize := 0;
        if not Match(')') then error(26);
        end
      else begin
        fscan; cstexpression;
        if gattr.atype.typ = INTEGR
        then newsize := gattr.treeptr^.cstvalu.ival
        else error(209);
        if (newsize > 0) and (newsize <= MAXCHAR)
        then defaultsize := newsize
        else error(98);
        if token <> RPARENSY then error(26);
        end;
      end
    else verifysize(ftyp,defaultsize,getint,defaultsize);
    if Match(',') then;
    end;
  if Match('FUNCTION')
  then procstmt(ftyp,defaultsize,TRUE,FUNCT)
  else
    begin
    needstate(SPECSTATE,SPECSTATE);
    fscan;
    ExitFlag := FALSE;
    repeat
      lpn := namedecl;
      lsize := defaultsize;
      if token = STARSY
      then
        begin
        fscan;
        if token = LPARENSY
        then begin
          fscan;
          if token = STARSY
          then begin lsize := 0; fscan; end
          else begin
            cstexpression;
            if gattr.atype.typ = INTEGR
            then begin
              newsize := gattr.treeptr^.cstvalu.ival;
              if (newsize > 0) and (newsize <= MAXCHAR)
              then lsize := newsize
              else error(98);
              end
            else error(209);
            end;
          if ftyp <> CHARACTER then error(208);
          if token = RPARENSY then fscan else error(26);
          end
        else
          if token = ICONSTSY
          then begin verifysize(ftyp,defaultsize,intval,lsize); fscan; end
          else error(210);
        end;
      if lpn <> nil
      then
        with lpn^ do
          begin
          if ExpType
          then error(33)
          else
            begin
            if (lsize = 0) and (lpn <> localtree) and
               (not Indirect) and (nkind = VARS)
            then begin makidstr(errname,name); error(216); end;
            if nkind = INTRIN
            then
              if idtype.typ <> ftyp then error(82);
            ExpType := TRUE; idtype.typ := ftyp; idtype.siz := lsize;
            end;
          end;
      if token = COMMASY then fscan else ExitFlag := TRUE;
    until ExitFlag;
  end;
  end; {typestmt}
  
  procedure compdeclstmt;
    var lch: char; LLabeledFlag,IsAssFlag: Boolean; olderrors: integer;
  begin
  LLabeledFlag := LabeledFlag; ErrorEnable := FALSE;
  oldstmtbufp := stmtbufp; IsAssFlag := IsAssign; stmtbufp := oldstmtbufp;
  ErrorEnable := TRUE; olderrors := errors;
  if IsAssFlag
  then ExitFlag := TRUE
  else
    if Match('FORMAT')
    then
      begin
      needstate(IMPSTATE,SPECSTATE);
      formatstmt;
      end
    else begin
      lch := getnonblank;
      if lch in ['B'..'F','I','L','P','R','S']
      then
        case lch of
          'B': if Match('LOCKDATA') then blockstmt
               else ExitFlag := TRUE;
          'C': if Match('OMMON') then commonstmt
               else 
                 if Match('HARACTER') then typestmt(CHARACTER,1) 
                 else
                   if Match('OMPLEX') then typestmt(COMPLEX,8)
                   else ExitFlag := TRUE;
          'D': if Match('IMENSION') then dimstmt
               else
                 if Match('OUBLEPRECISION') then typestmt(REEL,8)
                 else ExitFlag := TRUE;
          'E': if Match('QUIVALENCE') then equivstmt
               else
                 if Match('XTERNAL') then extstmt
                 else
                   if Match('NTRY') then entrystmt
                   else ExitFlag := TRUE;
          'F': if Match('UNCTION') then procstmt(INTEGR,0,FALSE,FUNCT)
               else ExitFlag := TRUE;
          'I': if Match('MPLICIT') then implstmt
               else
                 if Match('NTRINSIC') then intrinstmt
                 else 
                   if Match('NTEGER') then typestmt(INTEGR,defisize)
                   else ExitFlag := TRUE;
          'L': if Match('OGICAL') then typestmt(LOGICAL,deflsize)
               else ExitFlag := TRUE;
          'P': if Match('ROGRAM') then PROGSTMT 
               else
                 if Match('ARAMETER') then paramstmt
                 else ExitFlag := TRUE;
          'R': if Match('EAL') then typestmt(REEL,4)
               else ExitFlag := TRUE;
          'S': if Match('UBROUTINE') then procstmt(INTEGR,0,FALSE,SUBR)
               else
                 if Match('AVE') then savestmt 
                 else ExitFlag := TRUE;
        end {case}
      else
        ExitFlag := TRUE;
      end;
  if not ExitFlag and (token <> EOLNSY) and (olderrors = errors)
  then error(23);
  end; {compdeclstmt}
  
begin {declstmts}
ExitFlag := FALSE;
InsideLogIf := FALSE;
stmtbufp := oldstmtbufp;
FirstTime := TRUE;
firststmt := nil;
while StmtbufValid and not ExitFlag do begin
  if FirstTime
  then FirstTime := FALSE
  else getstmt;
  newstmt := nil;
  if StmtbufValid then compdeclstmt else error(4);
  if newstmt <> nil
  then begin
    if firststmt = nil
    then firststmt := newstmt
    else laststmt^.nextstmt := newstmt;
    laststmt := newstmt;
    end;
  end;
end; {declstmts}

