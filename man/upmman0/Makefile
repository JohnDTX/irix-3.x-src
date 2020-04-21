#
# Makefile for man0 directory.  See README.
#
# $Revision: 1.1 $
# $Date: 89/03/27 17:06:36 $
#
# install will complain about a bad -c option if you don't have USER set
# in your environment.
#
INSTALL=/etc/install -U BSD42

# don't do anything
all: 

ptxx: tocrc
	tocrc all | tee tocrc.out

ptx.out: ptxx ptx.in
# can't use -mptx since we set the line length in ptx.in
	psroff -t ptx.in /usr/lib/macros/ptx ptxx > ptx.out 

# sections 1, 1m
tocv1.out: #toc.in - prevent toc? files from unintentionally being rebuilt
	psroff -t -ra1 toc.in  > tocv1.out 

# section 2,3
tocv2.out: #toc.in - prevent toc? files from unintentionally being rebuilt
	psroff -t -ra2 toc.in > tocv2.out 

# section 4,5,6,7,8
tocv3.out: #toc.in - prevent toc? files from unintentionally being rebuilt
	psroff -t -ra3 toc.in > tocv3.out 



print-all: #ptx.out tocv1.out tocv2.out - manually regenerate when required.
	lp ptx.out
	lp tocv1.out
	lp tocv2.out
	lp tocv3.out

clean:
	rm -f mkmanxref mkbadxref tocx[1-8] toc[1-8] ptxx \
		badsubsect badxref *.out fixref fixfc look badfc badfclines

manxref: mkmanxref
	mkmanxref > manxref

badsubsect: manxref
	grep '[a-z])	' manxref > badsubsect

badxref: manxref mkbadxref look
	mkbadxref manxref > badxref

fixref.out: fixref fixref.sed
	fixref > fixref.out

#
# I don't understand why manegrep finds `.ft', `.ftP' and `.ft P', but it does,
# so remove them with the second egrep.
badfc badfclines: listbadfc
	../manegrep '^\.(ft|cs|bd)[ 	]*[^1-3P]|\\f[^1-3P]|^\.fp|^\.uf' | \
		egrep -v ':\.ft([ 	]*P|[ 	]*)$$' | \
		egrep -v '^\.IX*' | \
		tee badfclines | listbadfc > badfc

fixfc.sed: badfc
	@echo You must update $@ manually ; exit 1

fixfc.out: fixfc fixfc.sed
	fixfc > fixfc.out

tocrc: tocrc.sh cshcmd shcmd 
	$(INSTALL) -o "$$USER" -c -m 555 tocrc.sh $@

mkmanxref: mkmanxref.sh
	$(INSTALL) -o $$USER -c -m 555 mkmanxref.sh $@

mkbadxref: mkbadxref.sh
	$(INSTALL) -o $$USER -c -m 555 mkbadxref.sh $@

fixref: fixref.csh
	$(INSTALL) -o $$USER -c -m 555 fixref.csh $@

fixfc: fixfc.csh
	$(INSTALL) -o $$USER -c -m 555 fixfc.csh $@

mess:
	rm -r *.B* *.C*

covers:
	psroff cover.pro.1 cover.pro.2 cover.pro.3 cover.adm cover.usr.1 cover.usr.2

credits:
	psroff credit.pro.1 credit.pro.2 credit.pro.3 credit.adm credit.usr.1 credit.usr.2
