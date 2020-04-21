.na
.nr PS 12
.nr VS 14
.DA "22 November 1985"
.ds PS P\s-2OST\s+2S\s-2CRIPT\s+2
.ds TS T\s-2RAN\s+2S\s-2CRIPT\s+2
.de IR
\fI\\$1\fP\\$2
..
.TL
\*(TS 2.0
.br
Overview
.FS
\*(PS and \*(TS are trademarks of Adobe Systems Incorporated.
.br
UNIX is a trademark of AT&T Bell Laboratories.
.br
Apple and LaserWriter are trademarks of Apple Computer.
.br
QMS and LaserGrafix are registered trademarks of QMS Incorporated.
.br
Linotype and Linotronic are trademarks of Allied Corporation.
.br
Dataproducts is a registered trademark of Dataproducts Corporation.
.br
VAX, MicroVAX, and Ultrix are trademarks of Digital Equipment Corporation.
.br
Sun Workstation is a trademark of Sun Microsystems Incorporated.
.br
Times and Helvetica are registered trademarks of Allied Corporation.
.br
Scribe and U\s-1NILOGIC\s+1 are registered trademarks of U\s-1NILOGIC\s+1, Ltd.
.br
TeX is a trademark of the American Mathematical Society.
.br
Documenter's Workbench is a trademark of AT&T Technologies.
.br
Diablo Systems is a Xerox company.
.br
Tektronix is a registered trademark of Tektronix Corporation.
.br
Macintosh is a trademark of McIntosh Laboratories, Inc. licensed to
Apple Computer.
.br
IBM is a trademark of International Business Machines.
.FE
.AU
Adobe Systems Incorporated
.AI
1870 Embarcadero Road
Palo Alto, CA  94303
.NH
Introduction
.PP
The \*(TS package is a suite of UNIX programs that enable 
UNIX systems to access state-of-the-art \*(PS printers.
The \*(TS package transforms UNIX documents and graphics files
into \*(PS format.  
The \*(PS language is a device-independent programming language and print
file format for describing the appearance of printed pages. \*(PS
documents can combine text, graphics, line art, and scanned images
for printing on raster devices.  \*(PS files will print 
.I
without modification
.R
on any \*(PS printer.
.PP
The \*(TS package includes 
translator filters for
common UNIX document file formats like
.IR troff ,
.IR ditroff ,
and 
.IR plot .
It also includes filters for \*Qline-printer\*U listings,
Diablo 630 print files, and Tektronix 4014 files.
\*(TS software is fully integrated into both the UNIX 4.2BSD 
and System V line printer
spooling architectures, and includes the communications filters
necessary to print job banner pages, do page accounting,
and perform full RS232 communication with a \*(PS printer.
The \*(TS package also includes
complete documentation and installation instructions,
font metric information, and sample \*(PS print files.
.PP
The first few sections of this document give a general overview of the \*(TS
system.  The \*(TS system modules and architecture are then discussed in
more depth.  Then follows complete system installation 
instructions and information for system managers.
.NH
Licensing and Availability
.PP
\*(TS software is available for both Berkeley 4.2BSD and
AT&T System V Release 2.0 UNIX systems.
It has been successfully compiled and run on a wide variety of
hardware supporting these systems, including
VAX and MicroVAX systems, Sun Workstations, and AT&T 3B2's.
If you are interested in attempting a port yourself,
see the notes on porting in a later section.
.PP
\*(TS software is available in source or binary forms
under license from Adobe Systems Incorporated.
Binaries are available for Berkeley 4.2BSD VAX UNIX and Ultrix systems,
or for AT&T 3B2 and 3B5 computers running UNIX System V
Release 2.0 version 2.
\*(TS sources work with 4.2BSD and System V UNIX systems.
The \*(TS package consists of C programs, makefiles, Bourne shell
scripts, and a few uses of 
.I awk
and
.IR sed .
Binaries for other systems may be available from those vendors.
For licensing, dealer, or OEM information, contact Adobe Systems.
.PP
\*(PS printers are available from a number of manufacturers, 
including Apple Computer, QMS Incorporated, 
Dataproducts Corporation, and Linotype Company.
Hundreds of software packages on dozens of systems support
the \*(PS printing standard.
.NH
\*(TS Package Contents
.PP
The \*(TS distribution consists of the following hard-copy sections:
.DS
Overview (this document)
\*(PS Book Ordering Information
Installation Instructions
Release Notes
\*(TS Support Information & Customer Comment Form
Sample Output
Inserts for UNIX User's Manual
Magnetic media contents listing
.DE
.PP
The following directories and files are in the distribution:
.IP Notice
Copyright notice.
.IP Makefile
Master makefile that calls Makefile's in subdirectories.
.IP "bsd, sysv, config & printer"
Shell scripts to configure \*(TS installation for your host.
.IP src/
Sources or configurable binaries.
.IP sh/
Shell scripts.
.IP lib/
Font information, \*(PS prologue templates.
.IP etc/
UNIX System file information.
.IP man/
UNIX manual pages.
.IP doc/
Additional documentation, including this document.
.IP test/
Some sample \*(PS programs.
.IP cookbook/
Example sources from the \*(PS cookbook.
.IP goodies/
Usefull things of other sorts.
.NH
Translation Filters
.NH 2
Text Files \(bu enscript
.PP
One of the most useful parts of the \*(TS package is a program called
.I enscript.  
It makes listings of text files, with a multitude of options
to control fonts, page headings, rotation, 2-up printing, etc.
The default action of 
.I enscript
is to spool its output for printing.  
At many installations, 
.I enscript
will be the most frequently used interface to \*(PS printers. 
.NH 2
Ditroff \(bu psdit, psroff
.PP
The \*(TS package also includes a translator for 
.I ditroff
(Device Independent Troff).    
.I  ditroff 
is sold and licensed independently by AT&T either alone, or
in a package known as the
.B
Documenter's Workbench
.R
(DWB).
In the DWB utilities, 
.I ditroff
is known simply as
.IR troff.
Adobe does not currently sublicense 
.I ditroff
itself, but the translator is a part of the \*(TS distribution.
.PP
.I ditroff
allows more freedom and power than does original
.I troff,
as some of the restrictions which were imposed by the C/A/T
phototypesetter are no longer present.  
Many more fonts may be referenced at one time, and the notion
of what character set is provided is much for flexible.
.I ditroff
also provides some primitive graphics facilities, so that preprocessors like
.I pic
and
.I ideal
can work with it (and not with the original troff).  
.PP
.I psdit
is the \fIditroff\fP intermediate file to \*(PS converter.  It takes 
\fIditroff\fP output files and converts them into \*(PS programs.  
Font metric information and the \fIditroff\fP device description file
for the \*(PS ``virtual device'' are built automatically by shell and
\fIawk\fP scripts included and documented in this distribution.
The \*(TS package comes with description entries for 13 \*(PS font faces.
.PP
.I psroff
is a shell script which runs \fIditroff\fP, \fIpsdit\fP, and 
the line printer spooler in an environment to produce good output.
.PP
Some care must be taken when using \fIeqn\fP, \fIpic\fP, or \fIideal\fP,
in conjunction with these \*(TS filters.  The versions of these processors
distributed with \fIditroff\fP have various device types compiled-in.
(So much for device-independence!)
The \*(PS virtual device has a ``resolution'' of 576 units per inch
and a minimum point size of 2.
\fIeqn\fP, \fIpic\fP, and \fIideal\fP should be modified to understand the
.B \-Tpsc
flag and set their parameters accordingly.  (Actually, these programs should
look at the device description files, so that no such 
constants need be compiled in!)  See the \fIpsdit\fP 
and \fIpsroff\fP \fBman\fP pages for more information.
.PP
Full installation of the \fIditroff\fP utilities requires that you
have \fIditroff\fP (or DWB) at your site.  In particular, creation
of the font widths tables requires a program known as 
.I makedev
(called
.I devconfig
in the UCB-Gremlin package).  The \fImakedev\fP is \fBmissing\fP
from the 3B2 DWB package.  This is an error on the part of 
AT&T.  Contact your AT&T service representative for more information.
.PP
.I psdit
also allows the inclusion of arbitrary \*(PS code into \fIditroff\fP-generated
documents.  See the \fBman\fP page for \fIpsdit\fP for more information.
.NH 2
Troff \(bu pscat, pscatmap, and ptroff
.PP
Support for original
.I troff
comes in the form of three programs and associated font information.  
(In DWB, original 
.I troff
is known as 
.IR otroff .)  
The \*(TS package comes with 
font tables for both the Times and Helvetica families
(which are resident in ROM on all \*(PS printers).  
In addition, the user
has the ability to build new font mappings.  For example, 
.I troff
and 
.I pscat
could be told to use Times-Roman, Times-Italic, and Helvetica-Bold 
in positions 1, 2, and 3.  The user also has control over
character mappings, and can bind an arbitrary \*(PS procedure to a 
.I troff
character code.  This enables incorporation of
things like logotypes, or scanned images into
.I troff 
documents.
.PP
.I pscat
is the C/A/T to \*(PS converter.  It takes 
.I troff
output format files (which consist of control codes for a C/A/T
phototypesetter) and converts them into \*(PS programs.
.I pscatmap
builds font correspondence tables and troff width tables to be used by 
.I troff
and
.IR pscat .
The user may build a correspondence table which associates an
action with each character in the 
.I troff
character set.  The most common action is to print a character
in a \*(PS font.  Other possible actions are to \*Qfake\*U
ligatures not present in a \*(PS font, and to invoke
an arbitrary \*(PS procedure of the user's creation.
The \*(TS package comes with correspondence tables already built 
for the Times and Helvetica families.  When 
.I troff
is run, the proper font width tables must be loaded (and the proper font 
faces mounted) for the correct production of \*(PS documents.  
.I  pscat
must then be run with a correspondence table for the font set that 
.I troff
ran with.  The user must forego the ability to use arbitrary 
.B .fp 
commands in a 
.I troff 
document; however, with a proper correspondence table, this 
should not be a problem.
.PP
.I ptroff
is a shell script which runs 
.IR troff ,
.IR pscat , 
and the spooler in an environment to produce good output \- managing 
.B .fp
commands and correspondence tables automatically.
.NH 2
Plot Format \(bu psplot
.PP
.I psplot,
a UNIX 
.I plot
format filter is also provided.  
.I  plot
format is documented in plot(5), and produced by plot(3),
plot(1), and graph(1).
The translation of plot format to \*(PS programs is fairly
straightforward.  Plot commands are translated almost
one-for-one into \*(PS procedure calls.
The binding decisions
of what actually takes place are all present in the
\*(PS prologue of the translated file.  Thus, if the user is 
not content with the dashing pattern of lines, the size of
text, or the scale of the final image, for example, a simple edit
to the prologue can change the look of the printed page without
the need to change the translation filter itself.
.PP
AT&T has removed \fIplot\fP format support from UNIX System V, 
but \fIpsplot\fP is supplied with all \*(TS systems.
.NH 2
Diablo 630 \(bu ps630
.PP
.I ps630
is a translator for Diablo 630 print files.  The Diablo is 
a popular daisy-wheel printer.  This filter can be useful as a 
post-processor for applications which only know about
fixed-pitch printing devices.  For example,
it can be used for printing files generated with the 
.B \-Txerox
option to 
.IR nroff .  
In addition, programs like Scribe have the ability to
generate Diablo print files.
Note that some \*(PS printers also have a built-in Diablo
.I emulation 
mode.  The translator allows a little more control in the
printing process as well as the \*Qcapture\*U of 
the \*(PS version of the document.
.NH 2
Tektronix 4014 \(bu ps4014
.PP
.I ps4014
converts Tektronix 4014 display files into \*(PS programs for
printing.  Tektronix 4014 format is a popular vector-oriented
(calligraphic) display representation, and many programs have
options to drive Tektronix 4014 display devices.
.I ps4014
allows these programs to produce hardcopy output on \*(PS printers.
.NH 2
Page Selection \(bu psrev
.PP
.I psrev
is a filter which will select and reverse subsets of pages from
a \*(PS print file which conforms to certain file structuring
conventions.  It is useful for printing small portions of larger
documents.  Page reversal may also be performed automatically 
on a printer-by-printer basis.
.NH
\*(PS Print Files and Magic Numbers
.PP
\*(TS
determines if a spooled file is a \*(PS print file by examining the first
few bytes of the input for the \*(PS \*Qmagic number\*U.
If the file begins with \*Q%!\*U \- a percent sign and an exclamation mark \-
\*(TS assumes that the file is a \*(PS program to be 
.I executed
by the printer.  Moreover, if the file begins with \*Q%!PS\-Adobe\-\*U, 
\*(TS assumes that the files may be page-reversed with the filter
.I psrv
for printing in the proper collating sequence.
If the file does not begin with the magic number, 
\*(TS assumes that the file is a text file to be 
.I listed
on the printer, and formats it with the filter
.I pstext.
This convention must be used since sending a file which is not
a \*(PS program to a \*(PS printer will almost certainly 
produce the undesired results.
The \*(PS magic number and other comment conventions are explained 
more fully in the 
.I
\*(PS Language Reference Manual
.R
from Addison-Wesley.
.NH
UNIX System V Spooler Interface
.PP
\*(TS works with the UNIX System V Release 2.0 
.I
Line Printer Spooling Utilities.  
.R
The Line Printer Spooling Utilities are necessary for
spooling and printing \*(PS files from a System V system.
System Administrators should familiarize themselves with
.IR lp (1),
.IR lpstat (1),
.IR lpadmin (1M),
.IR lpsched (1M),
and related software.
.NH 2
psinterface
.PP
The System V 
.I lp/lpsched
spooler allows printer interface programs
to be specified for particular printers or printer-classes.
.I psinterface
is the top-level spooler interface invoked by
.I lpsched
to print on a \*(PS printer.  (It is the 
.I interface
program presented to 
.I lpadmin
when installing a \*(PS printer.)  
.I psinterface 
is a shell script that is responsible for parsing spooler arguments,
validating a print request, printing banner break pages,
performing job logging, and, or course, printing.  A particular instance of
.I psinterface
is created and installed with the 
.I mkprinter
shell script described in the Installation Instructions below.
.PP
.I psinterface
invokes several other \*(TS programs to accomplish its task:
.I psbanner
is used to create banner break pages,
.I pstext 
is used to format files which do not begin with the \*(PS magic number,
.I psrv
is used to page-reverse files when appropriate, and
.I pscomm
performs actual communication with a \*(PS printer.
.I psinterface
performs job logging in a printer-specific log file.  This file also 
contains any stream or error output generated by the print jobs.  
.I psinterface
can be configured to vary its functions: banner pages
may be printed before or after a print job (or both or neither).
Page reversal may be enabled or disabled (some \*(PS printers
do not require it, as they stack pages in the correct order).
.I psinterface 
recognizes 3 \*(TS-specific options (using the 
.B \-o 
switch to
.IR lp ):
.B \-h 
suppresses the printing of banner pages,
.B \-r 
forces page-reversal 
.IR off ,
and 
.B \-m 
causes the stream output of the job to be sent to the user with
.IR mail (1).
.PP
It is also possible to independently configure \*(TS's behavior for 
each printer.  The file /usr/spool/lp/transcript/\fIprinter\fP.opt
can set environment variables which influence the operation of 
.IR psinterface .
The environment variables BANNERFIRST and BANNERLAST (set to 1 or 0)
effect the printing of job banner break pages.  BANNERPRO
is the name of a \*(PS header file to help print the banner page
(and may vary from printer to printer).  REVERSE indicates that
page reversal should be performed.
.NH 2
Job Logging and Printer Status
.PP
.I psinterface
performs job logging and printer status reporting 
for each \*(PS printer on a system.  The file
/usr/spool/lp/transcript/\fIprinter\fP-log contains
job-by-job information about \fIprinter\fP's activity.
The log file contains error messages from the various
spooling utilities, and is one of the first places to 
look if there is a problem with a printer.
If also contains a record of all stream output 
.I from
the printer.  Thus, if a user wishes to have the printer
communicate information back to the host, output to the
\*(PS stream \*Q%stdout\*U (or from the \*(PS \*Qprint\*U operator)
will appear in this file. Printer error conditions are logged here.  
Problems like out-of-paper or paper-jam can be detected by examining
the "tail" of the log file.  
Since the log file can get rather large, it is a good idea to rotate
or truncate the log file on a regular basis.  (At Adobe, we do this
on a daily basis, see etc/daily.sysv for a
.I crontab
template to do this.)
.NH
4.2BSD Spooler Interface
.PP
\*(TS works with the 4.2BSD UNIX Line Printer Spooler.
See the 4.2 document titled
.I 
4.2BSD Line Printer Spooler Manual 
.R
by Ralph Campbell, and the UNIX manual pages for
.IR lpr (1),
.IR lpq (1),
.IR printcap (5),
.IR lpc (8),
and
.IR lpd (8)
for more information.
System Administrators should familiarize themselves with these
documents.
.NH 2
psint.sh
.PP
.I psint.sh
is the top-level \fIlpd\fP-invoked interface for \*(PS printers.
.I psint.sh 
is a shell script that gets invoked under different names to 
perform different functions.  File links allow 
.I psint.sh
to be invoked as any of the allowable printcap filter types.
These links, and associated printcap entries identify 
.I psint.sh
as one of 
.IR ps{i,o,c,d,g,n,r,t,v}f ,
representing the if, of, cf, df, gf, nf, rf, tf, and vf printcap
filters respectively.  Note that not all of these translators are
present in \*(TS.  
.I psint.sh
will issue an error message for an unavailable translator type.
.PP
.I psint.sh
will also run a printer-specific shell script (named \fI\*Q.options\*U\fP
in the printer spooling directory) which may perform additional processing
or set environment variables (like BANNERFIRST and REVERSE) to 
change \*(TS behavior on a printer by printer basis.
.NH 2
psif, pscomm, pstext, psrv
.PP
As the \*Qif\*U filter \fIpsint.sh\fP
does basic printing.  The program \fIpscomm\fP is 
the lowest-level filter, responsible for actual
communication with the printer, error handling, status reporting, etc.
It uses the \*Qmagic number\*U rules described above to distinguish
between text and \*(PS files.  It will exec \fIpstext\fP and \fIpsrv\fP
to format and page-reverse files as necessary, based on printer options
and the file's \*Qmagic number\*U.
.NH 2
psof, psbanner, banner.pro
.PP
As the \*Qof\*U filter,
.I psint.sh
execs
.IR psbanner ,
which is responsible for formatting job banner break pages.
It creates a file containing the banner page, and 
.I pscomm
actually prints it.
.I psbanner
only knows how to deal with the \*Qshort\*U banner format, so the
.B sb 
printcap entry 
.I must 
be specified.
The environment variables BANNERFIRST and BANNERLAST, plus the 
.B sh
printcap entry, determine exactly how the banner page gets printed.  If
.B sh
is specified in the printcap entry, no banner page will get
printed for any job, and banner strings will not appear in the log file.  If 
.B sh
is not specified, then the values of BANNERFIRST and
BANNERLAST are taken into account.  These may be set
independently, and determine whether the banner page should be
printed before and/or after the job.  In either case, 
if VERBOSELOG is set, the
banner string will appear in the log file before the job is
sent.  Any user can omit the printing of a banner page by
specifying the \-h option to 
.I lpr
or
.IR enscript .
Since banner pages take time, paper, and toner to print,
you should decide whether they are important for your site.
(At Adobe, we run without 
.B sh
in the printcap file, and with VERBOSELOG on and BANNERFIRST
and BANNERLAST off.  This allows lots of information to
appear in the printer log file, without wasting paper for
banner pages.)  Note that since the banner page is actually printed by 
.IR pscomm ,
these pages will be charged in the accounting file.
.PP
The format of the banner page is specified by BANNERPRO in
.IR psint.sh .  
This takes as input the short banner string presented by the spooler to 
the \*Qof\*U filter and formats a page displaying the information.  
If you want a different banner page design, you can achieve it by
changing BANNERPRO.
.NH 2
psgf, psnf, pstf, psvf, psrf, psdf, pscf, psbad
.PP
As the other translation filters, 
.I psint.sh
sets up a pipeline between a translator program and 
.I pscomm.
This allows the file format flags to 
.I lpr
to work correctly, but in all cases, much more functionality
is provided by using the translators explicitly and 
spooling the generated \*(PS files.  Note that when using 
.I lpr
to do translation, the \*Qcost\*U of the format translation is 
placed on the printer spooler. The printer may have to wait without printing
while the document translation takes place, thus reducing
job throughput.  It is far more advantageous to 
spool only \*(PS files, and place the burden of translation on user processes.
.PP
The translation filter entries may also specify that the translator
is not available (e.g., a 
.I cifplot
filter) by calling 
.I psbad.  
In this case they log an error message, and print an error page.
.NH 2
Log Files, Printer Status, and Job Accounting
.PP
The \*Qlf\*U entry in the printcap file specifies the printer
log file.  This file contains error messages from the various
spooling utilities.  (\fILpd\fP will complain here if an output 
filter malfunctions.)
It also contains a record of all stream output 
.I from
the printer.  Thus, if a user wishes to have the printer
communicate information back to the host, output to the
\*(PS stream \*Q%stdout\*U (or from the \*(PS \*Qprint\*U
operator) will appear in this file.
Printer error messages (e.g., out of paper) will also appear
here.
If VERBOSELOG is turned on (the default), job banner strings and
start and end markers will appear in the log file, helping to
delimit other output.
.PP
\*(TS will also communicate printer status using the 
\*Qstatus\*U file in the printer spooling directory.
The contents of this file gets printed by a
\fIlpq\fP or \fIlpc status\fP request.  When \*(TS detects
a printer error (e.g., out of paper, paper jam), a message
to that effect will appear in the status file.
.PP
If a proper \*Qaf\*U entry is present in the printcap file, 
.I pscomm
will perform job page accounting.  We suggest running with
accounting on, if only to give you a better idea of printer
usage, and to help keep track of total pages printed.  
.PP
Since the log file and accounting file can get rather large,
it is a good idea to rotate the log file and summarize the
accounting data regularly.  (At Adobe, we do it on a daily
basis, see etc/daily.bsd for a template to do this.)
.NH 
Downloadable Utilities
.NH 2
Error break page printing
.PP
Included in the release is the file 
.I lib/ehandler.ps.  
This is a \*(PS program designed to be downloaded outside the printer's
server loop which will modify the printer's behavior for
\*(PS errors.  The default action is to send an error
message over the output channel (which will appear in the log file),
and abort the current job.  
.I  ehandler.ps
causes the printer to print the current page
with some space taken over to describe the error and dump the
operand stack.  It also sends the error message to the log file.  Since
.I ehandler.ps
contains code that gets loaded outside the server loop, it
contains the default server password.  If you change the
password at your site, you will have to change 
.I ehandler.ps
also.  If the password is sensitive, you will want to 
ensure correct protection of 
.I ehandler.ps
and the spooling directory.  At Adobe, 
.I ehandler.ps
is spooled by root to our printers once an hour by a 
.IR cron -invoked
process (see the hourly templates under etc/ and see 
.IR cron (8)).
.NH 2
LaserWriter patch
.PP
Also included is the file \fIlib/uartpatch.ps\fP which 
may be downloaded to an Apple LaserWriter (it is simply
ignored by all other \*(PS printers).  This patch
fixes three known bugs in the LaserWriter serial I/O
driver.  These bugs include the first two noted under
\*QInput/output problems\*U on page 308 of the
\fI\*(PS Language Reference Manual\fP and a bug
related to voluminous serial output created by
the LaserWriter.
Downloading the patch will guarantee
better performance and fewer errors in the operation
of \*(TS.  Note that \fIuartpatch.ps\fP also
contains the LaserWriter server-loop password.
.NH
Porting to Other Systems
.PP
Most \*(TS modules are quite portable.  The translation filters
make fairly light use of UNIX system calls and attempt to 
use only common C library subroutines.  The translation filters
have no knowledge of the spooling or communications system
they are generating files for.
The actual spooler communications interface programs are, of necessity,
more system dependent.  They make intimate use of terminal driver
facilities, IOCTL's and several other low-level UNIX system calls.
They should serve as a good model as to how to implement
communications with the printer.  Anticipate about two man-weeks
worth of effort to port \*(TS to another variety of UNIX system.
.PP
\*(TS does not provide it's own spooler.  Older UNIX Systems
without a general spooling architecture will face the problem
of modifying an existing spooler (or writing a new one) for
handling spooling requests and managing a queue.
.PP
Please let us know if you succeed in porting \*(TS to another
system.  In particular, if you are willing to share your
experience with other \*(TS customers, we'd like to know.
Also let us know if you have trouble porting \*(TS that is
related to the \*(TS code itself.  We are always looking
to increase the portability of \*(TS with each release.
.NH
\*(PS Software and Other Resources
.PP
The \*(PS language is the subject of two books by Adobe Systems
and published by Addison-Wesley.  These books are available in
many bookstores or from Adobe Systems.  Ordering information is
on a separate sheet in the \*(TS package.
.PP
\*(PS and \*(TS are often topics on several USENET newsgroups.
The groups net.text, net.periphs, net.micro.mac, and net.sources.mac
may be of interest.  The ARPA \*QLASER-LOVERS\*U mailing list
(Usenet mod.computers.laser-printers) is a wealth of information
about digital printing.  Contact:
.DS
LASER\-LOVERS\-REQUEST@Washington.ARPA \fIor\fP
uw\-beaver!laser\-lovers\-request 
.DE
for more information.
.br
An ARPA \*QINFO-POSTSCRIPT\*U list was recently created.  
Contact
.DS
INFO\-POSTSCRIPT\-REQUEST@SU\-SCORE.ARPA \fIor\fP
glacier!info\-postscript\-request
.DE
for more information.
.LP
Several other UNIX software packages support \*(PS.  Among them:
.DS
the Scribe Document Production System from Unilogic, Ltd. of Pittsburgh, PA.
a TeX DVI to \*(PS converter from Textset, Inc. of Ann Arbor, MI.
a public domain DVI to \*(PS converter (contact the UNIX-TeX people)
.DE
Hundreds of Apple Macintosh applications and several IBM-PC applications
can generate \*(PS print files.
.PP
Adobe Systems publishes a quarterly newsletter \- \fIColophon\fP \- 
about \*(PS printers and software.  As a \*(TS customer, you will
automaticly be on our mailing list.  We also attempt to maintain a 
comprehensive list of software supporting \*(PS.  For more information,
contact Adobe Systems.
