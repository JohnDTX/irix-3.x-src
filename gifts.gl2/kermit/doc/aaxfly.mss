@Make(text)
@Style<Justification On, Hyphenation On, WidestBlank 1.4, Spacing 1,
        Spread 0.5, Indent 0, HyphenBreak Off, SingleSided, WidowAction Force>
@Modify<Format,Above 1,Below 1>
@Define<QQ,FaceCode R,AfterEntry=["],BeforeExit=["]>
@Case<Device,
 PagedFile="@Style(topmargin 2,bottommargin 0)",
 Diablo="@TypeWheel(Titan 10)",
 Imagen300="@Style(Spacing 1,SingleSided)
	    @Modify(Majorheading,Font TitleFont4,FaceCode R)
	    @Define(QQ,AfterEntry=[@r<``>],BeforeExit=[@r<''>])",
 Imprint10="@Style(FontFamily SmallRoman10,Spacing 1,SingleSided)
    @Define(QQ,AfterEntry=[@r<``>],BeforeExit=[@r<''>])",
 X9700="@Style(FontFamily Univers10,Singlesided,Spacing 0.7)
	@Style<TopMargin 0.7inch,BottomMargin 0.7inch>"
 >
@Pageheading(center "@b<Columbia University Center for Computing Activities>",
	immediate)
@MajorHeading(THE KERMIT FILE TRANSFER PROTOCOL)

@center(@i<July 1986>)
@blankspace(1.5)
@pageheading(center "")
Kermit is an error-correcting protocol for transferring sequential files
between computers of all sizes over ordinary asynchronous telecommunication
lines.  Kermit is non-@|proprietary, thoroughly documented, and in wide use.
The protocol and the original implementations were developed at Columbia
University and have been shared with thousands of other institutions all over
the world, many of which have made significant contributions of their own.
Kermit is presently available for more than 200 different machines and
operating systems, and additional versions are always under development.

All Kermit programs perform file transfer using the Kermit file transfer
protocol.  In addition, Kermit programs for personal computers also provide
terminal emulation, usually of the DEC VT52, VT100, or similar terminal, and
some of the mainframe Kermit programs are capable of initiating connections,
acting as dumb terminals to remote systems.  Kermit programs work only over
asynchronous RS-232 direct or dialup connections, or connections that simulate
them.  For file transfer to take place, there must be a Kermit program running
on @i<each> end of the connection, one on each computer.

There are Kermit programs for most popular "generic" operating systems,
including UNIX, MS-DOS, and CP/M, and for mainframes and minicomputers from
Burroughs, Cray, CDC, Data General, DEC, Gould (SEL), Harris, Honeywell,
Hewlett-Packard, IBM, Perkin-Elmer, Prime, Sperry/Univac, and Tandem, and for
particular microcomputers and workstations from Apple, Apollo, Atari,
Commodore, IBM, Tandy, and many others, written in a wide variety of languages
including many different assemblers, plus high-level languages like Algol,
Basic, Bliss, C, Forth, Fortran, Lisp, Mumps, Pascal, PL/I, and Ratfor.
A complete list of Kermit programs accompanies this flyer.

Here are some details about the several most popular Kermit programs.  Most of
the following implementations are capable of both local and remote operation,
server and client modes, text and binary file transfer, and support a full
range of communications options -- speed, parity, duplex, flow control,
handshake -- to allow adaptation to a wide variety of hosts (including IBM
mainframes) and communication media.
@begin<itemize>
@b<IBM PC Kermit> Version 2.29 runs under PC-DOS version 2.0 and later on the
entire IBM PC family, as well as on IBM "clones" and compatibles.  It provides
nearly complete DEC VT102 terminal emulation at speeds up to 38.4K baud fully
buffered and interrupt driven -- and includes support for color displays,
compatibility with various "desktop organizers," and selectable emulation of
other terminals.  There are also versions of Kermit specifically tailored for a
variety of other MS-DOS systems, including the DEC Rainbow, Zenith-100, Victor
9000, HP-110/150, and many others, and there is a "generic" MS-DOS Kermit for
systems not explicitly covered.

@b<Macintosh Kermit> Version 0.8 runs on the entire Apple Macintosh family,
from the original 128K Mac to the Mac/XL, to the fully configured
Macintosh-Plus.  It provides fairly complete VT102 emulation at speeds up to
9600 baud, and file transfer up to 56Kb.

@b<UNIX Kermit> is distributed only in C-language source form.  It may be built
for nearly any machine running practically any post-V6 variation of UNIX,
including V7, Berkeley 2.x and 4.x, AT&T System III and System V, Xenix, Venix,
and so on.  The same source also serves as a basis for Macintosh, Amiga, and
other Kermit programs.

@b<VAX/VMS Kermit> is written in Bliss, but it is also distributed in Macro-32
and hex form, so that a Bliss compiler is not required.  Other versions exist
in C and Pascal.

