.na
.DA "22 November 1985"
\" RCSID: $Header: /d2/3.7/src/usr.bin/print/trscript/doc/RCS/rnotes.ms,v 1.1 89/03/27 18:18:12 root Exp $
.ds PS P\s-2OST\s+2S\s-2CRIPT\s+2
.ds TS T\s-2RAN\s+2S\s-2CRIPT\s+2
.de IR
\fI\\$1\fP\\$2
..
.TL
\*(TS 2.0
.br
Release Notes
.FS
\*(PS and \*(TS are trademarks of Adobe Systems Incorporated.
.br
UNIX is a trademark of AT&T Bell Laboratories.
.FE
.AU
Adobe Systems Incorporated
.AI
1870 Embarcadero Road
Palo Alto, CA  94303
.NH
Introduction
.LP
These \fIRelease Notes\fP are included in the product for the following
purposes:
.IP \(bu
to outline new features, and bug fixes in the release
.IP \(bu
to identify any known problems with the release
.IP \(bu
to identify any potential differences for different \*(PS printers
.LP
The particulars of the various \*(TS filters are documented 
in their UNIX \fIman\fP pages.
.LP
The \*(TS Overview details the product
structure and the function of all \*(TS programs.
The \*(TS Installation Instructions gives more information to
systems administrators about the \*(TS software, file locations, etc.
.NH
Release 2.0 \- changes since release 1.0
.NH 2
new features
.IP ps4014 1i
.B ps4014,
the Tektronix 4014 file filter, has been added to the product.
.IP "System V"  1i
Support for UNIX System V and the lp/lpsched spooler has been added.
.NH 2
Enhancements and bug fixes to release 1.0
.IP "pscomm, psint.sh, psinterface"  1i
The top level spooler interface routine is now a shell script
for both 4.2bsd and System V.  This allows more flexibility
in adding functions and processing options at some slight
performance expense.  Printer-specific options have been
added.
.IP enscript 1i
Fixed several bugs related to blank pages, form feeds, etc.
Changed margins/linecount computations, added \-L option.
Added Gaudy mode by Guy Riddle.  Changed to \fIgetopt\fP
option processing.
Added output spooling messages.
Added PSLIBDIR and PSTEMPDIR environment variables and got
rid of METRICS.
Fixes for machines with short ints.
.IP ps630 1i
Fixed a bug related to font size selection.  
Code and \fBman\fP page are now in agreement.
Changed to \fIgetopt\fP option processing.
Fixes for machines with short ints.
.IP pscat 1i
Changed to \fIgetopt\fP option processing.
Fixes for machines with short ints.
Changed vertical position initialization and added init/EOP case
for Scribe-generated C/A/T files.
.IP pscatmap 1i
Support for System V ft\fIXX\fP files, and short filename mapping.
Better rounding and fixes for machines with short ints.
Use the PSLIBDIR environment variable if present.
.IP psdit 1i
Fixed several bugs related to font file loading and .fp processing.
Changed FlushShow processing.
Support for included raw \*(PS code.
Use the PSLIBDIR environment variable if present.
Considerable rework of font-table building.  Support for many
more \*(PS symbols.  Implemented a new Special font for \fItroff\fP
special characters.
.IP psplot 1i
Changed to \fIgetopt\fP option processing.
Fix to quoted character bug.
Uses PSLIBDIR if present.
.IP psrev 1i
Changed to \fIgetopt\fP option processing.
Fixed a bug related to the last line of the Trailer.
.IP pstext 1i
Change to argv list \- now used by both 4.2 and System V.
Fixed bugs related to blank pages and page reversal.
.IP "psroff, ptroff" 1i
Versions for System V and 4.2BSD.
Removed page reversal from this stage, now done by the spooler.
.IP psbanner 1i
Versions for System V and 4.2BSD.
.IP pscomm 1i
System V version is new.
Lots of changes to 4.2BSD version to remove race conditions that
were occasionally getting the printer queue wedged.  No longer
wedges if the printer is powered off when a job gets spooled.
.NH
Known Deficiencies in Release 2.0
.LP
The following deficiences are known to exist in Release 2.0:
.IP \(bu
.I psdit
does not yet support the UCB Gremlin extensions to 
the \fIditroff\fP intermediate file format.  This will
be available soon.
.IP \(bu
On 4.2BSD, the 
BANNERLAST feature behaves incorrectly when multiple files
or multiple copies are spooled with one \fIlpr\fP request.
This is due to the order in which \fIpsof\fP and \fIpsif\fP
(the ``of'' and ``if'' printcap filters) are invoked by the
spooling daemon.  There is nothing that can be done to fix this
problem \- the BANNERLAST option may be removed from future
releases of \*(TS software.  
If you set the BANNERLAST option on at your site, warn users that
banner pages may appear in incorrect places.  This problem does not
otherwise effect the operation of \*(TS or the correct printing of documents.
.IP \(bu
As distributed, \*(TS filters may fail to operate correctly with certain
non-standard versions of the 4.2BSD line printer spooler.  In particular,
spoolers that have been modified to pass extra arguments to the printcap
filters will result in error messages being logged in the printer log file.
One such modified spooler passes the flags
.DS
\-f filtertype \-p printername
.DE
to the output filters.  
These options may be parsed (and ignored) by 
making appropriate changes to the \fIpsint.sh\fP
script which \fImkprinter\fP installs in the \*(TS
library directory.
