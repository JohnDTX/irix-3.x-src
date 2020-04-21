#
# Standard make include for sgi standard makefiles.
#
# Define TARGETS to be a list of targets to produce.
#
# Define SRCS to be a list of source files that will produce TARGETS.
#
# Define HDRS to be a list of headers that the source uses.
#
# Define MISC to be a list of miscellanous stuff that is needed to
# produce TARGETS.  This is only used by the "depend" rule to catch
# any funny files that are necessary.
#
# Define QUIET to be "#" if you don't want any make depend noise
#
# Define AUTODEPEND to be "autodepend" if you want make to automatically
# generate dependencies if they don't exist in the makefile.  This won't
# automatically update the dependencies when a source file changes.
#
# $Source: /d2/3.7/src/usr.lib/RCS/make.h,v $
# $Revision: 1.1 $
# $Date: 89/03/27 18:32:39 $
#
CFLAGS= ${COPTS} ${CDEFS} ${CINCL} ${CPROF} -I${SRCDIR}

all: ${AUTODEPEND} ${TARGETS}

# This line gives make a handle on the relationship between source and RCS
# files.
#${SRCS} ${HDRS} ${MISC}: $$@,v

# Remove all non-targets and flak
clean:
	rm -f a.out core fluff *.o

# Remove targets only
clobber:
	rm -f ${TARGETS}

# Build a tags file for "vi"
tags: ${SRCS}
	ctags ${SRCS}

# Lint the source code
fluff: ${SRCS} ${HDRS} ${MISC}
	lint -Dlint ${CDEFS} ${CINCL} ${SRCS} ${LIBS} > fluff 2>&1

# Automatically make dependencies if they aren't in the file
autodepend:
	@-if egrep "THIS 2nd LINE" < Makefile > /dev/null; \
	    then true; else ${MAKE} depend; fi

# Make the source/header dependencies
depend: ${SRCS} ${HDRS} ${MISC}
	@rm -f makedep Makefile.BAK
	@cp Makefile "#Makefile"
	@${QUIET}echo Making dependencies:
	@for i in ${SRCS}; do \
		${QUIET} echo $$i; \
		cc -M ${CFLAGS} $$i | \
		sed -e 's:\.\./[^ ]*/\.\.:..:g' \
		    -e 's: ./: :' | \
		awk 'BEGIN { INDENT = "\t" } \
		     NF > 0 { \
			if ($$1 != lhs) { \
			    lhs = $$1; print p_line; p_line = $$1 " "; \
			    lim = 72; } \
			if (length(p_line) + length($$2) > lim) { \
			    print p_line "\\"; p_line = INDENT; lim = 63; \
			} \
			p_line = p_line $$2 " "; \
		     } \
		END { print p_line; }' >> makedep; \
	done
	@sed -e "/^# DO NOT DELETE THIS LINE -- make depend uses it/,/^# DO NOT DELETE THIS 2nd LINE -- make depend uses it/d" \
	    -e "/^# DO NOT DELETE THIS LINE -- make depend use it/"',$$d' \
	    < Makefile > Makefile.new
	@echo "# DO NOT DELETE THIS LINE -- make depend uses it" >> \
		Makefile.new
	@cat makedep >> Makefile.new
	@echo "# DO NOT DELETE THIS 2nd LINE -- make depend uses it" >> \
		Makefile.new
	@rm -f Makefile; mv Makefile.new Makefile
	@rm -f makedep "#Makefile"
	@${QUIET}echo Make depend is done