@b<IBM mainframe Kermit> programs for VM/CMS and MVS/TSO work only with
asynchronous ASCII TTY connections through 3705 or equivalent front ends, or
through Series/1, 7171, or similar protocol converters that support the Yale
ASCII Communications System; beyond this exception, Kermit cannot be used to
transfer files in the IBM 3270-style full-screen terminal environment.  There
are no Kermit programs for DOS/VSE, or IBM minis like the System/34 and
System/38, because these systems do not support asynchronous ASCII
communications.  Currently, IBM mainframe Kermits run only in remote mode.
@end<itemize>

The Kermit software -- including source code -- is furnished free and without
license, and without warranty of any kind, and neither Columbia University, nor
the individual authors, nor any institution that has contributed Kermit
material, acknowledge any liability for any claims arising from the use of
Kermit.  Furthermore, it must be stated that the quality of the Kermit programs
varies -- some are polished, well-documented professional products and others
are not.  Kermit programs are contributed by public-spirited volunteers, and
Columbia University does not wish to discourage such contributions by
subjecting them to a rating system.  Since source code is provided for all
implementations, users may make improvements or write documentation where it is
lacking and are encouraged to contribute their work back to Columbia for
further distribution.  Under certain conditions (described in a separate
document) software producers may include Kermit protocol in their products.

Although the Kermit software is free and unlicensed, Columbia University cannot
afford to distribute it for free because the demand is too great.  To defray
our costs for media, printing, postage, labor, and computing resources, we
require moderate distribution fees from those who request Kermit directly from
us.  The schedule is given on the accompanying Kermit Order Form.  You may also
obtain Kermit programs from many other sources, including user groups,
networks, dialup bulletin boards, and you may copy them from friends,
neighbors, and colleagues.  In fact, you may obtain Kermit programs from anyone
who is willing to share them with you, just as you may share them yourself.

Kermit is distributed by Columbia University primarily on 9-track magnetic
tape, suitable for reading on most mainframe and minicomputers.  It is assumed
that Kermit will be ordered in this form by institutional computer centers,
whose professional staff will take the responsibility for "bootstrapping" the
microcomputer versions from the tape to diskettes for their users.  The tapes
include source code and any available documentation for each Kermit
implementation, and in some cases also binaries (usually encoded in hex or
other printable format).  Selected microcomputer versions are also available
from Columbia on diskette.

Documentation includes the @i[Kermit User Guide], which contains complete
instructions for using and installing the major implementations of Kermit, and
the @i[Kermit Protocol Manual], which is a guide to writing new Kermit
programs.  One printed copy of each manual is included with any tape order, and
additional copies may be ordered separately.  The manuscript from the Kermit
article that appeared in the June and July 1984 issues of BYTE Magazine, and
the book @i<Kermit, A File Transfer Protocol> (Frank @w<da Cruz>, Digital
Press, 1986) may also be ordered separately.

Once you receive Kermit, you are encouraged to copy and redistribute it, with
the following stipulations: Kermit should not be sold for profit; credit should
be given where it is due; and new material should be sent back to Columbia
University so that we can maintain a definitive and comprehensive set of Kermit
implementations for further distribution.  And finally, @i<please use Kermit
only for peaceful and humane purposes>.
@pageheading[Center="@b<ORDERING INFORMATION>"]
@case<Device,File="@blankspace(1)ORDERING INFORMATION:@blankspace(1)",
	else="@newpage()" > There are two separate Kermit tapes, A and B.
There are too many Kermit files to fit on a single tape (soon, there will be
too many to fit on two tapes).  All tapes are half-inch, 2400-foot, 9-track,
1600bpi, odd parity.  They are available ONLY in the following formats:
@begin(description,leftmargin +10,indent -8,spread 0,above 1,below 1)
    ANSI:@\ANSI labeled ASCII, format D (variable length records, VMS COPY)

     TAR:@\UNIX TAR format (written on a VAX with 4.2bsd or Ultrix-32)

      OS:@\IBM OS standard labeled EBCDIC, format VB (variable length records)

     CMS:@\IBM VM/CMS VMFPLC2 format (unlabeled)

  DEC-10:@\DECsystem-10 Backup/Interchange format (unlabeled)

  DEC-20:@\DECSYSTEM-20 DUMPER format (unlabeled)
@end(description)
Blocksizes, when applicable, are our choice and in the range 8K-10K (use of
smaller blocksizes could overflow the tapes).  NO OTHER FORMATS ARE AVAILABLE.
We can NOT make 800bpi or 6250bpi tapes, unlabeled tapes (except as noted
above), fixed-block tapes, or custom tapes of any kind.  If none of the above
formats looks familiar to you, then specify ANSI -- this is an industry
standard format that @i<should> be readable by any computer system.

Tapes include machine readable source for both programs and documentation.

TAPE @qq(A) CONTAINS:
@begin(itemize,spread 0,above 0)
The microcomputer (PC, workstation) Kermit implementations

The Info-Kermit mail archive
@end(itemize)

TAPE @qq(B) CONTAINS:
@begin(itemize,spread 0,above 0)
The mainframe and minicomputer Kermit implementations.

The Kermit User Guide and the Kermit Protocol Manual
@end(itemize)

