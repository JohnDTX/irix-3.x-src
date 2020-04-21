#
# Makefile for man0 directory.  See README.
#
# $Revision: 1.1 $
# $Date: 89/03/27 16:39:14 $
#
all: ptx.in badsubsect badxref badnamesec

ptx.in toc.in: tocrc
	tocrc all

ptx.out: ptx.in
	psroff -tptx.out ptx.in

tocv1.out: #toc.in - prevent toc? files from unintentionally being rebuilt
	psroff -ttocv1.out -ra0 toc.in

tocv2.out: #toc.in - prevent toc? files from unintentionally being rebuilt
	psroff -ttocv2.out -ra1 toc.in

print-all: #ptx.out tocv1.out tocv2.out - manually regenerate when required.
	lp ptx.out
	lp tocv1.out
	lp tocv2.out

manxref: mkmanxref
	mkmanxref > manxref

badsubsect: manxref
	grep '[a-z])	' manxref | sort -b +1 > badsubsect

badxref: manxref mkbadxref exceptions/badxref
	mkbadxref manxref > temp
	sed -e '/^#/d' -e '/^[	 ]*$$/d' exceptions/badxref \
	    | sort -b \
	    | comm -23 temp - > badxref
	rm -f temp

badnamesec:
	( cd .. ; find [ua]_man troff \
		\( -name '*.[1-8]' -o -name '*.[1-8][a-z]' \) -print \
	    | sed -e '/\/gl1\//d' -e '/\/vms\//d' -e '/\/trash\//d' \
		  -e '\/intro\./d' \
	    | while read file; do \
		sed -e '/^\.so /d' -e '1,/^\.SH .*NAME/d' \
		    -e '/^\.SH .*SYNOPSIS/q' -e '/^\.SH .*SPECIFICATION/q' \
		    -e '/^\.SH .*DESCRIPTION/q' $$file \
		    | sed '/^\.SH /d' \
		    | tr '\012' ' ' \
		    | awk '{print fn ":", $$0}' fn=$$file - ; \
	      done ) > temp.ns
	( egrep -v '\\-' temp.ns ; egrep ',[^ ]' temp.ns ) > $@
	rm -f temp.ns
		
fixref.out: fixref fixref.sed
	fixref > fixref.out

tocrc: tocrc.sh cshcmd shcmd
	install -o $$USER -c -m 555 tocrc.sh $@

mkmanxref: mkmanxref.sh
	install -o $$USER -c -m 555 mkmanxref.sh $@

mkbadxref: mkbadxref.sh
	install -o $$USER -c -m 555 mkbadxref.sh $@

fixref: fixref.csh
	install -o $$USER -c -m 555 fixref.csh $@

clean:
	rm -f tocrc mkmanxref mkbadxref tocx[1-8] toc[1-8] ptxx \
		badsubsect badxref *.out badnamesec temp.ns
