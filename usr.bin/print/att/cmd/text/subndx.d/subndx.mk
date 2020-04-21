#
#ident	"\%W\%"
#
# Notes:
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	subj/ndx make file (text subsystem)
#
# DSL 2

#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs

#
# Compile Time Options
#
LDFLAGS = -s 
IFLAGS = -i
#
# Local Definitions
#
#
I_FLAGS	=-idb "dwb.sw.dwb"
# configuration files that should be discarded if user has one already
NOUPDATE=-idb "dwb.sw.dwb config(noupdate)"
# configuration files that should override, but save, existing version
UPDATE	=-idb "dwb.sw.dwb config(update)"
SU_FLAGS=-m 04555 -u root -g sys $(I_FLAGS)
GU_FLAGS=-m 755 -u bin -g bin $(UPDATE)
DAT_FLAGS=-m 644 -u bin -g bin $(UPDATE)

INSDIR = /usr/bin
INSLIB = /usr/lib

#
# Targets/Rules
#

default:    stamp all

clean:	subjclean ndxclean

clobber:	clean subjclobber ndxclobber

FRC:

#
# Specific Target/Rules follow
#

stamp:
	pwd

install: default
	$(INSTALL) $(I_FLAGS) -F $(INSDIR) "subj ndx"
	$(INSTALL) $(I_FLAGS) -F $(INSLIB)/dwb "sbj1 sbj2 sbj3 parts style1"
	$(INSTALL) $(I_FLAGS) -F $(INSLIB)/dwb "style2 style3 deroff ndexer"
	$(INSTALL) $(I_FLAGS) -F $(INSLIB)/dwb "pages ndxformat sbjprep"

compile all:	subj ndx

subj:	sbj1 sbj2 sbj3 parts
	cp subj.sh subj

sbj1:	sbj1.o cnst.h
	- if vax || u3b || m68000  ; \
	then $(CC) -O $(LDFLAGS) sbj1.o -ll -o sbj1 ; fi
	- if mips ; \
	then $(CC) $(CFLAGS) -O $(LDFLAGS) sbj1.o -ll -o sbj1 ; fi

sbj2:	sbj2.o case.o cnst.h
	- if vax || u3b || m68000  ; \
	then $(CC) -O $(LDFLAGS) sbj2.o case.o -ll -o sbj2 ; fi
	- if mips ; \
	then $(CC) $(CFLAGS) $(LDFLAGS) sbj2.o case.o -ll -o sbj2 ; fi

sbj3:	sbj3.o case.o omit.o cnst.h
	- if vax || u3b || m68000  ; \
	then $(CC) -O $(LDFLAGS) sbj3.o case.o omit.o -ll -o sbj3 ; fi
	- if mips ; \
	then $(CC) $(CFLAGS) $(LDFLAGS) sbj3.o case.o omit.o -ll -o sbj3 ; fi

ndx:	ndexer pages ndxformat sbjprep
	cp ndx.sh ndx

ndexer:	ndexer.o rootwd.o str.o strr.o case.o space.o dstructs.h ehash.h \
	edict.h
	- if vax || u3b || m68000  ; \
	then $(CC) -O $(LDFLAGS) ndexer.o rootwd.o str.o strr.o case.o \
	    space.o -ll $(IFLAGS) -o ndexer ; fi
	- if mips ; \
	then $(CC) $(CFLAGS) $(LDFLAGS) ndexer.o rootwd.o str.o strr.o \
	    case.o space.o -ll -o ndexer ; fi

pages:	pages.c
	- if vax || u3b || m68000  ; \
	then $(CC) -O $(LDFLAGS) pages.c -o pages ; fi
	- if mips ; \
	then $(CC) $(CFLAGS) $(LDFLAGS) pages.c -o pages ; fi

ndxformat:	ndxformat.c
	- if vax || u3b || m68000  ; \
	then $(CC) -O $(LDFLAGS) ndxformat.c -o ndxformat ; fi
	- if mips ; \
	then $(CC) $(CFLAGS) $(LDFLAGS) ndxformat.c -o ndxformat ; fi

sbjprep:	sbjprep.c
	- if vax || u3b || m68000  ; \
	then $(CC) -O $(LDFLAGS) sbjprep.c -o sbjprep ; fi
	- if mips ; \
	then $(CC) $(CFLAGS) $(LDFLAGS) sbjprep.c -o sbjprep ; fi

parts:	parts.sh style1 style2 style3 deroff
	cp parts.sh parts

style1:	nwords.o nhash.h dict.h ydict.h names.h abbrev.h
	- if vax || u3b || m68000  ; \
	then $(CC) -O $(LDFLAGS) nwords.o -ll -o style1 ; fi
	- if mips ; \
	then $(CC) $(CFLAGS) $(LDFLAGS) nwords.o -ll -o style1 ; fi

style2:	end.o ehash.h edict.h names.h
	- if vax || u3b || m68000  ; \
	then $(CC) -O $(LDFLAGS) end.o -ll -o style2 ; fi
	- if mips ; \
	then $(CC) $(CFLAGS) $(LDFLAGS) end.o -ll -o style2 ; fi

style3:	part.o pscan.o outp.o extern.o
	- if vax || u3b || m68000  ; \
	then $(CC) -O $(LDFLAGS) part.o pscan.o outp.o extern.o -ll -o style3 \
	 ; fi
	- if mips ; \
	then $(CC) $(CFLAGS) $(LDFLAGS) part.o pscan.o outp.o extern.o -ll -o \
	    style3 ; fi

deroff:	deroff.o
	- if vax || u3b || m68000  ; \
	then $(CC) -O $(LDFLAGS) deroff.o $(IFLAGS) -o deroff ; fi
	- if mips ; \
	then $(CC) $(CFLAGS) $(CFLAGS) $(LDFLAGS) deroff.o  -o deroff ; fi

subjclean:	
	rm -f sbj1.o sbj2.o sbj3.o case.o omit.o end.o nwords.o part.o \
		pscan.o outp.o extern.o deroff.o

ndxclean:	
	rm -f ndexer.o rootwd.o str.o strr.o case.o space.o pages.o \
		 ndxformat.o sbjprep.o

subjclobber:	
	rm -f sbj1 sbj2 sbj3 subj parts style1 style2 style3 deroff

ndxclobber:	
	rm -f ndx ndexer pages ndxformat sbjprep