EXCEPTIONS:
@begin(itemize,spread 0.6,above 0,spacing 0.8)
C-Kermit is the basis of all Unix Kermit implementations, mainframe and micro.
It is on tape B.  Macintosh and Amiga Kermits are also generated from the
C-Kermit sources, so they too are on tape B.  Duplicate copies of the Macintosh
and Amiga hex and doc files (no source) are also included on tape A for
convenience.

While the general documentation is on tape B, any documentation of a specific
nature is distributed together with the program it describes.
@end(itemize)
Kermit diskettes may also be ordered in certain formats; see the order form.

@b<TO ORDER KERMIT>, fill out the Kermit Order Form and send it to:
@begin(format,above 1,below 1)
    Kermit Distribution
    Columbia University Center for Computing Activities
    612 West 115th Street
    New York, NY  10025  (USA)
@end(format)

North American orders are shipped by delivery service or first class US mail,
and shipping costs are included.  Overseas orders are shipped first class US
mail; an additional shipping charge is required.  Orders are normally processed
within 2-4 weeks of receipt, but firm delivery schedules or methods cannot be
guaranteed.
@pageheading[left="(V3.00, 1 July 86)",
	center="@b(@ux<KERMIT ORDER FORM>)",
	right="#@t<__________>"
]
@case<Device,File="@blankspace(1)KERMIT ORDER FORM:@blankspace(1)",
	else="@newpage()"
>
@case<device,x9700="@begin(format,font smallbodyfont,spacing 0.7,spread 0.8)",
	else="@begin(format)">
@tabclear()
@case[device,pagedfile="@tabset(68)",else="@tabset(5.5inch)"]
Check each desired Kermit Distribution Tape, $100 PER TAPE:

@case<device,x9700="@begin(format,font smallbodyfont,tabexport false)",
	else="@begin(format,tabexport false)">
@case[device,pagedfile="@tabset(24,32,40,48,56,64)",
	file="@tabset(24,32,40,48,56,64)",
	else="@tabset(1.8inch,2.4inch,3.0inch,3.6inch,4.2inch,4.8inch)"]
@u<Format:            @\ANSI@\TAR@\ OS@ @\CMS @\DEC-10@\DEC-20>
  Tape A (micros):    @\@t<[  ]@\[  ]@\[  ]@\[  ]@\[  ]@\[  ]>
  Tape B (mainframes):@\@t<[  ]@\[  ]@\[  ]@\[  ]@\[  ]@\[  ]>
@end<format>

@begin<text,fill,rightmargin +1inch,leftmargin +3,indent -3>
@t<[  ] >For PRIME Computers: Specify ANSI and check here to
request a short ANSI-tape-reader program listing (no charge).
@end<text>

Tape Subtotal (number of tapes times $100)@ @). @\$@t<__________>

Kermit programs on diskette, no source code, $10 each:
@t<[  ] >@i<Apple Macintosh>@ @). @\$@t<__________>
@t<[  ] >@i<DEC Rainbow; CP/M-86>@ @). @\$@t<__________>
@t<[  ] >@i<DEC Rainbow; MS-DOS>@ @). @\$@t<__________>
@t<[  ] >@i<DEC VT-180 Robin>@ @). @\$@t<__________>
@t<[  ] >@i<IBM PC, XT, and AT; PC-DOS>@ @). @\$@t<__________>

Printed documents, enter quantity:
@t<[  ] >Book: @i<Kermit, A File Transfer Protocol> ($25)@ @). @\$@t<__________>
@t<[  ] >@i<Kermit User Guide> ($5 each)@ @). @\$@t<__________>
@t<[  ] >@i<Kermit Protocol Manual> ($5)@ @). @\$@t<__________>
@t<[  ] >@i<BYTE> Magazine article manuscript ($5)@ @). @\$@t<__________>

@begin<text,fill,rightmargin +1inch>
Program source listings, $5 each.  NO NEED to order source
listings if you have ordered tapes, since program source is on the
tapes.  List the ones you want on a separate sheet.  Use prefixes from
version list.
@end<text>

Listings Subtotal@ @). @\$@t<__________>

If you can NOT prepay with a check, include @b(@u(BOTH)):
  1.  A $100@t<.>00 Order Processing (billing) Fee: @). @\$@t<__________>
  2.  @b(@u(AND)) a Purchase Order; write your P.O. number here:

            P.O.#: @t<__________________________>

Outside North America, add $25@t<.>00 for shipping@). @\$@t<__________>
USA RUSH ORDERS (Sent Federal Express), add $20@t<.>00@). @\$@t<__________>

@ux<GRAND TOTAL>: (@i<Do Not Add Sales Tax>)@). @\$@t<__________>

Make checks in U.S. Dollars, payable to:

    @b<COLUMBIA UNIVERSITY CENTER FOR COMPUTING ACTIVITIES>

@begin<text,fill,rightmargin +1inch,leftmargin +3,indent -3>
@t<[  ] >Check here for a Columbia University DEC-20 account application
form.  A CU20B account will allow you to read the Info-Kermit electronic
newsletter, and to use Kermit itself to obtain new releases of Kermit. 
@end<text>

@ux<PLEASE WRITE YOUR SHIPPING ADDRESS HERE>:
@end(format)
