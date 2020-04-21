#
#ident	"$Header: /d2/3.7/src/usr.bin/print/trscript/src/RCS/makefile,v 1.1 89/03/27 18:20:39 root Exp $"
#
# Notes:
#

#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs

#
# Compile Time Options
#
HCC=	ROOTDIR=/usr /bin/cc

#
# Local Definitions
#
RM=	rm
CP=	cp
CFILES=	ctags.c fold.c mkstr.c xstr.c
CMDS=	ctags fold mkstr xstr cxref
HCMDS=	mkstr_host xstr_host

#
# Targets/Rules
#

default: $(CMDS) $(HCMDS)

clean:

clobber:	clean
	-rm -f $(CMDS) $(HCMDS)

FRC:

#
# Specific Target/Rules follow
#
install: default
	$(INSTALL) ctags
	$(INSTALL) fold
	$(INSTALL) mkstr
	$(INSTALL) xstr
	$(INSTALL) cxref
	
mkstr_host:
	$(HCC) -o $@ mkstr.c

xstr_host:
	$(HCC) -o $@ xstr.c

depend: /tmp
	grep '^#include' ${CFILES} | \
	      grep -v '<' | \
	      sed 's/:[^"]*"\([^"]*\)".*/: \1/' | \
	      sed 's/\.c/.o/' | \
	      sed 's/\.s/.o/' | \
	      sed 's,\.\./[a-z0-9]*/\([a-z0-9_]*\.o\),\1,' | \
	awk ' { if ($$1 != prev) { print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		       else rec = rec " " $$2 } } \
	      END { print rec } ' > makedep
	echo '/^# DO NOT DELETE THIS LINE/+1,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w makefile' >>eddep
	${CP} makefile makefile.bak
	${RM} makefile
	ed - makefile.bak < eddep
	${RM} eddep makedep

# DO NOT DELETE THIS LINE

