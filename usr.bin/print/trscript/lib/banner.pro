%!
% banner page prolog for System V-based spoolers
% Copyright (c) 1985 Adobe Systems Incorporated
% (user)(USER)(host)(seqid)(title)(printer)(date)Banner
% RCSID: $Header: /d2/3.7/src/usr.bin/print/trscript/lib/RCS/banner.pro,v 1.1 89/03/27 18:19:06 root Exp $
/Banner{/date exch def/printer exch def/title exch def/seqid exch def
/host exch def/User exch def/user exch def
statusdict/printername known
{/pn 31 string statusdict/printername get exec def}{/pn(PostScript)def}ifelse
/l{gsave .5 setgray 12 setlinewidth newpath moveto
 468 0 rlineto stroke grestore}def
/x{/w w 30 sub def}def/z{72 w moveto show x}def/s{show(       )show}def
/w 670 def/Helvetica-Bold findfont 14 scalefont setfont
72 720 l 72 w moveto user s host s User s x title z
/Helvetica findfont 14 scalefont setfont
date z 72 w moveto printer s seqid s pn s
72 540 l/Courier findfont 10 scalefont setfont 72 500 moveto/S{show(  )show}def
4{gsave printer S user S host S title S date S grestore
0 -25 rmoveto}repeat showpage}def
