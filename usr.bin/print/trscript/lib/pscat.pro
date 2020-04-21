% lib/pscat.pro -- prolog for pscat (troff) files
% Copyright (C) 1985 Adobe Systems, Inc.
save /pscatsave exch def
/$pscat 50 dict def
$pscat begin
/fm [1 0 0 1 0 0] def
/xo 0 def /yo 0 def
/M /moveto load def
/R /show load def
/S {exch currentpoint exch pop moveto show}def
/T {exch currentpoint pop exch moveto show}def
/U {3 1 roll moveto show}def
/siz 0 def
/font 0 def
/Z {/siz exch def SF}def
/F {/font exch def SF}def
/SF{font 0 ne
    {catfonts font 1 sub get fm 0 siz put fm 3 siz neg put 
     fm makefont setfont}if}def
/BP{save/catsv exch def 0 792 translate 72 432 div dup neg scale 
  xo yo translate 0 0 moveto}def
/EP{catsv restore showpage}def

% definitions for PPROC callback functions
% each PPROC is called with the following number on the stack:
% pointsize charcode railmag pswidth pschar x y wid
/$pprocs 50 dict def
/fractm [.65 0 0 .6 0 0] def
% fractions
/PS1{gsave $pprocs begin
    /wid exch def pop pop pop pop pop /ch exch def /size exch def
    /pair $pprocs ch get def /cf currentfont def
    cf fractm makefont setfont
    0 .3 size mul 6 mul 2 copy neg rmoveto pair 0 get show rmoveto
    currentfont cf setfont (\244) show setfont
    pair 1 get show grestore wid .06 div 0 rmoveto end}def
$pprocs begin
8#34 [(1)(4)] def
8#36 [(1)(2)] def
8#46 [(3)(4)] def
end
% boxes
/PS2{gsave /wid exch def pop pop /char exch def pop pop pop /size exch def
    /len size 3.5 mul def % length of a side
    len 0 rlineto 0 len neg rlineto len neg 0 rlineto closepath
    char 3 eq {fill}{size 5 mul .07 mul setlinewidth stroke}ifelse
    grestore wid .06 div 0 rmoveto}def
/PS3/PS2 load def		% boxes are the same...
% circle
/PS4{gsave /wid exch def pop pop pop pop pop pop /size exch def
    wid .8333 mul size 2.5 mul neg rmoveto currentpoint	% center
    newpath size 1.8 mul 0 360 arc size .2 mul setlinewidth stroke
    grestore wid .06 div 0 rmoveto}def
/bb{$pprocs begin /wid exch def pop pop pop pop pop pop /size exch 6 mul def
    /s2 size 2 div def /s4 size 4 div def gsave 
    currentpoint newpath transform round exch round exch itransform translate
    size 16 div setlinewidth 2 setlinejoin 0 setgray}def
$pprocs begin
/mrr{moveto rlineto rlineto}def
/be{stroke grestore wid .06 div 0 rmoveto end}def
end
% leftfloor
/PS6 {bb s4 0 0 size s4 size -.8 mul mrr be}def
% rightfloor
/PS7 {bb s4 neg 0 0 size s4 size -.8 mul mrr be}def
% leftceil
/PS8 {bb s4 0 0 size neg s4 size .2 mul mrr be}def
% rightceil
/PS9 {bb s4 neg 0 0 size neg s4 size .2 mul mrr be}def
% boldvert
/PS5 {bb 0 0 0 size neg s4 size .2 mul mrr be}def
% box rule
/PS32 {bb /sw size 24 div def sw 2 div size 4.5 div moveto
       0 size neg rlineto sw setlinewidth be}def
% rule (roman, bold and italic)
/PS16 {gsave $pprocs begin
    /wid exch def pop pop pop pop pop pop /size exch 6 mul def
    /sw size 14 div def currentpoint exch sw 2 div sub exch
    newpath transform round exch round exch itransform translate
    0 0 moveto size 2 div 0 rlineto sw setlinewidth be}def
% lefttopcurl    
/PS20 {bb s4 size .2 mul moveto 0 size -.55 mul rlineto currentpoint 
    pop size -.8 mul 2 copy exch s4 add exch s4 arcto pop pop pop pop be}def
% leftbotcurl
/PS21 {bb s4 size -.8 mul moveto 0 size .55 mul rlineto currentpoint 
    pop size .2 mul 2 copy exch s4 add exch s4 arcto pop pop pop pop be}def
% righttopcurl
/PS22 {bb s4 size .2 mul moveto 0 size -.55 mul rlineto currentpoint
     pop size -.8 mul 2 copy exch s4 sub exch s4 arcto pop pop pop pop be}def
% rightbotcurl
/PS23 {bb s4 size -.8 mul moveto 0 size .55 mul rlineto currentpoint
     pop size .2 mul 2 copy exch s4 sub exch s4 arcto pop pop pop pop be}def
% rightmidcurl
/PS25 {bb /s3 size -.3 mul def s4 size -.8 mul moveto s4 s3 s2 s3
    s4 arcto pop pop size add s4 s3 4 2 roll
    s4 arcto pop pop pop pop s4 size .2 mul lineto be}def
% leftmidcurl
/PS24 {bb /s3 size -.3 mul def s4 size -.8 mul moveto s4 s3 0 s3
    s4 arcto pop pop size add s4 s3 4 2 roll s4 arcto pop pop pop pop 
    s4 size .2 mul lineto be}def
