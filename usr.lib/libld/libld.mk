#	@(#)libld.mk	2.5	

#	GLOBAL makefile for libld.a library

#	Target system processors:	vax (VAX 11/780 and VAX 11/750)
#					pdp (PDP 11/70)

INSDIR = /usr/lib
CURDIR = ..


all:	libld

libld:
	-if vax; \
	then \
		cd vax; \
		$(MAKE) INSDIR=$(CURDIR); \
	elif pdp11; \
	then \
		cd pdp11; \
		$(MAKE) INSDIR=$(CURDIR); \
	else \
		@echo 'Cannot make libld.a library: unknown target procesor.'; \
	fi


clean:
	-if vax; \
	then \
		cd vax; \
		$(MAKE) clean; \
	elif pdp11; \
	then \
		cd pdp11; \
		$(MAKE) clean; \
	fi

install: libld
	-if vax; \
	then \
		cp libld.a $(INSDIR)/libld.a; \
		rm -f libld.a; \
	fi

clobber: clean
	-rm -f libld.a
