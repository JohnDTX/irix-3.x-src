$	was_verify = 'f$verify(0)'
$! CKVMAK.COM	1.0 (001) 17-May-1985
$!
$! -- This file should be renamed to CCMAKE.COM before use. --
$!
$! The SOURCE file is processed to create the OUTPUT file.  This command
$! checks creation dates and does not compile if both files are present
$! and the OUTPUT file is newer (younger) than the SOURCE file.
$!
$! Usage:
$!
$!	ccmake [SOURCE [OUTPUT [P3]]]
$!
$! I.e., if both source and output are given, they are used.  If only
$! a source file is given, the output is taken to be SOURCE.OBJ in
$! the current directory.
$!
$! A P3 argument may be given to disable the optimizer.
$!
$	SOURCE = P1
$	if SOURCE .eqs. "" then inquire SOURCE "C Source File"
$	SOURCE = f$parse(SOURCE, ".C")
$	fname = f$parse(SOURCE,,, "NAME")
$	OUTPUT = "''p2'"
$	if OUTPUT .eqs. "" then goto default
$	if "''f$parse(OUTPUT,,, "NAME")'" .eqs. "" then -
	    OUTPUT = "''OUTPUT'''fname'"
$	if "''f$parse(OUTPUT,,, "TYPE")'" .eqs. "." then -
		OUTPUT = "''OUTPUT'.OBJ"
$	goto checkdates
$ default:
$	OUTPUT = "''fname'.OBJ"
$	goto checkdates
$!
$! Continue at must_process if either file is missing or the source is younger
$! A missing SOURCE is, of course, an error -- but one that should be
$! caught by the "normal" command.
$!
$ checkdates:
$	on error then goto must_process
$	if f$search(SOURCE) .eqs. "" then goto must_process
$	if f$search(OUTPUT) .eqs. "" then goto must_process
$	src_time = f$file_attributes(SOURCE, "CDT")	! Get creation time
$	out_time = f$file_attributes(OUTPUT, "CDT")	! for both files.
$	if f$cvtime(src_time) .ges. f$cvtime(out_time) then -
	    goto must_process
$	write sys$output OUTPUT, " is up to date."
$	goto finish
$!
$! Come here to build OUTPUT from SOURCE
$!
$ must_process:
$	on error then goto finish
$	on controly then goto fail
$!
$! Insert commands to create OUTPUT from SOURCE here, for example:
$!
$	write sys$output OUTPUT, " <= ", SOURCE
$	cc/nolist/obj='OUTPUT''P3' 'SOURCE'
$!
$ finish:
$	if was_verify then set verify
$	exit
$!
$ fail:
$	if was_verify then set verify
$	stop
